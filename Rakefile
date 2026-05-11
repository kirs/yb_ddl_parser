# frozen_string_literal: true

require "rake/extensiontask"
require "rake/testtask"

Rake::ExtensionTask.new("yb_ddl_parser_ext") do |ext|
  ext.ext_dir = "ext/yb_ddl_parser"
  ext.lib_dir = "lib/yb_ddl_parser"
end

Rake::TestTask.new do |t|
  t.libs << "ext"
  t.libs << "test"
  t.pattern = "test/**/*_test.rb"
end

task default: %i[compile test]
