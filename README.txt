MBP-10 Orderbook Reconstruction from MBO Data
=============================================

Author: Aditya Srivastava
Date: December 2024

OVERVIEW
--------
This C++ implementation reconstructs MBP-10 (Market by Price - top 10 levels) orderbook 
from MBO (Market by Order) data. The program processes high-frequency trading data 
and generates the orderbook state after each action.

KEY FEATURES
------------
1. Handles all MBO action types: Add (A), Cancel (C), Trade (T), Fill (F)
2. Implements special T->F->C sequence handling as per requirements
3. Maintains separate bid/ask sides with proper price ordering
4. Generates MBP-10 output with top 10 levels for each side
5. High-performance implementation optimized for speed

SPECIAL OPERATIONS IMPLEMENTED
-----------------------------
1. Initial Clear Action: Skips the first row with action 'C' and side 'R'
2. T->F->C Sequence Handling: 
   - Trades (T) are stored on the opposite side of the book
   - Fills (F) and Cancels (C) complete the sequence
   - The trade affects the orderbook only after the complete sequence
3. Neutral Side Handling: Trades with side 'N' don't alter the orderbook

DATA STRUCTURES & ALGORITHMS
---------------------------
1. OrderBook Class:
   - Uses std::map for price-ordered levels (bids: descending, asks: ascending)
   - std::unordered_map for O(1) order lookups by order_id
   - std::vector for tracking pending trade sequences

2. Level Management:
   - Each price level tracks total size and set of order IDs
   - Automatic level cleanup when total size reaches zero
   - Efficient order addition/removal/update operations

3. Trade Sequence Tracking:
   - PendingTrade struct to track T->F->C sequences
   - Linear search through pending trades (optimized for small sequences)
   - Proper side matching for sequence completion

OPTIMIZATION STRATEGIES
----------------------
1. Compiler Optimizations:
   - -O3: Maximum optimization level
   - -march=native -mtune=native: CPU-specific optimizations
   - -std=c++17: Modern C++ features for better performance

2. Data Structure Choices:
   - std::map for price ordering (logarithmic operations)
   - std::unordered_map for order lookups (amortized O(1))
   - std::set for order ID tracking within levels
   - std::vector for output records (pre-allocated)

3. Memory Management:
   - Stack allocation for small objects
   - Efficient string parsing with std::stringstream
   - Minimal memory allocations during processing

4. I/O Optimizations:
   - Single-pass CSV parsing
   - Buffered output writing
   - Minimal string operations

PERFORMANCE CONSIDERATIONS
-------------------------
1. Time Complexity:
   - Order addition: O(log n) for price level insertion
   - Order lookup: O(1) average case
   - Trade sequence matching: O(k) where k is pending trades
   - MBP generation: O(10) per side (constant time)

2. Space Complexity:
   - O(n) for order storage
   - O(m) for price levels where m ≤ n
   - O(k) for pending trades where k is typically small

3. Bottlenecks Identified:
   - Linear search through pending trades (acceptable for small k)
   - CSV parsing overhead (mitigated with efficient parsing)
   - String operations in output generation

LIMITATIONS & IMPROVEMENTS
-------------------------
1. Current Limitations:
   - Linear search for trade sequence matching
   - No parallel processing capabilities
   - Memory usage scales with number of orders

2. Potential Improvements:
   - Use hash-based trade sequence tracking for O(1) lookups
   - Implement memory pools for order allocation
   - Add SIMD optimizations for bulk operations
   - Consider lock-free data structures for multi-threading

3. Scalability Considerations:
   - For very large datasets, consider streaming approach
   - Implement order ID compression for memory efficiency
   - Add configurable output formats

BUILD & USAGE
-------------
1. Build the executable:
   make

2. Run with input file:
   ./reconstruction_aditya mbo.csv

3. Test with sample data:
   make test

4. Performance testing:
   make perf

5. Clean build artifacts:
   make clean

INPUT FORMAT
-----------
CSV with columns: ts_event,ts_rtt,ts_instrument,side,action,level,order_id,price,size,channel,sequence

OUTPUT FORMAT
------------
CSV with columns: ts_event,ts_rtt,ts_instrument,side,level,price,size,channel,sequence

DEPENDENCIES
-----------
- C++17 compatible compiler (g++ recommended)
- Standard C++ library only (no external dependencies)

TESTING
-------
The implementation includes:
1. Sample MBO input data (mbo.csv)
2. Expected MBP output (mbp.csv)
3. Automated comparison in Makefile
4. Performance timing measurements

CORRECTNESS VERIFICATION
-----------------------
The implementation correctly handles:
1. All MBO action types
2. Special T->F->C sequence requirements
3. Price level ordering (bids descending, asks ascending)
4. Orderbook state consistency
5. Top 10 level output generation

For additional verification, the code can be extended with unit tests
using frameworks like Google Test or Catch2. 