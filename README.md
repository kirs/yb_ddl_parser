# yb_ddl_parser

Small Ruby gem for extracting stable facts from YugabyteDB DDL.

This is intentionally not a drop-in replacement for `pg_query`. It exposes a
small AST suited to migration safety checks and hides PostgreSQL/YugabyteDB
internal parse node churn.

## API

```ruby
require "yb_ddl_parser"

result = YbDDLParser.parse(sql)
result.statements # => [YbDDLParser::Statement, ...]
result.errors     # => [{ message:, position: }, ...]

stmt = YbDDLParser.parse!(sql).single_statement!
stmt.kind
stmt.relation&.qualified_name
```

`parse` always returns a `YbDDLParser::ParseResult`. `parse!` raises
`YbDDLParser::ParseError` on parser errors.

The public value objects are:

- `ParseResult`: `statements`, `errors`, `single_statement!`,
  `each_statement`, `each_column`
- `Statement`: stable DDL facts such as `kind`, `sql`, `relation`, `columns`,
  `constraints`, `primary_key`, `commands`, `keys`, `split`, `partition`,
  `partition_of`, `partition_bound_sql`, `definition_sql`, `raw_node_type`,
  `if_exists`, `if_not_exists`, and `new_name`
- `RelationName`: `schema`, `name`, `qualified_name`
- `Column`: `name`, `type`, `typmods`, `constraints`
- `Constraint`: `type`, `name`, `columns`, `key_columns`, `raw_expression`,
  `functions`
- `KeyColumn`: `name`, `expression`, `order`, `nulls`, `hash?`
- `Command`: `kind`, `column`, `definition`, `constraint`, `tablespace`,
  `missing_ok`
- `Split`: `type`, `num_tablets`, `points`
- `Partition`: `strategy`, `keys`

Useful `Statement` helpers include `drop_table?`, `drop_index?`,
`alter_index?`, `partition_parent?`, `partition_child?`,
`explicit_concurrently?`, `target_relation`, `target_relations`, and
`target_name`.

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
