#include "ruby.h"
#include "ruby/encoding.h"

#include "postgres.h"
#include "nodes/pg_list.h"
#include "nodes/parsenodes.h"
#include "nodes/value.h"
#include "parser/parser.h"
#include "utils/elog.h"

const char *yb_parser_last_error_message(void);
int yb_parser_last_error_position(void);

/*
 * Native extension entrypoint.
 *
 * This file is intentionally thin. The parser implementation for this gem
 * should be YugabyteDB's vendored PostgreSQL parser, not a separate SQL parser.
 *
 * Planned C boundary:
 *   1. call raw_parser(sql, RAW_PARSE_DEFAULT) from vendored YB postgres code
 *   2. walk the returned RawStmt nodes
 *   3. extract only the stable DDL facts exposed by lib/yb_ddl_parser/ast.rb
 *
 */

static VALUE
symbol_key(const char *name)
{
  return ID2SYM(rb_intern(name));
}

static VALUE
symbol_value(const char *name)
{
  return ID2SYM(rb_intern(name));
}

static void
hash_set_cstr(VALUE hash, const char *key, const char *value)
{
  if (value != NULL)
    rb_hash_aset(hash, symbol_key(key), rb_str_new_cstr(value));
}

static void
hash_set_bool(VALUE hash, const char *key, bool value)
{
  rb_hash_aset(hash, symbol_key(key), value ? Qtrue : Qfalse);
}

static VALUE
statement_sql(VALUE sql, RawStmt *raw_stmt)
{
  long sql_len = RSTRING_LEN(sql);
  long location = raw_stmt->stmt_location >= 0 ? raw_stmt->stmt_location : 0;
  long len = raw_stmt->stmt_len > 0 ? raw_stmt->stmt_len : sql_len - location;

  if (location < 0 || location > sql_len)
    return Qnil;

  if (len < 0)
    len = 0;
  if (location + len > sql_len)
    len = sql_len - location;

  VALUE slice = rb_str_new(RSTRING_PTR(sql) + location, len);
  rb_enc_copy(slice, sql);
  return slice;
}

static const char *
statement_kind(Node *stmt)
{
  if (stmt == NULL)
    return "unknown";

  switch (nodeTag(stmt))
  {
    case T_CreateStmt:
      return "create_table";
    case T_IndexStmt:
      return "create_index";
    case T_AlterTableStmt:
      return castNode(AlterTableStmt, stmt)->objtype == OBJECT_INDEX
               ? "alter_index"
               : "alter_table";
    case T_DropStmt:
      return "drop";
    case T_CreateSchemaStmt:
      return "create_schema";
    case T_CreateTableSpaceStmt:
      return "create_tablespace";
    case T_TruncateStmt:
      return "truncate";
    case T_CommentStmt:
      return "comment";
    case T_RenameStmt:
      return "rename";
    case T_ViewStmt:
      return "create_view";
    default:
      return "unknown";
  }
}

