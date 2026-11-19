# GTA Transit Route Optimizer

A high-performance C++17 transit routing engine and Python data/testing pipeline built on real GO Transit GTFS schedule data. Models regional rail corridors as a directed weighted graph, computes shortest-path routes via Dijkstra's algorithm, and supports real-time fault injection (station closures and line delays) with structured telemetry logging.

## Architecture
*   **`scripts/parse_gtfs.py` (Python):** Ingests raw Metrolinx/GO Transit GTFS feeds (`stops.txt`, `trips.txt`, `stop_times.txt`) and builds a deduplicated directed edge list (`stops.csv`, `edges.csv`).
*   **`src/graph.hpp` & `src/router.hpp` (C++17):** Adjacency-list graph loader and priority-queue Dijkstra routing engine supporting dynamic node/edge disruption overlays.
*   **`src/main.cpp` (C++17):** CLI interface with `--close-stop` and `--delay-route` fault-injection flags and millisecond-precision structured telemetry logging (`router.log`).
*   **`tests/run_tests.py` (Python):** Automated verification and validation (V&V) harness that executes normal, disruption, and boundary test cases and validates log integrity.

## Build & Run
```bash
# 1. Preprocess raw GTFS data
python scripts/parse_gtfs.py

# 2. Compile with strict flags
g++ -std=c++17 -Wall -Wextra -Werror -g -o router src/main.cpp

# 3. Run a normal route query (Whitby GO -> Union Station)
./router WH UN

# 4. Inject a station closure or route delay
./router WH UN --close-stop PIN
./router WH UN --delay-route 11260127-LE 600

# 5. Run the automated Python V&V test suite
python tests/run_tests.py