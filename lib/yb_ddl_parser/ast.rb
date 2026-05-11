# frozen_string_literal: true

module YbDDLParser
  RelationName = Data.define(:schema, :name) do
    def qualified_name
      schema && !schema.empty? ? "#{schema}.#{name}" : name
    end
  end

  KeyColumn = Data.define(:name, :expression, :order, :nulls, :hash) do
    def hash?
      hash == true
    end
  end

  Constraint = Data.define(:type, :name, :columns, :key_columns, :raw_expression, :functions)

  Column = Data.define(:name, :type, :typmods, :constraints)

  Command = Data.define(:kind, :column, :definition, :constraint, :tablespace, :missing_ok)

  Statement = Data.define(
    :kind,
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
    :split,
    :partition,
    :if_exists,
    :if_not_exists,
    :name,
    :owner,
    :tablespace_location,
    :replica_placement_json,
  ) do
    def create_table?
      kind == :create_table
    end

    def create_index?
      kind == :create_index
    end
  end

  ParseResult = Data.define(:statements, :errors) do
    def self.from_hash(hash)
      new(
        statements: Array(hash[:statements]).map { |stmt| build_statement(stmt) },
        errors: Array(hash[:errors]),
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
        split: hash[:split],
        partition: hash[:partition],
        if_exists: hash[:if_exists],
        if_not_exists: hash[:if_not_exists],
        name: hash[:name],
        owner: hash[:owner],
        tablespace_location: hash[:tablespace_location],
        replica_placement_json: hash[:replica_placement_json],
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
        hash: hash[:hash],
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

