#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include "graph.hpp"
#include "router.hpp"

void log_event(const std::string& start_id, const std::string& end_id,
               const DisruptionConfig& disruptions, const RouteResult& res,
               double elapsed_ms) {
    std::ofstream log_file("router.log", std::ios::app);
    if (!log_file.is_open()) return;

    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);

    log_file << std::put_time(lt, "%Y-%m-%d %H:%M:%S")
             << " | QUERY=" << start_id << "->" << end_id
             << " | CLOSED_STOPS=" << disruptions.closed_stops.size()
             << " | DELAYED_ROUTES=" << disruptions.route_delays_sec.size()
             << " | STATUS=" << (res.found ? "OK" : "UNREACHABLE")
             << " | TRAVEL_SEC=" << (res.found ? res.total_time_sec : -1)
             << " | EXEC_MS=" << std::fixed << std::setprecision(3) << elapsed_ms
             << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./router <START_ID> <END_ID> [--close-stop <ID>] [--delay-route <ROUTE_ID> <SEC>]\n";
        return 1;
    }

    std::string start_id = argv[1];
    std::string end_id = argv[2];
    DisruptionConfig disruptions;

    // Parse optional disruption flags
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--close-stop" && i + 1 < argc) {
            disruptions.closed_stops.insert(argv[++i]);
        } else if (arg == "--delay-route" && i + 2 < argc) {
            std::string route_id = argv[++i];
            int delay_sec = std::stoi(argv[++i]);
            disruptions.route_delays_sec[route_id] = delay_sec;
        }
    }

    TransitGraph graph;
    if (!graph.load_stops("data/processed/stops.csv") ||
        !graph.load_edges("data/processed/edges.csv")) {
        std::cerr << "Error: Could not load CSV files from data/processed/\n";
        return 1;
    }

    auto t_start = std::chrono::high_resolution_clock::now();
    RouteResult res = find_fastest_route(graph, start_id, end_id, disruptions);
    auto t_end = std::chrono::high_resolution_clock::now();

    double elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    log_event(start_id, end_id, disruptions, res, elapsed_ms);

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
              << res.total_time_sec << " sec) | Computed in "
              << std::fixed << std::setprecision(3) << elapsed_ms << " ms\n";

    return 0;
}