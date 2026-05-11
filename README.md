# yb_ddl_parser

Small Ruby gem for extracting stable facts from YugabyteDB DDL.

This is intentionally not a drop-in replacement for `pg_query`. It exposes a
small AST suited to migration safety checks and hides PostgreSQL/YugabyteDB
internal parse node churn.

## API

```ruby
require "yb_ddl_parser"

result = YbDDLParser.parse(sql)
result.statements # => [YbDDLParser::AST::Statement, ...]
result.errors     # => [YbDDLParser::AST::ParseDiagnostic, ...]

stmt = YbDDLParser.parse_one!(sql)
stmt.kind
stmt.relation&.qualified_name
```

`parse` always returns a `YbDDLParser::AST::ParseResult`. `parse!` raises
`YbDDLParser::ParseError` on parser errors. `parse_one!` also requires exactly
one statement.

The public value objects are:

- `AST::ParseResult`: `statements`, `errors`, `single_statement!`,
  `each_statement`, `each_column`
- `AST::Statement`: stable DDL facts such as `kind`, `sql`, `relation`,
  `columns`, `constraints`, `primary_key`, `commands`, `keys`, `tablet_split`,
  `partition_spec`, `partition_of`, `partition_bound_sql`, `raw_node_type`,
  `if_exists`, `if_not_exists`, and `new_name`
- `AST::RelationName`: `schema`, `name`, `qualified_name`
- `AST::Column`: `name`, `type`, `typmods`, `constraints`
- `AST::Constraint`: `type`, `name`, `columns`, `key_columns`, `raw_expression`,
  `functions`
- `AST::KeyColumn`: `name`, `expression`, `order`, `nulls`, `hashed`,
  `hash_group`, `hash?`
- `AST::Command`: `kind`, `column`, `definition`, `constraint`, `tablespace`,
  `missing_ok`
- `AST::TabletSplit`: `type`, `num_tablets`, `points`
- `AST::PartitionSpec`: `strategy`, `keys`
- `AST::ParseDiagnostic`: `message`, `position`

`YbDDLParser::ParseResult` and `YbDDLParser::Statement` are aliases for the
common AST types. `Statement#split` and `Statement#partition` remain aliases for
`tablet_split` and `partition_spec`.

Useful `Statement` helpers include `drop_table?`, `drop_index?`,
`alter_index?`, `partition_parent?`, `partition_child?`,
`explicit_concurrently?`, `target_relation`, `target_relations`, and
`target_name`, and `hash_key_groups`.

For Yugabyte HASH key groups such as `((tenant_id, user_id) HASH)`, grouped
keys share the same `KeyColumn#hash_group`. `Statement#hash_key_groups` returns
those grouped key names or expressions, for example `[["tenant_id", "user_id"]]`.

`raw_node_type` exposes the vendored parser node tag, such as `T_SelectStmt`,
for diagnostics when `kind` is `:unknown`.

`concurrently` is `:explicit` when SQL says `CONCURRENTLY`, `:disabled` when
SQL says `NONCONCURRENTLY` or the statement is not concurrent, and `:implicit`
for YugabyteDB index creation where concurrency is enabled by default.

Current statement families include `CREATE TABLE`, `CREATE INDEX`,
`CREATE SCHEMA`, `CREATE TABLESPACE`, `ALTER TABLE`, `DROP TABLE`, and
`DROP INDEX`. The parser also returns shallow kinds for common default-deny
statements such as `TRUNCATE`, `COMMENT`, table renames, and `CREATE VIEW`.
See `test/yb_ddl_parser_test.rb` for more examples.

## Parser

The gem embeds YugabyteDB's PostgreSQL parser fork at commit
`6fc6f6469031a60fa6dbe5d5fa70680280b9ad9b`.

DDL syntax is fairly stable, so this vendored parser should not need frequent
updates unless YugabyteDB adds syntax this gem needs to understand.

At runtime, SQL is parsed by the vendored native parser via
`raw_parser(sql, RAW_PARSE_DEFAULT)`. A C extractor turns the raw parser output
into Ruby hashes and `lib/yb_ddl_parser/ast.rb` wraps those hashes in the stable
value objects above.

## License

Project code is licensed under the Apache License, Version 2.0.

This gem vendors portions of YugabyteDB's PostgreSQL parser fork. Vendored
YugabyteDB code is Apache-2.0 unless otherwise noted by upstream files, and
PostgreSQL-derived files remain under the PostgreSQL License. See
`THIRD_PARTY_NOTICES.md` and `LICENSES/` for details.
