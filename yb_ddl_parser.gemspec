# frozen_string_literal: true

Gem::Specification.new do |spec|
  spec.name = "yb_ddl_parser"
  spec.version = File.read(File.join(__dir__, "VERSION")).strip
  spec.summary = "Small YugabyteDB DDL parser for migration tooling"
  spec.authors = ["Kirs"]
  spec.files = Dir[
    "LICENSE",
    "LICENSES/**/*.txt",
    "README.md",
    "THIRD_PARTY_NOTICES.md",
    "VERSION",
    "lib/**/*.rb",
    "ext/**/*.{c,h,rb}",
    "test/**/*.rb",
  ]
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/yb_ddl_parser/extconf.rb"]
  spec.required_ruby_version = ">= 3.2"
  spec.licenses = ["Apache-2.0", "PostgreSQL"]

  spec.add_development_dependency "minitest"
  spec.add_development_dependency "rake-compiler"
end
