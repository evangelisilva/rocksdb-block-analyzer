#ifndef STATS_REPORTER_H
#define STATS_REPORTER_H

#include <rocksdb/db.h>

// Reports aggregated properties and per-table stats
void ReportDBStats(rocksdb::DB *db);

#endif // STATS_REPORTER_H