static const char *
raw_node_type_name(Node *stmt)
{
  if (stmt == NULL)
    return NULL;

  switch (nodeTag(stmt))
  {
    case T_SelectStmt:
      return "T_SelectStmt";
    case T_ReturnStmt:
      return "T_ReturnStmt";
    case T_PLAssignStmt:
      return "T_PLAssignStmt";
    case T_AlterTableStmt:
      return "T_AlterTableStmt";
    case T_AlterTableCmd:
      return "T_AlterTableCmd";
    case T_AlterDomainStmt:
      return "T_AlterDomainStmt";
    case T_SetOperationStmt:
      return "T_SetOperationStmt";
    case T_GrantStmt:
      return "T_GrantStmt";
    case T_GrantRoleStmt:
      return "T_GrantRoleStmt";
    case T_AlterDefaultPrivilegesStmt:
      return "T_AlterDefaultPrivilegesStmt";
    case T_ClosePortalStmt:
      return "T_ClosePortalStmt";
    case T_ClusterStmt:
      return "T_ClusterStmt";
    case T_CopyStmt:
      return "T_CopyStmt";
    case T_CreateStmt:
      return "T_CreateStmt";
    case T_DefineStmt:
      return "T_DefineStmt";
    case T_DropStmt:
      return "T_DropStmt";
    case T_TruncateStmt:
      return "T_TruncateStmt";
    case T_CommentStmt:
      return "T_CommentStmt";
    case T_FetchStmt:
      return "T_FetchStmt";
    case T_IndexStmt:
      return "T_IndexStmt";
    case T_CreateFunctionStmt:
      return "T_CreateFunctionStmt";
    case T_AlterFunctionStmt:
      return "T_AlterFunctionStmt";
    case T_DoStmt:
      return "T_DoStmt";
    case T_RenameStmt:
      return "T_RenameStmt";
    case T_RuleStmt:
      return "T_RuleStmt";
    case T_NotifyStmt:
      return "T_NotifyStmt";
    case T_ListenStmt:
      return "T_ListenStmt";
    case T_UnlistenStmt:
      return "T_UnlistenStmt";
    case T_TransactionStmt:
      return "T_TransactionStmt";
    case T_ViewStmt:
      return "T_ViewStmt";
    case T_LoadStmt:
      return "T_LoadStmt";
    case T_CreateDomainStmt:
      return "T_CreateDomainStmt";
    case T_CreatedbStmt:
      return "T_CreatedbStmt";
    case T_DropdbStmt:
      return "T_DropdbStmt";
    case T_VacuumStmt:
      return "T_VacuumStmt";
    case T_ExplainStmt:
      return "T_ExplainStmt";
    case T_CreateTableAsStmt:
      return "T_CreateTableAsStmt";
    case T_CreateSeqStmt:
      return "T_CreateSeqStmt";
    case T_AlterSeqStmt:
      return "T_AlterSeqStmt";
    case T_VariableSetStmt:
      return "T_VariableSetStmt";
    case T_VariableShowStmt:
      return "T_VariableShowStmt";
    case T_DiscardStmt:
      return "T_DiscardStmt";
    case T_CreateTrigStmt:
      return "T_CreateTrigStmt";
    case T_CreatePLangStmt:
      return "T_CreatePLangStmt";
    case T_CreateRoleStmt:
      return "T_CreateRoleStmt";
    case T_AlterRoleStmt:
      return "T_AlterRoleStmt";
    case T_DropRoleStmt:
      return "T_DropRoleStmt";
    case T_LockStmt:
      return "T_LockStmt";
    case T_ConstraintsSetStmt:
      return "T_ConstraintsSetStmt";
    case T_ReindexStmt:
      return "T_ReindexStmt";
    case T_YbBackfillIndexStmt:
      return "T_YbBackfillIndexStmt";
    case T_CheckPointStmt:
      return "T_CheckPointStmt";
    case T_CreateSchemaStmt:
      return "T_CreateSchemaStmt";
    case T_AlterDatabaseStmt:
      return "T_AlterDatabaseStmt";
    case T_AlterDatabaseRefreshCollStmt:
      return "T_AlterDatabaseRefreshCollStmt";
    case T_AlterDatabaseSetStmt:
      return "T_AlterDatabaseSetStmt";
    case T_AlterRoleSetStmt:
      return "T_AlterRoleSetStmt";
    case T_CreateConversionStmt:
      return "T_CreateConversionStmt";
    case T_CreateCastStmt:
      return "T_CreateCastStmt";
    case T_CreateOpClassStmt:
      return "T_CreateOpClassStmt";
    case T_CreateOpFamilyStmt:
      return "T_CreateOpFamilyStmt";
    case T_AlterOpFamilyStmt:
      return "T_AlterOpFamilyStmt";
    case T_PrepareStmt:
      return "T_PrepareStmt";
    case T_ExecuteStmt:
      return "T_ExecuteStmt";
    case T_DeallocateStmt:
      return "T_DeallocateStmt";
    case T_DeclareCursorStmt:
      return "T_DeclareCursorStmt";
    case T_YbCreateTableGroupStmt:
      return "T_YbCreateTableGroupStmt";
    case T_CreateTableSpaceStmt:
      return "T_CreateTableSpaceStmt";
    case T_DropTableSpaceStmt:
      return "T_DropTableSpaceStmt";
    case T_AlterObjectDependsStmt:
      return "T_AlterObjectDependsStmt";
    case T_AlterObjectSchemaStmt:
      return "T_AlterObjectSchemaStmt";
    case T_AlterOwnerStmt:
      return "T_AlterOwnerStmt";
    case T_AlterOperatorStmt:
      return "T_AlterOperatorStmt";
    case T_AlterTypeStmt:
      return "T_AlterTypeStmt";
    case T_DropOwnedStmt:
      return "T_DropOwnedStmt";
    case T_ReassignOwnedStmt:
      return "T_ReassignOwnedStmt";
    case T_CompositeTypeStmt:
      return "T_CompositeTypeStmt";
    case T_CreateEnumStmt:
      return "T_CreateEnumStmt";
    case T_CreateRangeStmt:
      return "T_CreateRangeStmt";
    case T_AlterEnumStmt:
      return "T_AlterEnumStmt";
    case T_AlterTSDictionaryStmt:
      return "T_AlterTSDictionaryStmt";
    case T_AlterTSConfigurationStmt:
      return "T_AlterTSConfigurationStmt";
    case T_CreateFdwStmt:
      return "T_CreateFdwStmt";
    case T_AlterFdwStmt:
      return "T_AlterFdwStmt";
    case T_CreateForeignServerStmt:
      return "T_CreateForeignServerStmt";
    case T_AlterForeignServerStmt:
      return "T_AlterForeignServerStmt";
    case T_CreateUserMappingStmt:
      return "T_CreateUserMappingStmt";
    case T_AlterUserMappingStmt:
      return "T_AlterUserMappingStmt";
    case T_DropUserMappingStmt:
      return "T_DropUserMappingStmt";
    case T_AlterTableSpaceOptionsStmt:
      return "T_AlterTableSpaceOptionsStmt";
    case T_AlterTableMoveAllStmt:
      return "T_AlterTableMoveAllStmt";
    case T_SecLabelStmt:
      return "T_SecLabelStmt";
    case T_CreateForeignTableStmt:
      return "T_CreateForeignTableStmt";
    case T_ImportForeignSchemaStmt:
      return "T_ImportForeignSchemaStmt";
    case T_CreateExtensionStmt:
      return "T_CreateExtensionStmt";
    case T_AlterExtensionStmt:
      return "T_AlterExtensionStmt";
    case T_AlterExtensionContentsStmt:
      return "T_AlterExtensionContentsStmt";
    case T_CreateEventTrigStmt:
      return "T_CreateEventTrigStmt";
    case T_AlterEventTrigStmt:
      return "T_AlterEventTrigStmt";
    case T_RefreshMatViewStmt:
      return "T_RefreshMatViewStmt";
    case T_ReplicaIdentityStmt:
      return "T_ReplicaIdentityStmt";
    case T_AlterSystemStmt:
      return "T_AlterSystemStmt";
    case T_CreatePolicyStmt:
      return "T_CreatePolicyStmt";
    case T_AlterPolicyStmt:
      return "T_AlterPolicyStmt";
    case T_CreateTransformStmt:
      return "T_CreateTransformStmt";
    case T_CreateAmStmt:
      return "T_CreateAmStmt";
    case T_CreatePublicationStmt:
      return "T_CreatePublicationStmt";
    case T_AlterPublicationStmt:
      return "T_AlterPublicationStmt";
    case T_CreateSubscriptionStmt:
      return "T_CreateSubscriptionStmt";
    case T_AlterSubscriptionStmt:
      return "T_AlterSubscriptionStmt";
    case T_DropSubscriptionStmt:
      return "T_DropSubscriptionStmt";
    case T_CreateStatsStmt:
      return "T_CreateStatsStmt";
    case T_AlterCollationStmt:
      return "T_AlterCollationStmt";
    case T_CallStmt:
      return "T_CallStmt";
    case T_AlterStatsStmt:
      return "T_AlterStatsStmt";
    default:
      return "T_Unknown";
  }
}

