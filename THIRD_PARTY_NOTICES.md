# Third-Party Notices

This project includes original code for a Ruby native extension and Ruby API.
Unless otherwise noted, that original code is licensed under the Apache License,
Version 2.0.

This project vendors portions of YugabyteDB's PostgreSQL parser fork under:

```text
ext/yb_ddl_parser/vendor/yugabyte-postgres
```

YugabyteDB source code is licensed under the Apache License, Version 2.0,
except where otherwise noted by upstream files. The vendored parser fork also
contains PostgreSQL-derived source code under the PostgreSQL License. Source
file copyright headers are retained in place.

PostgreSQL-derived files include notices such as:

```text
Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
Portions Copyright (c) 1994, Regents of the University of California
```

Compatibility headers under:

```text
ext/yb_ddl_parser/vendor/compat/postgres
```

were copied from the locally installed `pg_query` gem / PostgreSQL
compatibility packaging and remain subject to their original notices and
license terms.

License text and third-party license text are available in:

```text
LICENSE
LICENSES/PostgreSQL.txt
```

The Apache-2.0 license identifier is also recorded at:

```text
LICENSES/Apache-2.0.txt
```

