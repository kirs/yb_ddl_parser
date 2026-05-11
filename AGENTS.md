# AGENTS.md

## Project Goal

`yb_ddl_parser` is a Ruby gem that parses YugabyteDB DDL using YugabyteDB's
vendored PostgreSQL parser fork and exposes a small, stable Ruby AST.

This is not a general `pg_query` replacement. Do not expose raw
PostgreSQL/Yugabyte parse nodes as the public API, and do not hand-parse SQL.

## Architecture

The intended boundary is:

```text
YbDDLParser.parse(sql)
  -> YbDDLParser::Native.parse(sql)
    -> raw_parser(sql, RAW_PARSE_DEFAULT)
      -> RawStmt list
        -> C extractor returns Ruby hashes/arrays
          -> lib/yb_ddl_parser/ast.rb wraps value objects
```

Important files:

- `lib/yb_ddl_parser.rb`: public API.
- `lib/yb_ddl_parser/ast.rb`: stable Ruby value objects.
- `ext/yb_ddl_parser/yb_ddl_parser_ext.c`: native parser boundary and DDL
  extractor.
- `ext/yb_ddl_parser/yb_parser_stubs.c`: standalone PostgreSQL/Yugabyte runtime
  shims.
- `ext/yb_ddl_parser/extconf.rb`: native extension build inputs.
- `scripts/import_yugabyte_parser.sh`: repeatable vendor refresh script.
- `test/yb_ddl_parser_test.rb`: extractor and parser behavior coverage.

## Current State

The native extension loads the vendored Yugabyte parser and calls:

```c
raw_parser(sql, RAW_PARSE_DEFAULT)
```

It extracts statement-level DDL facts plus the first stable pass of table,
index, constraint, column, split, partition, and alter-command facts.

Covered statement families include:

- `CREATE TABLE`
- `CREATE INDEX`
- `CREATE SCHEMA`
- `CREATE TABLESPACE`
- `ALTER TABLE`
- `DROP TABLE`
- `DROP INDEX`

Yugabyte-specific syntax is first-class parser output, including:

- `HASH` key columns
- `USING lsm`
- `NONCONCURRENTLY`
- `SPLIT INTO n TABLETS`
- `SPLIT AT VALUES`
- tablespace `replica_placement`

## Build And Test

Use the requested Ruby environment:

```sh
shadowenv exec -- /Users/kirs/.local/state/tec/profiles/base/current/global/bin/ruby -S bundle exec rake compile test
```

For a faster test-only pass after the extension has already been built:

```sh
shadowenv exec -- /Users/kirs/.local/state/tec/profiles/base/current/global/bin/ruby -S bundle exec rake test
```

`Gemfile.lock` should be committed.

## Remaining Work

### Runtime Hardening

- Replace placeholder catalog/expression stubs only where parser paths need real
  behavior.
- Review parser memory lifetime. Current `palloc` compatibility uses process
  allocations suitable for smoke tests, not a proper per-parse memory context.
- Add runtime-focused tests for Unicode escapes, string literals, comments,
  multi-statement input, and more invalid SQL errors.
- Use `libpg_query` / `pg_query` packaging patterns as references for standalone
  PostgreSQL parser runtime behavior.

### Extractor Hardening

- Broaden expression rendering/function collection beyond simple constants,
  column refs, function calls, casts, and binary operators.
- Add deeper foreign-key extraction, including referenced table/columns and
  update/delete actions.
- Preserve more table/index options where migration checks need them.
- Add stronger coverage for partitioning and `ALTER INDEX` variants.
- Decide whether public type names should stay parser-canonical (`int8`,
  `varchar`) or be normalized back to SQL spelling.

### Packaging

- Keep vendoring tight; vendor only what is needed to compile and run the parser.
- Keep generated parser files checked in so gem users do not need `bison` or
  `flex`.
- Keep `scripts/import_yugabyte_parser.sh` repeatable.

## Licensing

Project code is Apache-2.0. The gem vendors YugabyteDB/PostgreSQL parser code;
keep upstream copyright headers and third-party notices intact.

Relevant files:

- `LICENSE`
- `LICENSES/Apache-2.0.txt`
- `LICENSES/PostgreSQL.txt`
- `THIRD_PARTY_NOTICES.md`

The gemspec uses:

```ruby
spec.licenses = ["Apache-2.0", "PostgreSQL"]
```

