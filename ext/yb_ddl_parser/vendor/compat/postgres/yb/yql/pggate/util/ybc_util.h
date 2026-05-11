#ifndef YB_YQL_PGGATE_UTIL_YBC_UTIL_H
#define YB_YQL_PGGATE_UTIL_YBC_UTIL_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "yb/yql/pggate/ybc_gflags.h"

#ifndef YBC_STATUS_TYPE_DEFINED
#define YBC_STATUS_TYPE_DEFINED
typedef struct YbcStatusStruct *YbcStatus;
#endif

bool YBCStatusIsFatalError(YbcStatus status);
uint32_t YBCStatusPgsqlError(YbcStatus status);
void YBCFreeStatus(YbcStatus status);

const char *YBCStatusFilename(YbcStatus status);
int YBCStatusLineNumber(YbcStatus status);
const char *YBCStatusFuncname(YbcStatus status);
size_t YBCStatusMessageLen(YbcStatus status);
const char *YBCStatusMessageBegin(YbcStatus status);
const char *YBCMessageAsCString(YbcStatus status);
const char **YBCStatusArguments(YbcStatus status, size_t *nargs);

void YBCLogImpl(int severity,
                const char *file_name,
                int line_number,
                bool stack_trace,
                const char *format,
                ...);
void YBCLogVA(int severity,
              const char *file_name,
              int line_number,
              bool stack_trace,
              const char *format,
              va_list args);

#define YBC_LOG_INFO(...) YBCLogImpl(0, __FILE__, __LINE__, false, __VA_ARGS__)
#define YBC_LOG_WARNING(...) YBCLogImpl(1, __FILE__, __LINE__, false, __VA_ARGS__)
#define YBC_LOG_ERROR(...) YBCLogImpl(2, __FILE__, __LINE__, false, __VA_ARGS__)
#define YBC_LOG_FATAL(...) YBCLogImpl(3, __FILE__, __LINE__, false, __VA_ARGS__)
#define YBC_LOG_INFO_STACK_TRACE(...) YBCLogImpl(0, __FILE__, __LINE__, true, __VA_ARGS__)
#define YBC_LOG_WARNING_STACK_TRACE(...) YBCLogImpl(1, __FILE__, __LINE__, true, __VA_ARGS__)
#define YBC_LOG_ERROR_STACK_TRACE(...) YBCLogImpl(2, __FILE__, __LINE__, true, __VA_ARGS__)

const char *YBCGetStackTrace(void);
bool YBCIsInitDbModeEnvVarSet(void);
bool YBIsMajorUpgradeInitDb(void);
bool YBCIsObjectLockingEnabled(void);

#endif