static const char *
object_type_name(ObjectType type)
{
  switch (type)
  {
    case OBJECT_INDEX:
      return "index";
    case OBJECT_SCHEMA:
      return "schema";
    case OBJECT_TABLE:
      return "table";
    case OBJECT_TABLESPACE:
      return "tablespace";
    case OBJECT_VIEW:
      return "view";
    case OBJECT_SEQUENCE:
      return "sequence";
    default:
      return "unknown";
  }
}

static const char *
concurrency_name(YbConcurrencyContext concurrent)
{
  switch (concurrent)
  {
    case YB_CONCURRENCY_DISABLED:
      return "disabled";
    case YB_CONCURRENCY_IMPLICIT_ENABLED:
      return "implicit";
    case YB_CONCURRENCY_EXPLICIT_ENABLED:
      return "explicit";
    default:
      return "unknown";
  }
}

static VALUE
role_name(RoleSpec *role)
{
  if (role == NULL)
    return Qnil;

  switch (role->roletype)
  {
    case ROLESPEC_CSTRING:
      return role->rolename ? rb_str_new_cstr(role->rolename) : Qnil;
    case ROLESPEC_CURRENT_ROLE:
      return rb_str_new_cstr("current_role");
    case ROLESPEC_CURRENT_USER:
      return rb_str_new_cstr("current_user");
    case ROLESPEC_SESSION_USER:
      return rb_str_new_cstr("session_user");
    case ROLESPEC_PUBLIC:
      return rb_str_new_cstr("public");
    default:
      return Qnil;
  }
}

static VALUE
build_relation_hash_parts(const char *schema, const char *name)
{
  if (name == NULL)
    return Qnil;

  VALUE relation = rb_hash_new();
  if (schema != NULL)
    rb_hash_aset(relation, symbol_key("schema"), rb_str_new_cstr(schema));
  rb_hash_aset(relation, symbol_key("name"), rb_str_new_cstr(name));
  return relation;
}

static VALUE
build_relation_hash(RangeVar *relation)
{
  if (relation == NULL)
    return Qnil;

  return build_relation_hash_parts(relation->schemaname, relation->relname);
}

static VALUE
build_relation_hash_from_name_list(List *names)
{
  int len = list_length(names);
  const char *schema = NULL;
  const char *name = NULL;
  int index = 0;
  ListCell *cell;

  foreach(cell, names)
  {
    Node *node = (Node *) lfirst(cell);
    if (node == NULL || !IsA(node, String))
      return Qnil;

    if (index == len - 2)
      schema = strVal(node);
    if (index == len - 1)
      name = strVal(node);
    index++;
  }

  return build_relation_hash_parts(schema, name);
}

static VALUE
build_relation_array_from_name_lists(List *objects)
{
  VALUE array = rb_ary_new_capa(list_length(objects));
  ListCell *cell;

  foreach(cell, objects)
  {
    Node *node = (Node *) lfirst(cell);
    VALUE relation = Qnil;

    if (node != NULL && IsA(node, List))
      relation = build_relation_hash_from_name_list((List *) node);

    if (!NIL_P(relation))
      rb_ary_push(array, relation);
  }

  return array;
}

static VALUE
build_relation_array_from_range_vars(List *relations)
{
  VALUE array = rb_ary_new_capa(list_length(relations));
  ListCell *cell;

  foreach(cell, relations)
  {
    Node *node = (Node *) lfirst(cell);
    VALUE relation = Qnil;

    if (node != NULL && IsA(node, RangeVar))
      relation = build_relation_hash(castNode(RangeVar, node));

    if (!NIL_P(relation))
      rb_ary_push(array, relation);
  }

  return array;
}

static VALUE
first_relation_hash_from_range_vars(List *relations)
{
  if (relations == NIL)
    return Qnil;

  Node *node = (Node *) linitial(relations);
  if (node == NULL || !IsA(node, RangeVar))
    return Qnil;

  return build_relation_hash(castNode(RangeVar, node));
}

static VALUE expression_string(Node *node);
static void collect_expression_functions(Node *node, VALUE functions);
static void set_constraint_columns_for_column(VALUE constraint,
                                              const char *column_name);

static VALUE
name_list_string(List *names)
{
  if (names == NIL)
    return Qnil;

  VALUE out = rb_str_new("", 0);
  ListCell *cell;
  int index = 0;

  foreach(cell, names)
  {
    Node *node = (Node *) lfirst(cell);
    if (node == NULL || !IsA(node, String))
      return Qnil;

    const char *part = strVal(node);
    if (index == 0 && strcmp(part, "pg_catalog") == 0 &&
        list_length(names) > 1)
    {
      index++;
      continue;
    }

    if (RSTRING_LEN(out) > 0)
      rb_str_cat_cstr(out, ".");
    rb_str_cat_cstr(out, part);
    index++;
  }

  return out;
}

static VALUE
string_node_array(List *nodes)
{
  VALUE array = rb_ary_new_capa(list_length(nodes));
  ListCell *cell;

  foreach(cell, nodes)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, String))
      rb_ary_push(array, rb_str_new_cstr(strVal(node)));
  }

  return array;
}

static VALUE
literal_value(Node *node)
{
  if (node == NULL)
    return Qnil;

  switch (nodeTag(node))
  {
    case T_Integer:
      return INT2NUM(intVal(node));
    case T_Float:
      return rb_str_new_cstr(castNode(Float, node)->fval);
    case T_String:
      return rb_str_new_cstr(strVal(node));
    case T_Boolean:
      return boolVal(node) ? Qtrue : Qfalse;
    default:
      return Qnil;
  }
}

static VALUE
literal_string(Node *node)
{
  if (node == NULL)
    return Qnil;

  if (IsA(node, String))
  {
    VALUE out = rb_str_new_cstr("'");
    rb_str_cat_cstr(out, strVal(node));
    rb_str_cat_cstr(out, "'");
    return out;
  }

  VALUE value = literal_value(node);
  if (NIL_P(value))
    return Qnil;
  return rb_funcall(value, rb_intern("to_s"), 0);
}

