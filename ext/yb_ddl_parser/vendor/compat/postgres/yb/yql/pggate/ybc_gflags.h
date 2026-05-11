#ifndef YB_YQL_PGGATE_YBC_GFLAGS_H
#define YB_YQL_PGGATE_YBC_GFLAGS_H

#include <stdbool.h>

typedef struct YbcGFlags
{
  bool *ysql_enable_profile;
  bool *ysql_disable_index_backfill;
  bool *ysql_enable_reindex;
  bool *enable_object_locking_for_table_locks;
} YbcGFlags;

const YbcGFlags *YBCGetGFlags(void);

#endif

