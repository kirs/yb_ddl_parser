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

  Split = Data.define(:type, :num_tablets, :points)

  Partition = Data.define(:strategy, :keys)

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
    :split,
    :partition,
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
      create_table? && !partition.nil? && !partition_child?
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

    def definition_sql
      return unless create_table? && sql

      offset = self.class.send(:create_table_definition_offset, sql)
      offset && sql[offset..]&.strip
    end

    def self.create_table_definition_offset(sql)
      index = 0
      word, index = read_word(sql, index)
      return unless word == "create"

      loop do
        word, next_index = read_word(sql, index)
        return unless word

        index = next_index
        break if word == "table"
      end

      word, next_index = read_word(sql, index)
      if word == "if"
        word, next_index = read_word(sql, next_index)
        return unless word == "not"

        word, next_index = read_word(sql, next_index)
        return unless word == "exists"

        index = next_index
      end

      index = read_qualified_name(sql, index)
      index && skip_space_and_comments(sql, index)
    end
    private_class_method :create_table_definition_offset

    def self.read_qualified_name(sql, index)
      index = read_identifier(sql, index)
      return unless index

      loop do
        index = skip_space_and_comments(sql, index)
        break if sql[index] != "."

        index = read_identifier(sql, index + 1)
        return unless index
      end

      index
    end
    private_class_method :read_qualified_name

    def self.read_word(sql, index)
      index = skip_space_and_comments(sql, index)
      start = index

      while index < sql.length && sql[index].match?(/[A-Za-z_]/)
        index += 1
      end

      return [nil, start] if index == start

      [sql[start...index].downcase, index]
    end
    private_class_method :read_word

    def self.read_identifier(sql, index)
      index = skip_space_and_comments(sql, index)

      if sql[index] == '"'
        index += 1
        while index < sql.length
          if sql[index] == '"'
            if sql[index + 1] == '"'
              index += 2
              next
            end

            return index + 1
          end
          index += 1
        end
        return
      end

      start = index
      while index < sql.length && sql[index].match?(/[A-Za-z0-9_$]/)
        index += 1
      end

      index == start ? nil : index
    end
    private_class_method :read_identifier

    def self.skip_space_and_comments(sql, index)
      loop do
        index += 1 while index < sql.length && sql[index].match?(/\s/)

        if sql[index, 2] == "--"
          newline = sql.index("\n", index + 2)
          index = newline ? newline + 1 : sql.length
        elsif sql[index, 2] == "/*"
          close = sql.index("*/", index + 2)
          index = close ? close + 2 : sql.length
        else
          return index
        end
      end
    end
    private_class_method :skip_space_and_comments
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
        split: split(hash[:split]),
        partition: partition(hash[:partition]),
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

    def self.split(hash)
      return unless hash

      Split.new(
        type: sym(hash[:type]),
        num_tablets: hash[:num_tablets],
        points: hash[:points],
      )
    end

    def self.partition(hash)
      return unless hash

      Partition.new(
        strategy: hash[:strategy],
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