static VALUE
constant_string(A_Const *constant)
{
  if (constant->isnull)
    return rb_str_new_cstr("NULL");

  return literal_string(&constant->val.node);
}

static VALUE
typmods_array(TypeName *type_name)
{
  VALUE typmods = rb_ary_new();
  if (type_name == NULL)
    return typmods;

  ListCell *cell;
  foreach(cell, type_name->typmods)
  {
    Node *node = (Node *) lfirst(cell);
    VALUE value = Qnil;

    if (node == NULL)
      continue;

    if (IsA(node, A_Const))
    {
      A_Const *constant = castNode(A_Const, node);
      if (!constant->isnull)
        value = literal_value(&constant->val.node);
    }
    else
    {
      value = literal_value(node);
    }

    if (!NIL_P(value))
      rb_ary_push(typmods, value);
  }

  return typmods;
}

static VALUE
type_name_string(TypeName *type_name)
{
  if (type_name == NULL)
    return Qnil;

  return name_list_string(type_name->names);
}

static const char *
constraint_type_name(ConstrType type)
{
  switch (type)
  {
    case CONSTR_NULL:
      return "null";
    case CONSTR_NOTNULL:
      return "not_null";
    case CONSTR_DEFAULT:
      return "default";
    case CONSTR_CHECK:
      return "check";
    case CONSTR_PRIMARY:
      return "primary_key";
    case CONSTR_UNIQUE:
      return "unique";
    case CONSTR_FOREIGN:
      return "foreign_key";
    default:
      return "unknown";
  }
}

static const char *
alter_command_kind(AlterTableType type)
{
  switch (type)
  {
    case AT_AddColumn:
    case AT_AddColumnRecurse:
      return "add_column";
    case AT_DropColumn:
    case AT_DropColumnRecurse:
      return "drop_column";
    case AT_AddConstraint:
    case AT_AddConstraintRecurse:
      return "add_constraint";
    case AT_DropConstraint:
    case AT_DropConstraintRecurse:
      return "drop_constraint";
    case AT_AlterColumnType:
      return "alter_column_type";
    case AT_SetTableSpace:
      return "set_tablespace";
    case AT_ColumnDefault:
      return "set_default";
    case AT_DropNotNull:
      return "drop_not_null";
    case AT_SetNotNull:
      return "set_not_null";
    default:
      return "unknown";
  }
}

static const char *
sort_order_name(SortByDir ordering)
{
  switch (ordering)
  {
    case SORTBY_ASC:
      return "asc";
    case SORTBY_DESC:
      return "desc";
    case SORTBY_HASH:
      return "hash";
    case SORTBY_DEFAULT:
      return "default";
    default:
      return "unknown";
  }
}

static const char *
nulls_order_name(SortByNulls ordering)
{
  switch (ordering)
  {
    case SORTBY_NULLS_FIRST:
      return "first";
    case SORTBY_NULLS_LAST:
      return "last";
    case SORTBY_NULLS_DEFAULT:
      return "default";
    default:
      return "unknown";
  }
}

static VALUE
column_ref_string(ColumnRef *column_ref)
{
  return name_list_string(column_ref->fields);
}

static VALUE
function_call_string(FuncCall *call)
{
  VALUE out = name_list_string(call->funcname);
  if (NIL_P(out))
    out = rb_str_new_cstr("function");

  rb_str_cat_cstr(out, "(");

  if (call->agg_star)
  {
    rb_str_cat_cstr(out, "*");
  }
  else
  {
    ListCell *cell;
    int index = 0;
    foreach(cell, call->args)
    {
      VALUE arg = expression_string((Node *) lfirst(cell));
      if (NIL_P(arg))
        continue;

      if (index > 0)
        rb_str_cat_cstr(out, ", ");
      rb_str_append(out, arg);
      index++;
    }
  }

  rb_str_cat_cstr(out, ")");
  return out;
}

static VALUE
operator_name_string(List *names)
{
  if (names == NIL)
    return rb_str_new_cstr("?");

  Node *last = (Node *) llast(names);
  if (last != NULL && IsA(last, String))
    return rb_str_new_cstr(strVal(last));

  return rb_str_new_cstr("?");
}

static VALUE
expression_string(Node *node)
{
  if (node == NULL)
    return Qnil;

  switch (nodeTag(node))
  {
    case T_A_Const:
      return constant_string(castNode(A_Const, node));
    case T_ColumnRef:
      return column_ref_string(castNode(ColumnRef, node));
    case T_FuncCall:
      return function_call_string(castNode(FuncCall, node));
    case T_A_Expr:
    {
      A_Expr *expr = castNode(A_Expr, node);
      VALUE op = operator_name_string(expr->name);
      VALUE left = expression_string(expr->lexpr);
      VALUE right = expression_string(expr->rexpr);

      if (!NIL_P(left) && !NIL_P(right))
      {
        VALUE out = rb_str_dup(left);
        rb_str_cat_cstr(out, " ");
        rb_str_append(out, op);
        rb_str_cat_cstr(out, " ");
        rb_str_append(out, right);
        return out;
      }
      if (!NIL_P(right))
      {
        VALUE out = rb_str_dup(op);
        rb_str_cat_cstr(out, " ");
        rb_str_append(out, right);
        return out;
      }
      if (!NIL_P(left))
      {
        VALUE out = rb_str_dup(left);
        rb_str_cat_cstr(out, " ");
        rb_str_append(out, op);
        return out;
      }
      return op;
    }
    case T_TypeCast:
    {
      TypeCast *cast = castNode(TypeCast, node);
      VALUE arg = expression_string(cast->arg);
      VALUE type_name = type_name_string(cast->typeName);
      if (!NIL_P(arg) && !NIL_P(type_name))
      {
        VALUE out = rb_str_dup(arg);
        rb_str_cat_cstr(out, "::");
        rb_str_append(out, type_name);
        return out;
      }
      return arg;
    }
    default:
      return Qnil;
  }
}

static void
add_function_name(VALUE functions, VALUE name)
{
  if (NIL_P(name))
    return;

  if (rb_funcall(functions, rb_intern("include?"), 1, name) == Qfalse)
    rb_ary_push(functions, name);
}

