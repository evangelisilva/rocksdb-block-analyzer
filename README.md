# RocksDB Block Analyzer

A C++ tool to generate RocksDB data with specific key/value characteristics and analyze the resulting Data, Index, and Filter block sizes.

## Build

Requirements:
- C++17 compiler
- RocksDB library installed (e.g., `brew install rocksdb`)

```bash
make clean && make
```

## Usage

```bash
./rocksdb_block_size_demo [options]
```

### Options

| Option | Default | Description |
|--------|---------|-------------|
| `--num-keys <N>` | `1000000` | Number of keys to insert |
| `--key-size <N>` | `512` | Size of the key in bytes |
| `--val-size <N>` | `8` | Size of the value in bytes |
| `--db-path <PATH>` | `/tmp/rocksdb_block_demo` | Path to database directory (will be cleared on start) |
| `--encoding <TYPE>`| `hybrid` | Key encoding strategy: `hybrid`, `random`, or `sequential` |

### Key Encodings

1.  **`hybrid`** (Default):
    *   **Structure**: 8-byte Order-Preserving Prefix (Sign-flipped Big Endian) + Deterministic High-Entropy Suffix.
    *   **Purpose**: Simulates a schema with ordered IDs but high entropy content, preventing excessive prefix compression while maintaining sort order.
2.  **`random`**:
    *   **Structure**: Purely random bytes.
    *   **Purpose**: Maximum entropy test case.
3.  **`sequential`**:
    *   **Structure**: Zero-padded sequential number string (e.g., "000...001").
    *   **Purpose**: Low entropy, high prefix compression potential (if enabled).

## Experiment Results (20 Million Keys)

We ran a large-scale test to analyze the block size overhead.

**Configuration:**
- **Keys:** 20,000,000
- **Key Size:** 512 bytes (Hybrid Encoding)
- **Value Size:** 8 bytes
- **Compression:** None
- **Index/Filter Pinning:** Enabled

**Results:**

| Metric | Aggregated Total | Avg (Per SST File) |
|--------|------------------|--------------------|
| **Data Size** | ~9.83 GB | ~61.5 MB |
| **Index Size** | ~54.48 MB | ~341 KB |
| **Filter Size** | ~23.85 MB | ~149 KB |
| **SST Files** | 164 | - |

**Block Breakdown:**
- **Data Block Size (Avg):** 3.69 KB
- **Index Block Size:** One block per SST (~341 KB)
- **Filter Block Size:** One block per SST (~149 KB)

*Observation*: With 512-byte keys and ~3.7KB data blocks, the index overhead is approximately **0.55%** of the data size, and the Bloom filter overhead (10 bits/key) is approximately **0.24%**.
