# Fast C++ Market Data Feed Handler & Exchange Simulator (ITCH 5.0)

A high-performance, zero-allocation feed handler and exchange simulator written in C++23. It replays historical NASDAQ ITCH 5.0 data over MoldUDP64 to simulate a live exchange, processing millions of messages a second to maintain a localized Limit Order Book.

## System Flow

The system is split into two halves: the Exchange (which streams the data) and the Consumer (which parses it and builds the order book).

```text
            EXCHANGE (Publisher) 
            
            [ ITCH File Replay ]
                     │
                     ▼
            [ MoldUDP64 Packer ]
                     │
                     ▼
            [ UDP Socket Client ]
                     │
                (UDP Network)
                     │
            CONSUMER (Receiver) 
                     ▼
            [ UDP Socket Server ]
                     │
                     ▼
          [ MoldUDP64 Unpacker ]
                     │
                     ▼
             [ ITCH 5.0 Parser ]
                     │
                     ▼
            [ Limit Order Book ]

```

## Directory Structure
```text
├── apps/             # Executable entry points (consumer, exchange, benchmark)
├── docs/             # Protocol specifications (NASDAQ ITCH 5.0, MoldUDP64)
├── include/          # Public headers (.h)
│   ├── common/       # Global type definitions and configurations
│   ├── exchange/     # Exchange simulator logic
│   ├── market/       # ITCH 5.0 parser, Limit Order Book, and Market state
│   ├── moldudp64/    # MoldUDP64 packet builders and parsers
│   └── network/      # UDP Client/Server socket wrappers
└── src/              # Implementation files (.cpp) matching the include structure
```
---

## Getting Started

> **Note:** This project is built under the assumption that the host machine is **Little-Endian**.

### Dependencies

Requires a C++23 compiler (GCC 13+) and the Boost library (`boost::container::flat_map`).

```bash
sudo apt install libboost-all-dev

```

### Build

```bash
make all        # Builds everything
make exchange   # Builds exchange simulator
make consumer   # Builds market data consumer
make benchmark  # Builds benchmark engine

```

### Run

To test the live UDP pipeline, open two terminals:

```bash
# Terminal 1: Start the consumer 
./build/market_data_consumer 18000 

# Terminal 2: Start the exchange
./build/exchange_simulator real_data/real_data_100_mb.bin 127.0.0.1 18000

```

To benchmark the parser + order book:

```bash
./build/benchmark_engine real_data/real_data_100_mb.bin

```

---

## Technical Optimizations

* **Dispatch Table:** Replaced `switch` statements with a compile-time function pointer array for branchless routing.
* **Struct Packing:** Mapped raw network bytes directly to C++ structs using `#pragma pack(push, 1)`.
* **Hardware Endianness:** Used C++20 `std::byteswap` to convert network byte order using single CPU instructions.
* **Data Locality:** Replaced node-based `std::map` with `boost::container::flat_map` to maximize L1/L2 cache hits.
* **Tokenized Lookups:** Swapped string hashing for flat integer arrays for instant Order ID and Symbol resolution.
* **Branch Hinting:** Guided the CPU branch predictor with `[[likely]]` and `[[unlikely]]` to keep the hot path dense.
* **Cold Logging:** Pushed I/O error handling into `__attribute__((noinline, cold))` functions to preserve instruction caches.
* **Cache Line Alignment:** Applied `alignas(64)` to data structures to align with CPU cache lines.
* **Zero Allocations:** Eliminated heap allocations in the hot path by using fixed `std::array` and `std::string_view`.

---

## Performance Results

All benchmarks were executed in a standard **GitHub Codespace (4-Core CPU, 16GB RAM)** running Linux.

### 1. Pure Engine Benchmark (In-Memory Offline)

Measures the absolute maximum speed of the isolated ITCH parser and OrderBook.

* **100 MB Dataset:**
     * **Messages Processed:** 3,406,528
     * **Throughput:** **24.87 Million msg/sec**
     * **Latency:** **40.19 ns / msg**


* **500 MB Dataset:**
     * **Messages Processed:** 15,336,144
     * **Throughput:** **8.02 Million msg/sec**
     * **Latency:** **124.66 ns / msg**



### 2. Live Simulation

* **Dataset:** 100 MB ITCH file
* **Throughput:** **15.24 Million msg/sec**
* **Description:** Measures throughput when the exchange simulator feeds the complete protocol pipeline (MoldUDP64 unpacking, ITCH parsing, and OrderBook updates) in-process without UDP socket overhead.

---

## References & Resources

* [NASDAQ ITCH 5.0 Specification](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf)
* [MoldUDP64 Protocol Specification](https://nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/moldudp64.pdf)
* [Nasdaq ITCH 5.0 Dataset](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/)
* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch)
* [CppCon: Carl Cook - "When a Microsecond Is an Eternity: High Performance Trading Systems in C++"](https://www.youtube.com/watch?v=NH1Tta7purM)
* [CppCon 2024: David Gross - "When Nanoseconds Matter: Ultrafast Trading Systems in C++"](https://youtu.be/sX2nF1fW7kI?si=T4qdBDuWy4nHkEuG)
