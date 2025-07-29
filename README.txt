# 🚀 MBP-10 Orderbook Reconstruction from MBO Data

**Author:** Aditya Srivastava  

---

## 📋 Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [Special Operations](#special-operations)
- [Data Structures & Algorithms](#data-structures--algorithms)
- [Optimization Strategies](#optimization-strategies)
- [Performance](#performance)
- [Limitations & Improvements](#limitations--improvements)
- [Build & Usage](#build--usage)
- [Input/Output Format](#inputoutput-format)
- [Dependencies](#dependencies)
- [Testing & Verification](#testing--verification)

---

## 📝 Overview
This C++ project reconstructs the MBP-10 (Market by Price, top 10 levels) orderbook from MBO (Market by Order) data. It is designed for high-frequency trading environments, efficiently processing streams of trade actions to maintain an accurate, real-time orderbook.

---

## ✨ Key Features
- **Handles all MBO action types:** Add (A), Cancel (C), Trade (T), Fill (F)
- **Implements T→F→C sequence logic** as required by modern exchanges
- **Maintains separate bid/ask sides** with correct price ordering
- **Outputs MBP-10** (top 10 levels per side) in the required CSV format
- **Highly optimized** for speed and memory efficiency

---

## ⚡ Special Operations
- **Initial Clear Action:** Skips the first row if action is 'C' and side is 'R' (start with empty book)
- **T→F→C Sequence Handling:**
  - Only the Cancel (C) updates the book, but T/F/C are combined into a single trade event
  - T on ASK (not in book) is matched to BID side, and vice versa
  - T with side 'N' is ignored
- **Neutral Side Handling:** Trades with side 'N' do not alter the orderbook

---

## 🏗️ Data Structures & Algorithms
- **OrderBook Class:**
  - `std::map` for price-ordered levels (bids: descending, asks: ascending)
  - `std::unordered_map` for O(1) order lookups by order_id
  - `std::vector` for tracking pending trade sequences
- **Level Management:**
  - Each price level tracks total size and set of order IDs
  - Automatic cleanup when total size reaches zero
- **Trade Sequence Tracking:**
  - `PendingTrade` struct for T→F→C sequences
  - Linear search through pending trades (efficient for small k)

---

## 🏎️ Optimization Strategies
- **Compiler:** `-O3` for max optimization, `-std=c++17`
- **Data Structures:**
  - `std::map` for O(log n) price operations
  - `std::unordered_map` for O(1) order lookups
  - `std::set` for order ID tracking within levels
- **Memory:**
  - Stack allocation for small objects
  - Efficient string parsing with `std::stringstream`
- **I/O:**
  - Single-pass CSV parsing
  - Buffered output writing

---

## 📈 Performance
- **Time Complexity:**
  - Order addition: O(log n)
  - Order lookup: O(1)
  - Trade sequence matching: O(k) (small k)
  - MBP generation: O(10) per side (constant)
- **Space Complexity:**
  - O(n) for orders
  - O(m) for price levels (m ≤ n)
  - O(k) for pending trades
- **Bottlenecks:**
  - Linear search for trade sequences (acceptable for small k)
  - CSV parsing (optimized)

---

## 🚧 Limitations & Improvements
- **Current:**
  - Linear search for trade sequence matching
  - No parallel processing
  - Memory usage scales with number of orders
- **Potential:**
  - Hash-based trade sequence tracking
  - Memory pools for order allocation
  - SIMD optimizations
  - Lock-free data structures for multi-threading
  - Streaming for very large datasets

---

## 🛠️ Build & Usage
1. **Build the executable:**
   ```sh
   make
   ```
2. **Run with input file:**
   ```sh
   ./reconstruction_aditya mbo.csv
   ```
3. **Test with sample data:**
   ```sh
   make test
   ```
4. **Performance testing:**
   ```sh
   make perf
   ```
5. **Clean build artifacts:**
   ```sh
   make clean
   ```

---

## 📥 Input/Output Format
- **Input CSV:**
  - `ts_event,ts_rtt,ts_instrument,side,action,level,order_id,price,size,channel,sequence`
- **Output CSV:**
  - `ts_event,ts_rtt,ts_instrument,side,level,price,size,channel,sequence`

---

## 📦 Dependencies
- C++17 compatible compiler (g++ recommended)
- Standard C++ library only (no external dependencies)

---

## ✅ Testing & Verification
- **Sample MBO input:** `mbo.csv`
- **Expected MBP output:** `mbp.csv`
- **Automated comparison:** `make test`
- **Performance timing:** `make perf`

### Correctness
- Handles all MBO action types
- Implements T→F→C sequence requirements
- Maintains price level ordering (bids descending, asks ascending)
- Ensures orderbook state consistency
- Outputs top 10 levels per side

---
