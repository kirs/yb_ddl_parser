#include "ruby.h"
#include "postgres.h"
#include "catalog/pg_type_d.h"
#include "commands/defrem.h"
#include "mb/pg_wchar.h"
#include "nodes/nodeFuncs.h"
#include "nodes/parsenodes.h"
#include "nodes/value.h"
#include "utils/lsyscache.h"
#include "yb/yql/pggate/ybc_gflags.h"

#include <stdarg.h>
#include <stdio.h>

/*
 * PostgreSQL's port.h maps libc printf-family calls to pg_* replacements.
 * This gem does not compile PostgreSQL's port/snprintf.c, so provide small
 * wrappers around the platform libc functions for the parser runtime.
 */
#ifdef vsnprintf
#undef vsnprintf
#endif
#ifdef snprintf
#undef snprintf
#endif
#ifdef vsprintf
#undef vsprintf
#endif
#ifdef sprintf
#undef sprintf
#endif
#ifdef vfprintf
#undef vfprintf
#endif
#ifdef fprintf
#undef fprintf
#endif
#ifdef vprintf
#undef vprintf
#endif
#ifdef printf
#undef printf
#endif

bool IsBinaryUpgrade = false;
bool IsYsqlUpgrade = false;
MemoryContext YbCurrentMemoryContext = (MemoryContext) 0x1;
ErrorContextCallback *error_context_stack = NULL;
sigjmp_buf *PG_exception_stack = NULL;
int pg_database_encoding_max_length_value = 1;

static int yb_last_errcode = ERRCODE_SUCCESSFUL_COMPLETION;
static int yb_current_elevel = LOG;
static int yb_error_position = 0;
static char yb_error_message[1024];

static bool yb_true = true;
static bool yb_false = false;

static const YbcGFlags yb_gflags = {
  &yb_true,
  &yb_false,
  &yb_true,
  &yb_true,
};

const YbcGFlags *
YBCGetGFlags(void)
{
  return &yb_gflags;
}

bool
IsYugaByteEnabled(void)
{
  return true;
}

bool
YBIsUsingYBParser(void)
{
  return true;
}

bool
YbIsConnectedToTemplateDb(void)
{
  return false;
}

bool
YBIsCollationEnabled(void)
{
  return true;
}

int
YBUnsupportedFeatureSignalLevel(void)
{
  return ERROR;
}

bool
YBCIsEnvVarTrue(const char *flag)
{
  return false;
}

bool
YBCIsEnvVarTrueWithDefault(const char *flag, bool default_value)
{
  return default_value;
}

MemoryContext
GetThreadLocalCurrentMemoryContext(void)
{
  return YbCurrentMemoryContext;
}

MemoryContext
SetThreadLocalCurrentMemoryContext(MemoryContext memctx)
{
  MemoryContext old = YbCurrentMemoryContext;
  YbCurrentMemoryContext = memctx;
  return old;
}

static void *
yb_alloc(Size size, bool zero)
{
  Size total = size + sizeof(MemoryContext);
  char *base = zero ? calloc(1, total) : malloc(total);
  if (base == NULL)
    rb_memerror();
  *((MemoryContext *) base) = YbCurrentMemoryContext;
  return base + sizeof(MemoryContext);
}

void *
MemoryContextAlloc(MemoryContext context, Size size)
{
  return yb_alloc(size, false);
}

void *
MemoryContextAllocZero(MemoryContext context, Size size)
{
  return yb_alloc(size, true);
}

void *
MemoryContextAllocZeroAligned(MemoryContext context, Size size)
{
  return yb_alloc(size, true);
}

void *
MemoryContextAllocExtended(MemoryContext context, Size size, int flags)
{
  return yb_alloc(size, (flags & MCXT_ALLOC_ZERO) != 0);
}

void *
palloc(Size size)
{
  return MemoryContextAlloc(CurrentMemoryContext, size);
}

void *
palloc0(Size size)
{
  return MemoryContextAllocZero(CurrentMemoryContext, size);
}

void *
palloc_extended(Size size, int flags)
{
  return MemoryContextAllocExtended(CurrentMemoryContext, size, flags);
}

void *
repalloc(void *pointer, Size size)
{
  if (pointer == NULL)
    return palloc(size);

  char *old_base = ((char *) pointer) - sizeof(MemoryContext);
  char *new_base = realloc(old_base, size + sizeof(MemoryContext));
  if (new_base == NULL)
    rb_memerror();
  return new_base + sizeof(MemoryContext);
}

void
pfree(void *pointer)
{
  if (pointer != NULL)
    free(((char *) pointer) - sizeof(MemoryContext));
}

char *
pstrdup(const char *in)
{
  Size len = strlen(in) + 1;
  char *out = palloc(len);
  memcpy(out, in, len);
  return out;
}

int
pg_vsnprintf(char *str, size_t count, const char *fmt, va_list args)
{
  return vsnprintf(str, count, fmt, args);
}

int
pg_snprintf(char *str, size_t count, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int result = vsnprintf(str, count, fmt, args);
  va_end(args);
  return result;
}

int
pg_vsprintf(char *str, const char *fmt, va_list args)
{
  return vsprintf(str, fmt, args);
}

int
pg_sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int result = vsprintf(str, fmt, args);
  va_end(args);
  return result;
}

int
pg_vfprintf(FILE *stream, const char *fmt, va_list args)
{
  return vfprintf(stream, fmt, args);
}

int
pg_fprintf(FILE *stream, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int result = vfprintf(stream, fmt, args);
  va_end(args);
  return result;
}

int
pg_vprintf(const char *fmt, va_list args)
{
  return vprintf(fmt, args);
}

int
pg_printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int result = vprintf(fmt, args);
  va_end(args);
  return result;
}

