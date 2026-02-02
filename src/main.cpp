#include "key_generator.h"
#include "stats_reporter.h"
#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/statistics.h>
#include <rocksdb/table.h>
#include <string>

int main(int argc, char **argv) {
  // Defaults
  int64_t num_keys = 1000000;
  int key_size = 512;
  int val_size = 8;
  std::string db_path = "/tmp/rocksdb_block_demo";
  std::string encoding = "hybrid";

  // Simple arg parsing
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [options]\n"
                << "Options:\n"
                << "  --num-keys <N>   Number of keys (default: " << num_keys
                << ")\n"
                << "  --key-size <N>   Key size in bytes (default: " << key_size
                << ")\n"
                << "  --val-size <N>   Value size in bytes (default: "
                << val_size << ")\n"
                << "  --db-path <PATH> Path to DB directory (default: "
                << db_path << ")\n"
                << "  --encoding <TYPE> hybrid|random|sequential (default: "
                << encoding << ")\n";
      return 0;
    } else if (arg == "--num-keys" && i + 1 < argc) {
      num_keys = std::stoll(argv[++i]);
    } else if (arg == "--key-size" && i + 1 < argc) {
      key_size = std::stoi(argv[++i]);
    } else if (arg == "--val-size" && i + 1 < argc) {
      val_size = std::stoi(argv[++i]);
    } else if (arg == "--db-path" && i + 1 < argc) {
      db_path = argv[++i];
    } else if (arg == "--encoding" && i + 1 < argc) {
      encoding = argv[++i];
    } else {
      std::cerr << "Unknown or incomplete argument: " << arg << std::endl;
      return 1;
    }
  }

  std::cout << "Configuration:\n"
            << "  Num Keys:  " << num_keys << "\n"
            << "  Key Size:  " << key_size << "\n"
            << "  Val Size:  " << val_size << "\n"
            << "  Encoding:  " << encoding << "\n"
            << "  DB Path:   " << db_path << "\n\n";

  // Clean up previous run
  std::string cmd = "rm -rf " + db_path;
  system(cmd.c_str());

  rocksdb::DB *db;
  rocksdb::Options options;

  // --- User Requested Options ---
  options.create_if_missing = true;
  options.statistics = rocksdb::CreateDBStatistics();
  options.stats_dump_period_sec = 0;

  // durability / compression
  options.use_fsync = false;
  options.compression = rocksdb::kNoCompression;
  options.disable_auto_compactions = true;

  // prevent OS page cache from helping
  options.use_direct_io_for_flush_and_compaction = true;
  options.use_direct_reads = true;

  // table / cache config
  rocksdb::BlockBasedTableOptions table_options;
  table_options.cache_index_and_filter_blocks = true;
  table_options.pin_l0_filter_and_index_blocks_in_cache = true;
  table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, false));
  table_options.whole_key_filtering = true;

  // Using a cache to support pinning
  table_options.block_cache =
      rocksdb::NewLRUCache(128 * 1024 * 1024); // 128MB cache

  options.table_factory.reset(
      rocksdb::NewBlockBasedTableFactory(table_options));
  // -----------------------------

  // Open DB
  rocksdb::Status s = rocksdb::DB::Open(options, db_path, &db);
  if (!s.ok()) {
    std::cerr << "Open failed: " << s.ToString() << std::endl;
    return 1;
  }

  std::cout << "Generating and inserting " << num_keys << " keys..."
            << std::endl;

  rocksdb::WriteOptions write_options;
  write_options.disableWAL = true; // Speed up load, we only care about SSTs

  for (int i = 0; i < num_keys; ++i) {
    if (i % 100000 == 0)
      std::cout << "Inserted " << i << "..." << std::endl;

    std::string key;
    if (encoding == "random") {
      key = EncodeKeyRandom(i, key_size);
    } else if (encoding == "sequential") {
      key = EncodeKeySequential(i, key_size);
    } else { // hybrid (default)
      key = EncodeKeyHybrid(i, key_size);
    }

    std::string value = GenerateRandomValue(val_size);

    s = db->Put(write_options, key, value);
    if (!s.ok()) {
      std::cerr << "Put failed at " << i << ": " << s.ToString() << std::endl;
      return 1;
    }
  }

  // Flush to ensuring data is on disk in SSTs
  std::cout << "Flushing data to disk..." << std::endl;
  rocksdb::FlushOptions flush_options;
  s = db->Flush(flush_options);
  if (!s.ok()) {
    std::cerr << "Flush failed: " << s.ToString() << std::endl;
    return 1;
  }

  // Report Stats
  ReportDBStats(db);

  // Cleanup
  delete db;
  // rocksdb::DestroyDB(db_path, options);

  return 0;
}