static void
collect_expression_functions(Node *node, VALUE functions)
{
  if (node == NULL)
    return;

  switch (nodeTag(node))
  {
    case T_FuncCall:
    {
      FuncCall *call = castNode(FuncCall, node);
      add_function_name(functions, name_list_string(call->funcname));

      ListCell *cell;
      foreach(cell, call->args)
      {
        collect_expression_functions((Node *) lfirst(cell), functions);
      }
      break;
    }
    case T_A_Expr:
    {
      A_Expr *expr = castNode(A_Expr, node);
      collect_expression_functions(expr->lexpr, functions);
      collect_expression_functions(expr->rexpr, functions);
      break;
    }
    case T_TypeCast:
      collect_expression_functions(castNode(TypeCast, node)->arg, functions);
      break;
    default:
      break;
  }
}

static VALUE
defelem_arg_value(DefElem *defelem)
{
  if (defelem == NULL || defelem->arg == NULL)
    return Qnil;

  switch (nodeTag(defelem->arg))
  {
    case T_String:
      return rb_str_new_cstr(strVal(defelem->arg));
    case T_Integer:
      return INT2NUM(intVal(defelem->arg));
    case T_Float:
      return rb_str_new_cstr(castNode(Float, defelem->arg)->fval);
    case T_Boolean:
      return boolVal(defelem->arg) ? Qtrue : Qfalse;
    default:
      return Qnil;
  }
}

static VALUE
find_option_value(List *options, const char *name)
{
  ListCell *cell;

  foreach(cell, options)
  {
    DefElem *defelem = castNode(DefElem, lfirst(cell));
    if (defelem->defname != NULL && strcmp(defelem->defname, name) == 0)
      return defelem_arg_value(defelem);
  }

  return Qnil;
}

static VALUE
build_key_column(IndexElem *elem)
{
  VALUE key = rb_hash_new();
  VALUE expr = expression_string(elem->expr);

  hash_set_cstr(key, "name", elem->name);
  if (!NIL_P(expr))
    rb_hash_aset(key, symbol_key("expression"), expr);
  rb_hash_aset(key, symbol_key("order"),
               symbol_value(sort_order_name(elem->ordering)));
  rb_hash_aset(key, symbol_key("nulls"),
               symbol_value(nulls_order_name(elem->nulls_ordering)));
  hash_set_bool(key, "hash", elem->ordering == SORTBY_HASH);

  return key;
}

static VALUE
build_key_columns(List *elems)
{
  VALUE keys = rb_ary_new_capa(list_length(elems));
  ListCell *cell;

  foreach(cell, elems)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, IndexElem))
      rb_ary_push(keys, build_key_column(castNode(IndexElem, node)));
  }

  return keys;
}

static VALUE
build_include_columns(List *elems)
{
  VALUE columns = rb_ary_new_capa(list_length(elems));
  ListCell *cell;

  foreach(cell, elems)
  {
    Node *node = (Node *) lfirst(cell);
    if (node == NULL || !IsA(node, IndexElem))
      continue;

    IndexElem *elem = castNode(IndexElem, node);
    if (elem->name != NULL)
      rb_ary_push(columns, rb_str_new_cstr(elem->name));
    else
    {
      VALUE expr = expression_string(elem->expr);
      if (!NIL_P(expr))
        rb_ary_push(columns, expr);
    }
  }

  return columns;
}

static VALUE
build_constraint(Constraint *constraint)
{
  VALUE hash = rb_hash_new();
  VALUE functions = rb_ary_new();

  rb_hash_aset(hash, symbol_key("type"),
               symbol_value(constraint_type_name(constraint->contype)));
  hash_set_cstr(hash, "name", constraint->conname);

  VALUE columns = string_node_array(constraint->keys);
  if (constraint->fk_attrs != NIL)
    columns = string_node_array(constraint->fk_attrs);
  rb_hash_aset(hash, symbol_key("columns"), columns);

  if (constraint->yb_index_params != NIL)
    rb_hash_aset(hash, symbol_key("key_columns"),
                 build_key_columns(constraint->yb_index_params));

  if (constraint->raw_expr != NULL)
  {
    VALUE raw_expression = expression_string(constraint->raw_expr);
    if (!NIL_P(raw_expression))
      rb_hash_aset(hash, symbol_key("raw_expression"), raw_expression);
    collect_expression_functions(constraint->raw_expr, functions);
  }
  else if (constraint->cooked_expr != NULL)
  {
    rb_hash_aset(hash, symbol_key("raw_expression"),
                 rb_str_new_cstr(constraint->cooked_expr));
  }

  rb_hash_aset(hash, symbol_key("functions"), functions);

  return hash;
}

static VALUE
build_constraints(List *constraints)
{
  VALUE array = rb_ary_new_capa(list_length(constraints));
  ListCell *cell;

  foreach(cell, constraints)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, Constraint))
      rb_ary_push(array, build_constraint(castNode(Constraint, node)));
  }

  return array;
}

static VALUE
build_table_constraints(List *table_elts, List *constraints)
{
  VALUE array = build_constraints(constraints);
  ListCell *cell;

  foreach(cell, table_elts)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, Constraint))
      rb_ary_push(array, build_constraint(castNode(Constraint, node)));
  }

  return array;
}

static VALUE
build_default_constraint(Node *default_expr)
{
  VALUE hash = rb_hash_new();
  VALUE functions = rb_ary_new();
  VALUE raw_expression = expression_string(default_expr);

  rb_hash_aset(hash, symbol_key("type"), symbol_value("default"));
  if (!NIL_P(raw_expression))
    rb_hash_aset(hash, symbol_key("raw_expression"), raw_expression);
  collect_expression_functions(default_expr, functions);
  rb_hash_aset(hash, symbol_key("functions"), functions);

  return hash;
}

static VALUE
build_not_null_constraint(void)
{
  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, symbol_key("type"), symbol_value("not_null"));
  return hash;
}

static bool
constraints_include_type(List *constraints, ConstrType type)
{
  ListCell *cell;

  foreach(cell, constraints)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, Constraint) &&
        castNode(Constraint, node)->contype == type)
      return true;
  }

  return false;
}

