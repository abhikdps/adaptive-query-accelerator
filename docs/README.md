# Adaptive Query Accelerator

An Adaptive Query Accelerator in C++ that dynamically optimizes data access paths using learned caching and prefetching strategies.

AQA(Adaptive Query Accelerator) sits between the database storage layer and the query engine and focuses on:

- Predicting which data will be needed next
- Fetching it earlier
- Placing it in the optimal cache tier
- Reducing cache misses and disk I/O

## Build

To build the project, run:

```bash
cmake -B build
cmake --build build
```

## Run

To generate the data:

```bash
./build/src/app generate 10000
```

To read the data:

```bash
./build/src/app read 3
```

To run sequential benchmark:

```bash
./build/src/app benchmark seq 10000
```

To run random benchmark:

```bash
./build/src/app benchmark rnd 10000
```

To run the benchmark:

```bash
./build/benchmarks/baseline_benchmark
```

### Expected Result

Sequential is significantly faster than Random access because standard HDDs/SSDs and OS file readahead optimize for sequential patterns while random access incurs more seeking overhead (especially on HDDs) or IOPS limits (on SSDs).

## Test

To test, run:

```bash
ctest --test-dir build/ --output-on-failure
```
