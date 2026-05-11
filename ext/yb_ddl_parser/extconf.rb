# frozen_string_literal: true

require "mkmf"

vendor = File.expand_path("vendor/yugabyte-postgres", __dir__)
compat = File.expand_path("vendor/compat/postgres", __dir__)
parser_src = File.join(vendor, "src/backend/parser")
nodes_src = File.join(vendor, "src/backend/nodes")
common_src = File.join(vendor, "src/common")

$INCFLAGS = [
  "-I#{compat}",
  "-I#{vendor}/src/include",
  "-I#{vendor}/src/backend",
  "-I#{vendor}/src/backend/parser",
  "-I#{vendor}/src/common",
  $INCFLAGS,
].join(" ")

$CFLAGS << " -std=gnu99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function"

vendor_sources = %w[
  parser.c
  gram.c
  scan.c
  scansup.c
  list.c
  value.c
  makefuncs.c
  nodes.c
  kwlookup.c
  keywords.c
]

$VPATH << parser_src
$VPATH << nodes_src
$VPATH << common_src

$srcs = ["yb_ddl_parser_ext.c", "yb_parser_stubs.c"] + vendor_sources

create_makefile("yb_ddl_parser/yb_ddl_parser_ext")
