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

To run the executable:

```bash
./src/app # from the /build directory
```

OR

```bash
./buils/src/app # from the root directory
```

## Test

To test, run:

```bash
ctest --output-on-failure
```
