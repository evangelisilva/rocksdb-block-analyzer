#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include <iostream>
#include <string>

using namespace rocksdb;

int main() {
  DB *db;
  Options options;
  options.create_if_missing = true;
  options.write_buffer_size =
      1024 * 1024; // Flush often might create small SSTs or just trigger flush
                   // manually.

  Status s = DB::Open(options, "/tmp/test_rocksdb_blocks", &db);
  assert(s.ok());

  for (int i = 0; i < 10000; ++i) {
    std::string key = "key" + std::to_string(i);
    std::string value(1000, 'x'); // 1KB value
    s = db->Put(WriteOptions(), key, value);
    if (!s.ok())
      std::cerr << s.ToString() << std::endl;
  }

  FlushOptions flush_options;
  flush_options.wait = true;
  db->Flush(flush_options);

  delete db;
  return 0;
}