static VALUE
build_column(ColumnDef *column)
{
  VALUE hash = rb_hash_new();
  VALUE constraints = build_constraints(column->constraints);
  long i;

  hash_set_cstr(hash, "name", column->colname);
  rb_hash_aset(hash, symbol_key("type"), type_name_string(column->typeName));
  rb_hash_aset(hash, symbol_key("typmods"), typmods_array(column->typeName));

  for (i = 0; i < RARRAY_LEN(constraints); i++)
  {
    set_constraint_columns_for_column(rb_ary_entry(constraints, i),
                                      column->colname);
  }

  if (column->is_not_null &&
      !constraints_include_type(column->constraints, CONSTR_NOTNULL))
    rb_ary_push(constraints, build_not_null_constraint());

  if (column->raw_default != NULL &&
      !constraints_include_type(column->constraints, CONSTR_DEFAULT))
    rb_ary_push(constraints, build_default_constraint(column->raw_default));

  rb_hash_aset(hash, symbol_key("constraints"), constraints);

  return hash;
}

static VALUE
build_columns(List *table_elts)
{
  VALUE columns = rb_ary_new();
  ListCell *cell;

  foreach(cell, table_elts)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, ColumnDef))
      rb_ary_push(columns, build_column(castNode(ColumnDef, node)));
  }

  return columns;
}

static VALUE
find_primary_key(VALUE constraints)
{
  long i;
  for (i = 0; i < RARRAY_LEN(constraints); i++)
  {
    VALUE constraint = rb_ary_entry(constraints, i);
    VALUE type = rb_hash_aref(constraint, symbol_key("type"));
    if (type == symbol_value("primary_key"))
      return constraint;
  }

  return Qnil;
}

static void
set_constraint_columns_for_column(VALUE constraint, const char *column_name)
{
  VALUE type = rb_hash_aref(constraint, symbol_key("type"));

  if (type == symbol_value("primary_key") || type == symbol_value("unique") ||
      type == symbol_value("foreign_key"))
  {
    VALUE columns = rb_hash_aref(constraint, symbol_key("columns"));
    if (RARRAY_LEN(columns) == 0 && column_name != NULL)
      rb_ary_push(columns, rb_str_new_cstr(column_name));
  }
}

static VALUE
build_split(YbOptSplit *split)
{
  if (split == NULL)
    return Qnil;

  VALUE hash = rb_hash_new();
  if (split->split_type == NUM_TABLETS)
  {
    rb_hash_aset(hash, symbol_key("type"), symbol_value("num_tablets"));
    rb_hash_aset(hash, symbol_key("num_tablets"),
                 INT2NUM(split->num_tablets));
  }
  else if (split->split_type == SPLIT_POINTS)
  {
    VALUE points = rb_ary_new_capa(list_length(split->split_points));
    ListCell *point_cell;

    foreach(point_cell, split->split_points)
    {
      List *exprs = (List *) lfirst(point_cell);
      VALUE point = rb_ary_new_capa(list_length(exprs));
      ListCell *expr_cell;

      foreach(expr_cell, exprs)
      {
        VALUE expr = expression_string((Node *) lfirst(expr_cell));
        rb_ary_push(point, NIL_P(expr) ? Qnil : expr);
      }

      rb_ary_push(points, point);
    }

    rb_hash_aset(hash, symbol_key("type"), symbol_value("split_points"));
    rb_hash_aset(hash, symbol_key("points"), points);
  }

  return hash;
}

static VALUE
build_partition(PartitionSpec *partition)
{
  if (partition == NULL)
    return Qnil;

  VALUE hash = rb_hash_new();
  VALUE keys = rb_ary_new_capa(list_length(partition->partParams));
  ListCell *cell;

  hash_set_cstr(hash, "strategy", partition->strategy);

  foreach(cell, partition->partParams)
  {
    PartitionElem *elem = castNode(PartitionElem, lfirst(cell));
    if (elem->name != NULL)
      rb_ary_push(keys, rb_str_new_cstr(elem->name));
    else
    {
      VALUE expr = expression_string(elem->expr);
      if (!NIL_P(expr))
        rb_ary_push(keys, expr);
    }
  }

  rb_hash_aset(hash, symbol_key("keys"), keys);
  return hash;
}

static void
append_expression_list(VALUE out, List *exprs)
{
  ListCell *cell;
  int index = 0;

  foreach(cell, exprs)
  {
    VALUE expr = expression_string((Node *) lfirst(cell));

    if (index > 0)
      rb_str_cat_cstr(out, ", ");
    rb_str_append(out, NIL_P(expr) ? rb_str_new_cstr("?") : expr);
    index++;
  }
}

static VALUE
partition_range_datum_string(Node *node)
{
  if (node == NULL)
    return Qnil;

  if (IsA(node, PartitionRangeDatum))
  {
    PartitionRangeDatum *datum = castNode(PartitionRangeDatum, node);

    switch (datum->kind)
    {
      case PARTITION_RANGE_DATUM_MINVALUE:
        return rb_str_new_cstr("MINVALUE");
      case PARTITION_RANGE_DATUM_MAXVALUE:
        return rb_str_new_cstr("MAXVALUE");
      case PARTITION_RANGE_DATUM_VALUE:
        return expression_string(datum->value);
      default:
        return Qnil;
    }
  }

  return expression_string(node);
}

static void
append_partition_range_datums(VALUE out, List *datums)
{
  ListCell *cell;
  int index = 0;

  foreach(cell, datums)
  {
    VALUE datum = partition_range_datum_string((Node *) lfirst(cell));

    if (index > 0)
      rb_str_cat_cstr(out, ", ");
    rb_str_append(out, NIL_P(datum) ? rb_str_new_cstr("?") : datum);
    index++;
  }
}

