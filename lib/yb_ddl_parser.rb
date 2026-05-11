# frozen_string_literal: true

require_relative "yb_ddl_parser/version"
require_relative "yb_ddl_parser/ast"
require "yb_ddl_parser/yb_ddl_parser_ext"

module YbDDLParser
  class ParseError < StandardError
    attr_reader :position

    def initialize(message, position: nil)
      @position = position
      super(message)
    end
  end

  def self.parse(sql)
    ParseResult.from_hash(Native.parse(sql.to_s))
  end

  def self.parse_one!(sql)
    parse!(sql).single_statement!
  end

  def self.parse!(sql)
    result = parse(sql)
    error = result.errors.first
    raise ParseError.new(error.message, position: error.position) if error

    result
  end
end
