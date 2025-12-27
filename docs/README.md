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
mkdir build && cd build
cmake ..
cmake --build .
```

## Run

To generate the data:

```bash
./src/app generate 5 # from the /build directory
```

To read the data:

```bash
./src/app read 3 # from the /build directory
```

## Test

To test, run:

```bash
ctest --output-on-failure
```
