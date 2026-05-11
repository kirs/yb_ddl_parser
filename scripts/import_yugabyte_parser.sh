#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "usage: $0 /path/to/yugabyte-db" >&2
  exit 2
fi

src_root="$1"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
vendor_root="$repo_root/ext/yb_ddl_parser/vendor/yugabyte-postgres"

if [[ ! -f "$src_root/src/postgres/src/backend/parser/gram.y" ]]; then
  echo "not a yugabyte-db checkout: $src_root" >&2
  exit 1
fi

rm -rf "$vendor_root"
mkdir -p "$vendor_root/src/backend/parser"
mkdir -p "$vendor_root/src/backend/nodes"
mkdir -p "$vendor_root/src/include"
mkdir -p "$vendor_root/src/common"

copy_file() {
  local rel="$1"
  mkdir -p "$vendor_root/$(dirname "$rel")"
  cp "$src_root/src/postgres/$rel" "$vendor_root/$rel"
}

# Parser sources. Generated gram.c/gram.h/scan.c should be copied when present
# so gem installation does not require bison/flex.
for rel in \
  src/backend/parser/parser.c \
  src/backend/parser/scansup.c \
  src/backend/parser/gram.y \
  src/backend/parser/scan.l \
  src/backend/parser/check_keywords.pl
do
  copy_file "$rel"
done

# Common scanner keyword support.
for rel in \
  src/common/kwlookup.c \
  src/common/keywords.c
do
  copy_file "$rel"
done

for rel in \
  src/backend/parser/gram.c \
  src/backend/parser/gram.h \
  src/backend/parser/scan.c
do
  [[ -f "$src_root/src/postgres/$rel" ]] && copy_file "$rel"
done

# Node support needed by raw parser trees and extraction.
for rel in \
  src/backend/nodes/list.c \
  src/backend/nodes/value.c \
  src/backend/nodes/makefuncs.c \
  src/backend/nodes/nodes.c \
  src/backend/nodes/outfuncs.c
do
  copy_file "$rel"
done

# Headers are intentionally broad at first; later we can shrink this once the
# extension links cleanly.
rsync -a --include='*/' --include='*.h' --exclude='*' \
  "$src_root/src/postgres/src/include/" "$vendor_root/src/include/"

if [[ ! -f "$vendor_root/src/backend/parser/gram.c" || ! -f "$vendor_root/src/backend/parser/gram.h" ]]; then
  (
    cd "$vendor_root/src/backend/parser"
    /usr/bin/perl check_keywords.pl gram.y "$vendor_root/src/include/parser/kwlist.h"
    bison -d -o gram.c gram.y
  )
fi

if [[ ! -f "$vendor_root/src/backend/parser/scan.c" ]]; then
  (
    cd "$vendor_root/src/backend/parser"
    flex -o scan.c scan.l
  )
fi

# Generated headers required by the standalone parser build.
/usr/bin/perl "$src_root/src/postgres/src/backend/utils/generate-errcodes.pl" \
  "$src_root/src/postgres/src/backend/utils/errcodes.txt" \
  > "$vendor_root/src/include/utils/errcodes.h"

(
  cd "$vendor_root/src/include/storage"
  /usr/bin/perl "$src_root/src/postgres/src/backend/storage/lmgr/generate-lwlocknames.pl" \
    "$src_root/src/postgres/src/backend/storage/lmgr/lwlocknames.txt"
  rm -f lwlocknames.c
)

/usr/bin/perl -I "$src_root/src/postgres/src/backend/catalog" \
  "$src_root/src/postgres/src/backend/utils/Gen_fmgrtab.pl" \
  --include-path "$src_root/src/postgres/src/include" \
  --output "$vendor_root/src/include/utils" \
  "$src_root/src/postgres/src/include/catalog/pg_proc.dat"
rm -f "$vendor_root/src/include/utils/fmgrtab.c"

/usr/bin/perl "$src_root/src/postgres/src/tools/gen_keywordlist.pl" \
  --extern \
  --output "$vendor_root/src/common" \
  "$src_root/src/postgres/src/include/parser/kwlist.h"

catalog_headers=()
while IFS= read -r header; do
  catalog_headers+=("$header")
done < <(find "$src_root/src/postgres/src/include/catalog" -maxdepth 1 \
  \( -name 'pg_*.h' -o -name 'yb_*.h' \) | sort)

/usr/bin/perl "$src_root/src/postgres/src/backend/catalog/genbki.pl" \
  --set-version 15 \
  --include-path "$src_root/src/postgres/src/include" \
  --output "$vendor_root/src/include/catalog" \
  "${catalog_headers[@]}"
rm -f "$vendor_root/src/include/catalog/postgres.bki" \
  "$vendor_root/src/include/catalog/system_constraints.sql" \
  "$vendor_root/src/include/catalog/schemapg.h"

if git -C "$src_root" rev-parse HEAD >/dev/null 2>&1; then
  git -C "$src_root" rev-parse HEAD > "$vendor_root/YUGABYTE_COMMIT"
else
  echo "unknown" > "$vendor_root/YUGABYTE_COMMIT"
fi

echo "Imported Yugabyte parser slice into $vendor_root"
