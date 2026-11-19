#include <iostream>
#include <string>
#include "graph.hpp"
#include "router.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./router <START_STOP_ID> <END_STOP_ID>\n";
        std::cerr << "Example: ./router WH UN\n";
        return 1;
    }

    std::string start_id = argv[1];
    std::string end_id = argv[2];

    TransitGraph graph;
    if (!graph.load_stops("data/processed/stops.csv") ||
        !graph.load_edges("data/processed/edges.csv")) {
        std::cerr << "Error: Could not load CSV files from data/processed/\n";
        return 1;
    }

    RouteResult res = find_fastest_route(graph, start_id, end_id);

    if (!res.found) {
        std::cout << "No route found between " << start_id << " and " << end_id << ".\n";
        return 0;
    }

    std::cout << "=== Fastest Route: " << graph.stops[start_id].name
              << " -> " << graph.stops[end_id].name << " ===\n";
    for (const auto& step : res.steps) {
        int mins = step.cumulative_sec / 60;
        std::cout << "  [" << mins << " min] " << step.stop_id
                  << " (" << graph.stops[step.stop_id].name << ")"
                  << " via " << step.route_id << "\n";
    }
    std::cout << "Total travel time: " << (res.total_time_sec / 60) << " min ("
              << res.total_time_sec << " sec)\n";

    return 0;
}