#ifndef YB_YQL_PGGATE_YBC_PG_TYPEDEFS_H
#define YB_YQL_PGGATE_YBC_PG_TYPEDEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define YB_DEFINE_HANDLE_TYPE(name) typedef struct name *Ybc##name;

YB_DEFINE_HANDLE_TYPE(PgStatement)
YB_DEFINE_HANDLE_TYPE(PgExpr)
YB_DEFINE_HANDLE_TYPE(PgFunction)
YB_DEFINE_HANDLE_TYPE(PgTableDesc)
YB_DEFINE_HANDLE_TYPE(OtelSpanContext)
YB_DEFINE_HANDLE_TYPE(OtelScope)
YB_DEFINE_HANDLE_TYPE(PgChangeRecordBatch)
YB_DEFINE_HANDLE_TYPE(PgRowMessage)

typedef unsigned int YbcPgOid;
typedef uint64_t YbcPgXLogRecPtr;
typedef uint64_t YbcReadPointHandle;
static const YbcPgOid kYBCPgSequencesDataDatabaseOid = 65535;
#ifndef YBC_STATUS_TYPE_DEFINED
#define YBC_STATUS_TYPE_DEFINED
typedef struct YbcStatusStruct *YbcStatus;
#endif

typedef enum YbcPgDataType
{
  YB_YQL_DATA_TYPE_NOT_SUPPORTED = -1,
  YB_YQL_DATA_TYPE_UNKNOWN_DATA = 999
} YbcPgDataType;

typedef enum YbcPgDatumKind
{
  YB_YQL_DATUM_STANDARD_VALUE = 0,
  YB_YQL_DATUM_LIMIT_MAX,
  YB_YQL_DATUM_LIMIT_MIN
} YbcPgDatumKind;

typedef enum YbcPgMetricsCaptureType
{
  YB_METRICS_CAPTURE_NONE = 0
} YbcPgMetricsCaptureType;

typedef enum YbcPgTransactionSetting
{
  YB_TRANSACTIONAL = 0
} YbcPgTransactionSetting;

typedef struct YbcPgTypeAttrs
{
  int32_t typmod;
} YbcPgTypeAttrs;

typedef struct YbcPgTypeEntity
{
  int type_oid;
  YbcPgDataType yb_type;
  bool allow_for_primary_key;
  int64_t datum_fixed_size;
  bool direct_datum;
} YbcPgTypeEntity;

typedef struct YbcPgTypeEntities
{
  const YbcPgTypeEntity *data;
  uint32_t count;
} YbcPgTypeEntities;

typedef struct YbcPgColumnInfo
{
  int attr_num;
  bool is_key;
  bool is_hash;
} YbcPgColumnInfo;

typedef struct YbcPgCollationInfo
{
  bool collate_is_valid_non_c;
  const char *sortkey;
} YbcPgCollationInfo;

typedef struct YbcPgAttrValueDescriptor
{
  int attr_num;
  uint64_t datum;
  bool is_null;
  const YbcPgTypeEntity *type_entity;
  YbcPgCollationInfo collation_info;
  int collation_id;
} YbcPgAttrValueDescriptor;

typedef struct YbcPgExecOutParamValue YbcPgExecOutParamValue;
typedef struct YbcPgExecOutParam YbcPgExecOutParam;
typedef struct YbcPgExecParameters
{
  uint64_t limit_count;
  uint64_t limit_offset;
  bool limit_use_default;
  int rowmark;
  int pg_wait_policy;
  int docdb_wait_policy;
  char *bfinstr;
  uint64_t backfill_read_time;
  uint64_t *stmt_in_txn_limit_ht_for_reads;
  char *partition_key;
  YbcPgExecOutParam *out_param;
  bool is_index_backfill;
  int work_mem;
  int yb_fetch_row_limit;
  int yb_fetch_size_limit;
  int yb_index_check;
} YbcPgExecParameters;
typedef struct YbcPgExecReadWriteStats
{
  uint64_t reads;
  uint64_t read_ops;
  uint64_t writes;
  uint64_t read_wait;
  uint64_t rows_scanned;
  uint64_t rows_received;
} YbcPgExecReadWriteStats;

typedef struct YbcPgExecEventMetric
{
  int64_t sum;
  int64_t count;
} YbcPgExecEventMetric;

typedef struct YbcPgExecStorageMetrics
{
  uint64_t version;
  uint64_t gauges[1];
  int64_t counters[1];
  YbcPgExecEventMetric events[1];
} YbcPgExecStorageMetrics;
typedef struct YbcPgPrepareParameters YbcPgPrepareParameters;
typedef struct YbcPgInitPostgresInfo YbcPgInitPostgresInfo;
typedef struct YbcPgLastKnownCatalogVersionInfo YbcPgLastKnownCatalogVersionInfo;
typedef struct YbcPgSharedDataPlaceholder
{
  char data[32];
} YbcPgSharedDataPlaceholder;
typedef struct YbcPgTableLocalityInfo YbcPgTableLocalityInfo;
typedef struct YbcPgYBTupleIdDescriptor YbcPgYBTupleIdDescriptor;
typedef struct YbcWaitEventInfo YbcWaitEventInfo;
typedef struct YbcWaitEventInfoPtr YbcWaitEventInfoPtr;
typedef struct YbcCatalogMessageLists YbcCatalogMessageLists;
typedef struct YbcCloneInfo YbcCloneInfo;
typedef struct YbcFlushDebugContext YbcFlushDebugContext;
typedef struct YbcReplicationSlotDescriptor YbcReplicationSlotDescriptor;
typedef struct YbcReplicationSlotHashRange YbcReplicationSlotHashRange;
typedef struct YbcSlotEntryDescriptor YbcSlotEntryDescriptor;
typedef struct YbcTablePropertiesData *YbcTableProperties;
typedef struct YbcTypeEntityProvider YbcTypeEntityProvider;
typedef struct YbcSliceSet
{
  void *data;
} YbcSliceSet;
typedef struct YbcConstSliceVector
{
  void *data;
} YbcConstSliceVector;
typedef struct YbcAshQueryPlanPair
{
  uint64_t query_id;
  uint64_t plan_id;
} YbcAshQueryPlanPair;
typedef struct YbcAshMetadata
{
  unsigned char root_request_id[16];
  YbcAshQueryPlanPair qp;
  int32_t pid;
  uint32_t database_id;
  uint32_t user_id;
  unsigned char client_addr[16];
  uint16_t client_port;
  uint8_t addr_family;
  int64_t pss_mem_bytes;
} YbcAshMetadata;
typedef struct Ybctid Ybctid;

#endif
