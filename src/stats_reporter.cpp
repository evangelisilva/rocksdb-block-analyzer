#include "stats_reporter.h"
#include <iostream>
#include <map>
#include <rocksdb/table.h>
#include <rocksdb/table_properties.h>
#include <string>

// Helper to format bytes to MB
static std::string print_mb(uint64_t bytes) {
  return std::to_string(bytes) + " bytes (" +
         std::to_string(bytes / 1024.0 / 1024.0) + " MB)";
}

void ReportDBStats(rocksdb::DB *db) {
  std::cout << "\n--- Aggregated Properties (Across all SSTs) ---" << std::endl;
  std::map<std::string, std::string> props;
  db->GetMapProperty(rocksdb::DB::Properties::kAggregatedTableProperties,
                     &props);

  std::cout << "Total Data Size: " << print_mb(std::stoull(props["data_size"]))
            << std::endl;
  std::cout << "Total Index Size: "
            << print_mb(std::stoull(props["index_size"])) << std::endl;
  std::cout << "Total Filter Size: "
            << print_mb(std::stoull(props["filter_size"])) << std::endl;

  // Per Table Details (First few only if many)
  std::cout << "\n--- Per-Table Properties (Showing first 5) ---" << std::endl;
  rocksdb::TablePropertiesCollection tables_props;
  db->GetPropertiesOfAllTables(&tables_props);

  uint64_t total_data_blocks = 0;
  uint64_t total_index_blocks = 0;
  uint64_t total_filter_blocks = 0;

  int count = 0;
  for (const auto &table : tables_props) {
    // Aggregate counts
    total_data_blocks += table.second->num_data_blocks;
    if (table.second->index_size > 0)
      total_index_blocks++;
    if (table.second->filter_size > 0)
      total_filter_blocks++;

    if (count++ < 5) {
      std::cout << "SST File: " << table.first << std::endl;
      std::cout << "  Prop: Data Size:   " << print_mb(table.second->data_size)
                << std::endl;
      std::cout << "  Prop: Index Size:  " << print_mb(table.second->index_size)
                << std::endl;
      std::cout << "  Prop: Filter Size: "
                << print_mb(table.second->filter_size) << std::endl;
      std::cout << "  Prop: Num Data Blocks: " << table.second->num_data_blocks
                << std::endl;
      if (table.second->num_data_blocks > 0) {
        std::cout << "  Calc: Avg Data Block Size: "
                  << table.second->data_size / table.second->num_data_blocks
                  << " bytes" << std::endl;
      }
      std::cout << "  Prop: Num Entries: " << table.second->num_entries
                << std::endl;
      std::cout << "------------------------------------------------"
                << std::endl;
    }
  }

  std::cout << "\n--- Aggregated Block Counts ---" << std::endl;
  std::cout << "Total SST Files: " << tables_props.size() << std::endl;
  std::cout << "Total Data Blocks: " << total_data_blocks << std::endl;
  std::cout << "Total Index Blocks: " << total_index_blocks << " (1 per file)"
            << std::endl;
  std::cout << "Total Filter Blocks: " << total_filter_blocks << " (1 per file)"
            << std::endl;
}