static VALUE
build_partition_bound_sql(PartitionBoundSpec *bound)
{
  if (bound == NULL)
    return Qnil;

  if (bound->is_default)
    return rb_str_new_cstr("DEFAULT");

  switch (bound->strategy)
  {
    case PARTITION_STRATEGY_HASH:
      return rb_sprintf("FOR VALUES WITH (modulus %d, remainder %d)",
                        bound->modulus, bound->remainder);
    case PARTITION_STRATEGY_LIST:
    {
      VALUE out = rb_str_new_cstr("FOR VALUES IN (");
      append_expression_list(out, bound->listdatums);
      rb_str_cat_cstr(out, ")");
      return out;
    }
    case PARTITION_STRATEGY_RANGE:
    {
      VALUE out = rb_str_new_cstr("FOR VALUES FROM (");
      append_partition_range_datums(out, bound->lowerdatums);
      rb_str_cat_cstr(out, ") TO (");
      append_partition_range_datums(out, bound->upperdatums);
      rb_str_cat_cstr(out, ")");
      return out;
    }
    default:
      return Qnil;
  }
}

static VALUE
build_alter_command(AlterTableCmd *cmd)
{
  VALUE hash = rb_hash_new();

  rb_hash_aset(hash, symbol_key("kind"),
               symbol_value(alter_command_kind(cmd->subtype)));
  hash_set_bool(hash, "missing_ok", cmd->missing_ok);

  switch (cmd->subtype)
  {
    case AT_AddColumn:
    case AT_AddColumnRecurse:
      if (cmd->def != NULL && IsA(cmd->def, ColumnDef))
        rb_hash_aset(hash, symbol_key("column"),
                     build_column(castNode(ColumnDef, cmd->def)));
      break;
    case AT_AddConstraint:
    case AT_AddConstraintRecurse:
      if (cmd->def != NULL && IsA(cmd->def, Constraint))
        rb_hash_aset(hash, symbol_key("constraint"),
                     build_constraint(castNode(Constraint, cmd->def)));
      break;
    case AT_DropColumn:
    case AT_DropColumnRecurse:
    case AT_DropConstraint:
    case AT_DropConstraintRecurse:
      hash_set_cstr(hash, "definition", cmd->name);
      break;
    case AT_AlterColumnType:
      if (cmd->def != NULL && IsA(cmd->def, ColumnDef))
        rb_hash_aset(hash, symbol_key("column"),
                     build_column(castNode(ColumnDef, cmd->def)));
      else
        hash_set_cstr(hash, "definition", cmd->name);
      break;
    case AT_SetTableSpace:
      hash_set_cstr(hash, "tablespace", cmd->name);
      break;
    default:
      hash_set_cstr(hash, "definition", cmd->name);
      break;
  }

  return hash;
}

static VALUE
build_alter_commands(List *cmds)
{
  VALUE commands = rb_ary_new_capa(list_length(cmds));
  ListCell *cell;

  foreach(cell, cmds)
  {
    Node *node = (Node *) lfirst(cell);
    if (node != NULL && IsA(node, AlterTableCmd))
      rb_ary_push(commands, build_alter_command(castNode(AlterTableCmd, node)));
  }

  return commands;
}

static void
extract_create_stmt(VALUE hash, CreateStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("relation"), build_relation_hash(stmt->relation));
  hash_set_bool(hash, "if_not_exists", stmt->if_not_exists);
  hash_set_cstr(hash, "tablespace", stmt->tablespacename);
  hash_set_cstr(hash, "access_method", stmt->accessMethod);

  VALUE constraints = build_table_constraints(stmt->tableElts,
                                              stmt->constraints);
  rb_hash_aset(hash, symbol_key("columns"), build_columns(stmt->tableElts));
  rb_hash_aset(hash, symbol_key("constraints"), constraints);
  rb_hash_aset(hash, symbol_key("primary_key"), find_primary_key(constraints));
  rb_hash_aset(hash, symbol_key("split"), build_split(stmt->split_options));
  rb_hash_aset(hash, symbol_key("partition"), build_partition(stmt->partspec));

  if (stmt->partbound != NULL)
  {
    rb_hash_aset(hash, symbol_key("partition_of"),
                 first_relation_hash_from_range_vars(stmt->inhRelations));
    rb_hash_aset(hash, symbol_key("partition_bound_sql"),
                 build_partition_bound_sql(stmt->partbound));
  }
}

static void
extract_index_stmt(VALUE hash, IndexStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("relation"), build_relation_hash(stmt->relation));
  hash_set_cstr(hash, "index_name", stmt->idxname);
  hash_set_bool(hash, "unique", stmt->unique);
  hash_set_bool(hash, "if_not_exists", stmt->if_not_exists);
  hash_set_cstr(hash, "access_method", stmt->accessMethod);
  hash_set_cstr(hash, "tablespace", stmt->tableSpace);
  rb_hash_aset(hash, symbol_key("concurrently"),
               symbol_value(concurrency_name(stmt->concurrent)));
  rb_hash_aset(hash, symbol_key("keys"), build_key_columns(stmt->indexParams));
  rb_hash_aset(hash, symbol_key("include_columns"),
               build_include_columns(stmt->indexIncludingParams));
  rb_hash_aset(hash, symbol_key("where_sql"),
               expression_string(stmt->whereClause));
  rb_hash_aset(hash, symbol_key("split"), build_split(stmt->split_options));
}

static void
extract_alter_table_stmt(VALUE hash, AlterTableStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("relation"), build_relation_hash(stmt->relation));
  rb_hash_aset(hash, symbol_key("object_type"),
               symbol_value(object_type_name(stmt->objtype)));
  hash_set_bool(hash, "if_exists", stmt->missing_ok);
  rb_hash_aset(hash, symbol_key("commands"), build_alter_commands(stmt->cmds));
}

static void
extract_drop_stmt(VALUE hash, DropStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("object_type"),
               symbol_value(object_type_name(stmt->removeType)));
  rb_hash_aset(hash, symbol_key("objects"),
               build_relation_array_from_name_lists(stmt->objects));
  hash_set_bool(hash, "if_exists", stmt->missing_ok);
  rb_hash_aset(hash, symbol_key("concurrently"),
               symbol_value(stmt->concurrent ? "explicit" : "disabled"));
}

