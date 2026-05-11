# frozen_string_literal: true

require "test_helper"

class YbDDLParserTest < Minitest::Test
  def test_public_ast_namespace_and_top_level_aliases
    assert_same YbDDLParser::AST::Statement, YbDDLParser::Statement
    assert_same YbDDLParser::AST::ParseResult, YbDDLParser::ParseResult
    refute YbDDLParser.const_defined?(:Split, false)
    refute YbDDLParser.const_defined?(:Partition, false)
  end

  def test_multi_statement_sql_slices_and_create_table_relation
    result = YbDDLParser.parse!(<<~SQL)
      CREATE TABLE IF NOT EXISTS "core"."orders" (id int);
      CREATE INDEX idx_orders_id ON "core"."orders" (id);
    SQL

    assert_equal 2, result.statements.length

    table = result.statements.fetch(0)
    assert_equal :create_table, table.kind
    assert_equal "CREATE TABLE IF NOT EXISTS \"core\".\"orders\" (id int)", table.sql.strip
    assert_equal "core", table.relation.schema
    assert_equal "orders", table.relation.name
    assert_equal true, table.if_not_exists
    assert_equal "T_CreateStmt", table.raw_node_type
    assert_equal "core.orders", table.target_name
    assert_equal [table.relation], table.target_relations

    index = result.statements.fetch(1)
    assert_equal :create_index, index.kind
    assert_equal "CREATE INDEX idx_orders_id ON \"core\".\"orders\" (id)", index.sql.strip
    assert_equal "core", index.relation.schema
    assert_equal "orders", index.relation.name
    assert_equal "idx_orders_id", index.index_name
    assert_equal "idx_orders_id", index.target_name
    assert_equal "core.orders", index.target_relation.qualified_name
  end

  def test_create_index_yugabyte_flags
    stmt = YbDDLParser.parse_one!(<<~SQL)
      CREATE UNIQUE INDEX IF NOT EXISTS idx_orders_shop_created
      ON "core"."orders"
      USING lsm ((shop_id) HASH, created_at DESC);
    SQL

    assert_equal :create_index, stmt.kind
    assert_equal "idx_orders_shop_created", stmt.index_name
    assert_equal "core", stmt.relation.schema
    assert_equal "orders", stmt.relation.name
    assert_equal true, stmt.unique
    assert_equal true, stmt.if_not_exists
    assert_equal "lsm", stmt.access_method
    assert_equal :implicit, stmt.concurrently
    assert_equal false, stmt.explicit_concurrently?
    assert_equal true, stmt.keys.first.hashed
    assert_equal ["shop_id", "created_at"], stmt.keys.map(&:name)
    assert_equal [true, false], stmt.keys.map(&:hash?)
    assert_equal [:hash, :desc], stmt.keys.map(&:order)
  end

  def test_create_index_nonconcurrently
    stmt = YbDDLParser.parse!("CREATE INDEX NONCONCURRENTLY idx_orders_id ON orders (id);").single_statement!

    assert_equal :create_index, stmt.kind
    assert_equal false, stmt.unique
    assert_equal false, stmt.if_not_exists
    assert_equal :disabled, stmt.concurrently
    assert_equal false, stmt.explicit_concurrently?
  end

  def test_drop_index_relation_and_flags
    stmt = YbDDLParser.parse!("DROP INDEX IF EXISTS core.idx_orders_id;").single_statement!

    assert_equal :drop, stmt.kind
    assert_equal :index, stmt.object_type
    assert_equal true, stmt.if_exists
    assert_equal :disabled, stmt.concurrently
    assert_equal true, stmt.drop_index?
    assert_equal false, stmt.drop_table?
    assert_equal 1, stmt.objects.length
    assert_equal "core", stmt.objects.first.schema
    assert_equal "idx_orders_id", stmt.objects.first.name
  end

  def test_alter_table_relation_and_object_type
    stmt = YbDDLParser.parse!("ALTER TABLE IF EXISTS core.orders ADD COLUMN name text;").single_statement!

    assert_equal :alter_table, stmt.kind
    assert_equal :table, stmt.object_type
    assert_equal true, stmt.if_exists
    assert_equal false, stmt.alter_index?
    assert_equal "core", stmt.relation.schema
    assert_equal "orders", stmt.relation.name
  end

  def test_create_table_columns_constraints_primary_key_and_split
    stmt = YbDDLParser.parse!(<<~SQL).single_statement!
      CREATE TABLE "core"."currency_conversion_rate_history" (
          "code" character varying(255) NOT NULL,
          "created_at" timestamp without time zone NOT NULL,
          "id" bigint NOT NULL,
          "rate" double precision DEFAULT 1.5,
          CONSTRAINT "currency_conversion_rate_history_pkey"
            PRIMARY KEY(("code") HASH, "created_at" ASC, "id" ASC),
          CONSTRAINT chk_code CHECK (octet_length(code) <= 255)
      ) SPLIT INTO 8 TABLETS;
    SQL

    assert_equal :create_table, stmt.kind
    assert_equal "core", stmt.relation.schema
    assert_equal "currency_conversion_rate_history", stmt.relation.name
    assert_equal %w[code created_at id rate], stmt.columns.map(&:name)
    assert_equal ["varchar", "timestamp", "int8", "float8"], stmt.columns.map(&:type)
    assert_equal [255], stmt.columns.first.typmods
    assert_equal :not_null, stmt.columns.first.constraints.first.type
    assert_equal :default, stmt.columns.last.constraints.first.type
    assert_equal "1.5", stmt.columns.last.constraints.first.raw_expression

    assert_equal "currency_conversion_rate_history_pkey", stmt.primary_key.name
    assert_equal %w[code created_at id], stmt.primary_key.columns
    assert_equal %w[code created_at id], stmt.primary_key.key_columns.map(&:name)
    assert_equal [true, false, false], stmt.primary_key.key_columns.map(&:hash?)
    assert_equal [:hash, :asc, :asc], stmt.primary_key.key_columns.map(&:order)

    check = stmt.constraints.detect { |constraint| constraint.type == :check }
    assert_equal "chk_code", check.name
    assert_equal "octet_length(code) <= 255", check.raw_expression
    assert_equal ["octet_length"], check.functions

    assert_equal YbDDLParser::AST::TabletSplit.new(type: :num_tablets, num_tablets: 8, points: nil), stmt.tablet_split
    assert_equal stmt.tablet_split, stmt.split
  end

  def test_create_index_keys_include_predicate_and_split
    stmt = YbDDLParser.parse!(<<~SQL).single_statement!
      CREATE UNIQUE INDEX idx_delivery_option_estimate_shop_variant_country_method_type
      ON "core"."delivery_option_estimate_snapshots"
      USING lsm ((shop_id) HASH, variant_id, country_code, method_type)
      INCLUDE (id)
      SPLIT INTO 4 TABLETS
      WHERE octet_length(country_code) = 2;
    SQL

    assert_equal :create_index, stmt.kind
    assert_equal true, stmt.unique
    assert_equal "lsm", stmt.access_method
    assert_equal %w[shop_id variant_id country_code method_type], stmt.keys.map(&:name)
    assert_equal [true, false, false, false], stmt.keys.map(&:hash?)
    assert_equal ["id"], stmt.include_columns
    assert_equal "octet_length(country_code) = 2", stmt.where_sql
    assert_equal YbDDLParser::AST::TabletSplit.new(type: :num_tablets, num_tablets: 4, points: nil), stmt.tablet_split
  end

  def test_split_at_values
    stmt = YbDDLParser.parse!("CREATE TABLE t (id int primary key) SPLIT AT VALUES ((1), (2));").single_statement!

    assert_equal :create_table, stmt.kind
    assert_equal YbDDLParser::AST::TabletSplit.new(type: :split_points, num_tablets: nil, points: [["1"], ["2"]]), stmt.tablet_split
    assert_equal :primary_key, stmt.columns.first.constraints.first.type
    assert_equal ["id"], stmt.columns.first.constraints.first.columns
  end

  def test_create_table_partition_of
    stmt = YbDDLParser.parse!(
      "CREATE TABLE core.orders_1 PARTITION OF core.orders FOR VALUES IN (1) TABLESPACE tsp;"
    ).single_statement!

    assert_equal :create_table, stmt.kind
    assert_equal "core.orders_1", stmt.relation.qualified_name
    assert_equal "core.orders", stmt.partition_of.qualified_name
    assert_equal "FOR VALUES IN (1)", stmt.partition_bound_sql
    assert_equal true, stmt.partition_child?
    assert_equal "tsp", stmt.tablespace
  end

  def test_partitioned_parent_split
    stmt = YbDDLParser.parse!(
      "CREATE TABLE core.orders (id int) PARTITION BY LIST (id) SPLIT INTO 1 TABLETS;"
    ).single_statement!

    assert_equal YbDDLParser::AST::PartitionSpec.new(strategy: :list, keys: ["id"]), stmt.partition_spec
    assert_equal stmt.partition_spec, stmt.partition
    assert_equal YbDDLParser::AST::TabletSplit.new(type: :num_tablets, num_tablets: 1, points: nil), stmt.tablet_split
    assert_nil stmt.partition_of
    assert_equal true, stmt.partition_parent?
    assert_equal false, stmt.partition_child?
  end

  def test_alter_table_add_and_drop_column_commands
    add = YbDDLParser.parse!("ALTER TABLE core.orders ADD COLUMN name text NOT NULL DEFAULT 'x';").single_statement!
    add_column = add.commands.first

    assert_equal :add_column, add_column.kind
    assert_equal "name", add_column.column.name
    assert_equal "text", add_column.column.type
    assert_equal [:not_null, :default], add_column.column.constraints.map(&:type)
    assert_equal "'x'", add_column.column.constraints.last.raw_expression

    drop = YbDDLParser.parse!("ALTER TABLE core.orders DROP COLUMN IF EXISTS old_name;").single_statement!
    drop_column = drop.commands.first

    assert_equal :drop_column, drop_column.kind
    assert_equal "old_name", drop_column.definition
    assert_equal true, drop_column.missing_ok
  end

  def test_drop_table_relation_and_flags
    stmt = YbDDLParser.parse!("DROP TABLE IF EXISTS core.orders;").single_statement!

    assert_equal :drop, stmt.kind
    assert_equal :table, stmt.object_type
    assert_equal true, stmt.if_exists
    assert_equal true, stmt.drop_table?
    assert_equal false, stmt.drop_index?
    assert_equal 1, stmt.objects.length
    assert_equal "core", stmt.objects.first.schema
    assert_equal "orders", stmt.objects.first.name
    assert_equal "core.orders", stmt.target_name
  end

  def test_drop_index_concurrently
    stmt = YbDDLParser.parse!("DROP INDEX CONCURRENTLY core.idx_orders;").single_statement!

    assert_equal :drop, stmt.kind
    assert_equal :index, stmt.object_type
    assert_equal "core.idx_orders", stmt.objects.first.qualified_name
    assert_equal :explicit, stmt.concurrently
    assert_equal true, stmt.explicit_concurrently?
  end

  def test_common_denied_statement_kinds
    truncate = YbDDLParser.parse!("TRUNCATE TABLE core.orders;").single_statement!
    assert_equal :truncate, truncate.kind
    assert_equal "T_TruncateStmt", truncate.raw_node_type
    assert_equal :table, truncate.object_type
    assert_equal "core.orders", truncate.objects.first.qualified_name
    assert_equal "core.orders", truncate.target_name

    comment = YbDDLParser.parse!("COMMENT ON TABLE core.orders IS 'x';").single_statement!
    assert_equal :comment, comment.kind
    assert_equal :table, comment.object_type
    assert_equal "core.orders", comment.relation.qualified_name

    rename = YbDDLParser.parse!("ALTER TABLE core.orders RENAME TO orders_old;").single_statement!
    assert_equal :rename, rename.kind
    assert_equal :table, rename.object_type
    assert_equal "core.orders", rename.relation.qualified_name
    assert_equal "orders_old", rename.new_name

    view = YbDDLParser.parse!("CREATE VIEW core.v AS SELECT 1;").single_statement!
    assert_equal :create_view, view.kind
    assert_equal "core.v", view.relation.qualified_name
  end

  def test_unknown_statement_exposes_raw_node_type
    stmt = YbDDLParser.parse!("SELECT 1;").single_statement!

    assert_equal :unknown, stmt.kind
    assert_equal "T_SelectStmt", stmt.raw_node_type
  end

  def test_create_schema_and_tablespace
    schema = YbDDLParser.parse!("CREATE SCHEMA IF NOT EXISTS core AUTHORIZATION alice;").single_statement!

    assert_equal :create_schema, schema.kind
    assert_equal "core", schema.name
    assert_equal "alice", schema.owner
    assert_equal true, schema.if_not_exists

    tablespace = YbDDLParser.parse!(<<~SQL).single_statement!
      CREATE TABLESPACE tsp_core WITH (replica_placement = '{"num_replicas":3}');
    SQL

    assert_equal :create_tablespace, tablespace.kind
    assert_equal "tsp_core", tablespace.name
    assert_equal "tsp_core", tablespace.target_name
    assert_equal '{"num_replicas":3}', tablespace.replica_placement_json
  end

  def test_invalid_sql_reports_parser_error
    result = YbDDLParser.parse("create table")

    assert_empty result.statements
    assert_equal YbDDLParser::AST::ParseDiagnostic, result.errors.first.class
    assert_equal "syntax error at end of input", result.errors.first.message
    assert_equal 13, result.errors.first.position
  end
end
