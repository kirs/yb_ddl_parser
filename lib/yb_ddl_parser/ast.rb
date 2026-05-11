# frozen_string_literal: true

module YbDDLParser
  module AST
    RelationName = Data.define(:schema, :name) do
      def qualified_name
        schema && !schema.empty? ? "#{schema}.#{name}" : name
      end
    end

    KeyColumn = Data.define(:name, :expression, :order, :nulls, :hashed, :hash_group) do
      def hash?
        hashed == true
      end
    end

    Constraint = Data.define(:type, :name, :columns, :key_columns, :raw_expression, :functions)

    Column = Data.define(:name, :type, :typmods, :constraints)

    Command = Data.define(:kind, :column, :definition, :constraint, :tablespace, :missing_ok)

    TabletSplit = Data.define(:type, :num_tablets, :points)

    PartitionSpec = Data.define(:strategy, :keys)

    ParseDiagnostic = Data.define(:message, :position)

    Statement = Data.define(
      :kind,
      :raw_node_type,
      :sql,
      :location,
      :relation,
      :object_type,
      :objects,
      :columns,
      :constraints,
      :primary_key,
      :commands,
      :index_name,
      :unique,
      :concurrently,
      :access_method,
      :keys,
      :include_columns,
      :where_sql,
      :tablespace,
      :tablet_split,
      :partition_spec,
      :partition_of,
      :partition_bound_sql,
      :if_exists,
      :if_not_exists,
      :name,
      :owner,
      :new_name,
      :tablespace_location,
      :replica_placement_json,
    ) do
      def create_table?
        kind == :create_table
      end

      def create_index?
        kind == :create_index
      end

      def drop_table?
        kind == :drop && object_type == :table
      end

      def drop_index?
        kind == :drop && object_type == :index
      end

      def alter_index?
        kind == :alter_index
      end

      def partition_parent?
        create_table? && !partition_spec.nil? && !partition_child?
      end

      def partition_child?
        !partition_of.nil?
      end

      def explicit_concurrently?
        concurrently == :explicit
      end

      def target_relations
        return objects unless objects.empty?
        return [relation] if relation

        []
      end

      def target_relation
        target_relations.first
      end

      def target_name
        index_name || target_relation&.qualified_name || name
      end

      def hash_key_groups
        groups = keys.each_with_object({}) do |key, grouped|
          next if key.hash_group.nil?

          grouped[key.hash_group] ||= []
          grouped[key.hash_group] << (key.name || key.expression)
        end

        groups.keys.sort.map { |group| groups.fetch(group) }
      end

      def split
        tablet_split
      end

      def partition
        partition_spec
      end
    end

    ParseResult = Data.define(:statements, :errors) do
      def self.from_hash(hash)
        new(
          statements: Array(hash[:statements]).map { |stmt| build_statement(stmt) },
          errors: Array(hash[:errors]).map { |error| diagnostic(error) },
        )
      end

      def single_statement!
        if statements.length != 1
          raise ParseError, "expected exactly one statement, got #{statements.length}"
        end

        statements.first
      end

      def each_statement(&block)
        statements.each(&block)
      end

      def each_column
        return enum_for(:each_column) unless block_given?

        statements.each do |stmt|
          stmt.columns.each { |column| yield column, stmt } if stmt.columns
          stmt.commands.each do |cmd|
            yield cmd.column, stmt if cmd.column
          end if stmt.commands
        end
      end

      def self.build_statement(hash)
        Statement.new(
          kind: sym(hash[:kind]),
          raw_node_type: hash[:raw_node_type],
          sql: hash[:sql],
          location: hash[:location],
          relation: relation(hash[:relation]),
          object_type: sym(hash[:object_type]),
          objects: Array(hash[:objects]).map { |obj| relation(obj) },
          columns: Array(hash[:columns]).map { |col| column(col) },
          constraints: Array(hash[:constraints]).map { |constraint_hash| constraint(constraint_hash) },
          primary_key: hash[:primary_key] && constraint(hash[:primary_key]),
          commands: Array(hash[:commands]).map { |cmd| command(cmd) },
          index_name: hash[:index_name],
          unique: hash[:unique],
          concurrently: sym(hash[:concurrently]),
          access_method: hash[:access_method],
          keys: Array(hash[:keys]).map { |key| key_column(key) },
          include_columns: Array(hash[:include_columns]),
          where_sql: hash[:where_sql],
          tablespace: hash[:tablespace],
          tablet_split: tablet_split(hash[:split]),
          partition_spec: partition_spec(hash[:partition]),
          partition_of: relation(hash[:partition_of]),
          partition_bound_sql: hash[:partition_bound_sql],
          if_exists: hash[:if_exists],
          if_not_exists: hash[:if_not_exists],
          name: hash[:name],
          owner: hash[:owner],
          new_name: hash[:new_name],
          tablespace_location: hash[:tablespace_location],
          replica_placement_json: hash[:replica_placement_json],
        )
      end

      def self.diagnostic(hash)
        ParseDiagnostic.new(
          message: hash[:message],
          position: hash[:position],
        )
      end

      def self.tablet_split(hash)
        return unless hash

        TabletSplit.new(
          type: sym(hash[:type]),
          num_tablets: hash[:num_tablets],
          points: hash[:points],
        )
      end

      def self.partition_spec(hash)
        return unless hash

        PartitionSpec.new(
          strategy: sym(hash[:strategy]),
          keys: Array(hash[:keys]),
        )
      end

      def self.command(hash)
        Command.new(
          kind: sym(hash[:kind]),
          column: hash[:column] && column(hash[:column]),
          definition: hash[:definition],
          constraint: hash[:constraint] && constraint(hash[:constraint]),
          tablespace: hash[:tablespace],
          missing_ok: hash[:missing_ok],
        )
      end

      def self.column(hash)
        Column.new(
          name: hash[:name],
          type: hash[:type],
          typmods: Array(hash[:typmods]),
          constraints: Array(hash[:constraints]).map { |constraint_hash| constraint(constraint_hash) },
        )
      end

      def self.constraint(hash)
        Constraint.new(
          type: sym(hash[:type]),
          name: hash[:name],
          columns: Array(hash[:columns]),
          key_columns: Array(hash[:key_columns]).map { |key| key_column(key) },
          raw_expression: hash[:raw_expression],
          functions: Array(hash[:functions]),
        )
      end

      def self.key_column(hash)
        KeyColumn.new(
          name: hash[:name],
          expression: hash[:expression],
          order: sym(hash[:order]),
          nulls: sym(hash[:nulls]),
          hashed: hash[:hash],
          hash_group: hash[:hash_group],
        )
      end

      def self.relation(hash)
        return unless hash

        RelationName.new(schema: hash[:schema], name: hash[:name])
      end

      def self.sym(value)
        value.nil? ? nil : value.to_sym
      end
    end
  end

  Statement = AST::Statement
  ParseResult = AST::ParseResult
end