static void
extract_create_schema_stmt(VALUE hash, CreateSchemaStmt *stmt)
{
  hash_set_cstr(hash, "name", stmt->schemaname);
  rb_hash_aset(hash, symbol_key("owner"), role_name(stmt->authrole));
  hash_set_bool(hash, "if_not_exists", stmt->if_not_exists);
}

static void
extract_create_tablespace_stmt(VALUE hash, CreateTableSpaceStmt *stmt)
{
  hash_set_cstr(hash, "name", stmt->tablespacename);
  rb_hash_aset(hash, symbol_key("owner"), role_name(stmt->owner));
  hash_set_cstr(hash, "tablespace_location", stmt->location);

  VALUE replica_placement = find_option_value(stmt->options, "replica_placement");
  if (!NIL_P(replica_placement))
    rb_hash_aset(hash, symbol_key("replica_placement_json"), replica_placement);
}

static void
extract_truncate_stmt(VALUE hash, TruncateStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("object_type"), symbol_value("table"));
  rb_hash_aset(hash, symbol_key("objects"),
               build_relation_array_from_range_vars(stmt->relations));
  rb_hash_aset(hash, symbol_key("relation"),
               first_relation_hash_from_range_vars(stmt->relations));
}

static void
extract_comment_stmt(VALUE hash, CommentStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("object_type"),
               symbol_value(object_type_name(stmt->objtype)));

  if (stmt->object != NULL && IsA(stmt->object, List))
  {
    List *names = (List *) stmt->object;
    VALUE relation = build_relation_hash_from_name_list(names);
    if (!NIL_P(relation))
    {
      rb_hash_aset(hash, symbol_key("relation"), relation);

      VALUE objects = rb_ary_new_capa(1);
      rb_ary_push(objects, relation);
      rb_hash_aset(hash, symbol_key("objects"), objects);
    }

    rb_hash_aset(hash, symbol_key("name"), name_list_string(names));
  }
}

static void
extract_rename_stmt(VALUE hash, RenameStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("object_type"),
               symbol_value(object_type_name(stmt->renameType)));
  rb_hash_aset(hash, symbol_key("relation"), build_relation_hash(stmt->relation));
  hash_set_cstr(hash, "name", stmt->subname);
  hash_set_cstr(hash, "new_name", stmt->newname);
  hash_set_bool(hash, "if_exists", stmt->missing_ok);
}

static void
extract_view_stmt(VALUE hash, ViewStmt *stmt)
{
  rb_hash_aset(hash, symbol_key("relation"), build_relation_hash(stmt->view));
  hash_set_bool(hash, "if_not_exists", false);
}

static VALUE
build_statement_hash(VALUE sql, RawStmt *raw_stmt)
{
  VALUE stmt = rb_hash_new();
  Node *node = raw_stmt->stmt;

  rb_hash_aset(stmt, symbol_key("kind"),
               symbol_value(statement_kind(node)));
  hash_set_cstr(stmt, "raw_node_type", raw_node_type_name(node));
  rb_hash_aset(stmt, symbol_key("location"), INT2NUM(raw_stmt->stmt_location));
  rb_hash_aset(stmt, symbol_key("sql"), statement_sql(sql, raw_stmt));

  if (node == NULL)
    return stmt;

  switch (nodeTag(node))
  {
    case T_CreateStmt:
      extract_create_stmt(stmt, castNode(CreateStmt, node));
      break;
    case T_IndexStmt:
      extract_index_stmt(stmt, castNode(IndexStmt, node));
      break;
    case T_AlterTableStmt:
      extract_alter_table_stmt(stmt, castNode(AlterTableStmt, node));
      break;
    case T_DropStmt:
      extract_drop_stmt(stmt, castNode(DropStmt, node));
      break;
    case T_CreateSchemaStmt:
      extract_create_schema_stmt(stmt, castNode(CreateSchemaStmt, node));
      break;
    case T_CreateTableSpaceStmt:
      extract_create_tablespace_stmt(stmt, castNode(CreateTableSpaceStmt, node));
      break;
    case T_TruncateStmt:
      extract_truncate_stmt(stmt, castNode(TruncateStmt, node));
      break;
    case T_CommentStmt:
      extract_comment_stmt(stmt, castNode(CommentStmt, node));
      break;
    case T_RenameStmt:
      extract_rename_stmt(stmt, castNode(RenameStmt, node));
      break;
    case T_ViewStmt:
      extract_view_stmt(stmt, castNode(ViewStmt, node));
      break;
    default:
      break;
  }

  return stmt;
}

static VALUE
native_parse(VALUE self, VALUE sql)
{
  Check_Type(sql, T_STRING);

  VALUE result = Qnil;

  PG_TRY();
  {
    List *raw_statements = raw_parser(StringValueCStr(sql), RAW_PARSE_DEFAULT);
    VALUE statements = rb_ary_new_capa(list_length(raw_statements));

    ListCell *cell;
    foreach(cell, raw_statements)
    {
      RawStmt *raw_stmt = castNode(RawStmt, lfirst(cell));
      rb_ary_push(statements, build_statement_hash(sql, raw_stmt));
    }

    result = rb_hash_new();
    rb_hash_aset(result, symbol_key("statements"), statements);
    rb_hash_aset(result, symbol_key("errors"), rb_ary_new());
  }
  PG_CATCH();
  {
    VALUE error = rb_hash_new();
    rb_hash_aset(error, symbol_key("message"),
                 rb_str_new_cstr(yb_parser_last_error_message()));
    if (yb_parser_last_error_position() > 0)
      rb_hash_aset(error, symbol_key("position"),
                   INT2NUM(yb_parser_last_error_position()));

    VALUE errors = rb_ary_new();
    rb_ary_push(errors, error);

    result = rb_hash_new();
    rb_hash_aset(result, symbol_key("statements"), rb_ary_new());
    rb_hash_aset(result, symbol_key("errors"), errors);
  }
  PG_END_TRY();

  return result;
}

void
Init_yb_ddl_parser_ext(void)
{
  VALUE mYbDDLParser = rb_define_module("YbDDLParser");
  VALUE mNative = rb_define_module_under(mYbDDLParser, "Native");
  rb_define_singleton_method(mNative, "parse", native_parse, 1);
}
