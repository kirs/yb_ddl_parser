# yb_ddl_parser

Small Ruby gem for extracting stable facts from YugabyteDB DDL.

This is intentionally not a drop-in replacement for `pg_query`. It exposes a
small AST suited to migration safety checks and hides PostgreSQL/YugabyteDB
internal parse node churn.

## License

Project code is licensed under the Apache License, Version 2.0.

This gem vendors portions of YugabyteDB's PostgreSQL parser fork. Vendored
YugabyteDB code is Apache-2.0 unless otherwise noted by upstream files, and
PostgreSQL-derived files remain under the PostgreSQL License. See
`THIRD_PARTY_NOTICES.md` and `LICENSES/` for details.