char *
psprintf(const char *fmt, ...)
{
  va_list args;
  va_list args_copy;
  va_start(args, fmt);
  va_copy(args_copy, args);
  int len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (len < 0)
    rb_raise(rb_eRuntimeError, "vsnprintf failed");
  char *out = palloc((Size) len + 1);
  vsnprintf(out, (Size) len + 1, fmt, args_copy);
  va_end(args_copy);
  return out;
}

void
YBCLogImpl(int severity, const char *file_name, int line_number, bool stack_trace,
           const char *format, ...)
{
}

void
YBCLogVA(int severity, const char *file_name, int line_number, bool stack_trace,
         const char *format, va_list args)
{
}

const char *
YBCGetStackTrace(void)
{
  return "";
}

bool
YBCIsInitDbModeEnvVarSet(void)
{
  return false;
}

bool
YBIsMajorUpgradeInitDb(void)
{
  return false;
}

bool
YBCIsObjectLockingEnabled(void)
{
  return false;
}

bool
message_level_is_interesting(int elevel)
{
  return elevel >= ERROR;
}

bool
errstart(int elevel, const char *domain)
{
  yb_current_elevel = elevel;
  if (elevel >= ERROR)
  {
    yb_error_position = 0;
    yb_error_message[0] = '\0';
  }
  return true;
}

bool
errstart_cold(int elevel, const char *domain)
{
  return errstart(elevel, domain);
}

void
errfinish(const char *filename, int lineno, const char *funcname)
{
  if (yb_current_elevel >= ERROR)
  {
    if (PG_exception_stack != NULL)
      siglongjmp(*PG_exception_stack, 1);

    rb_raise(rb_eRuntimeError, "%s", yb_error_message[0] ? yb_error_message : "parser error");
  }
}

int
errcode(int sqlerrcode)
{
  yb_last_errcode = sqlerrcode;
  return 0;
}

static void
yb_set_error_message(const char *fmt, va_list args)
{
  vsnprintf(yb_error_message, sizeof(yb_error_message), fmt, args);
}

int
errmsg(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  yb_set_error_message(fmt, args);
  va_end(args);
  return 0;
}

int
errmsg_internal(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  yb_set_error_message(fmt, args);
  va_end(args);
  return 0;
}
int errdetail(const char *fmt, ...) { return 0; }
int errhint(const char *fmt, ...) { return 0; }
int
errposition(int cursorpos)
{
  yb_error_position = cursorpos;
  return 0;
}

int
geterrcode(void)
{
  return yb_last_errcode;
}

bool yb_errstart(int elevel) { return errstart(elevel, NULL); }
bool yb_errstart_cold(int elevel) { return errstart(elevel, NULL); }
void yb_errfinish(const char *filename, int lineno, const char *funcname) {}
sigjmp_buf *yb_get_exception_stack(void) { return PG_exception_stack; }
void yb_set_exception_stack(sigjmp_buf *new_sigjmp_buf) { PG_exception_stack = new_sigjmp_buf; }
void yb_reset_error_status(void) {}

const char *
yb_parser_last_error_message(void)
{
  return yb_error_message[0] ? yb_error_message : "parser error";
}

int
yb_parser_last_error_position(void)
{
  return yb_error_position;
}

int
pg_database_encoding_max_length(void)
{
  return pg_database_encoding_max_length_value;
}

int
pg_get_client_encoding(void)
{
  return PG_SQL_ASCII;
}

int
pg_mbstrlen_with_len(const char *mbstr, int len)
{
  return len;
}

int
pg_mbcliplen(const char *mbstr, int len, int limit)
{
  return len < limit ? len : limit;
}

bool
pg_verifymbstr(const char *mbstr, int len, bool noError)
{
  return true;
}

void
pg_unicode_to_server(pg_wchar c, unsigned char *s)
{
  if (c <= 0x7f)
  {
    s[0] = (unsigned char) c;
    s[1] = '\0';
  }
  else
  {
    s[0] = '?';
    s[1] = '\0';
  }
}

int
strtoint(const char *str, char **endptr, int base)
{
  long value = strtol(str, endptr, base);
  return (int) value;
}

#undef qsort
void
pg_qsort(void *base, size_t nel, size_t elsize,
         int (*cmp)(const void *, const void *))
{
  qsort(base, nel, elsize, cmp);
}

void *
copyObjectImpl(const void *obj)
{
  return (void *) obj;
}

bool
equal(const void *a, const void *b)
{
  return a == b;
}

Oid exprType(const Node *expr) { return InvalidOid; }
int32 exprTypmod(const Node *expr) { return -1; }
Oid exprCollation(const Node *expr) { return InvalidOid; }
int exprLocation(const Node *expr) { return -1; }

char *get_rel_name(Oid relid) { return NULL; }
Oid get_rel_type_id(Oid relid) { return InvalidOid; }
bool type_is_rowtype(Oid typid) { return false; }

void
get_typlenbyval(Oid typid, int16 *typlen, bool *typbyval)
{
  *typlen = -1;
  *typbyval = false;
}

int32
defGetInt32(DefElem *def)
{
  if (def != NULL && def->arg != NULL && IsA(def->arg, Integer))
    return intVal(def->arg);
  return 0;
}

char *
NameListToString(List *names)
{
  Size len = 1;
  ListCell *cell;

  foreach(cell, names)
  {
    len += strlen(strVal(lfirst(cell))) + 1;
  }

  char *out = palloc(len);
  out[0] = '\0';

  foreach(cell, names)
  {
    if (cell != list_head(names))
      strcat(out, ".");
    strcat(out, strVal(lfirst(cell)));
  }

  return out;
}

struct varlena *
pg_detoast_datum(struct varlena *datum)
{
  return datum;
}
