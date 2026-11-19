#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "graph.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct DisruptionConfig {
    std::unordered_set<std::string> closed_stops;
    std::unordered_map<std::string, int> route_delays_sec;
};

struct RouteStep {
    std::string stop_id;
    std::string route_id;
    int cumulative_sec;
};

struct RouteResult {
    bool found = false;
    int total_time_sec = 0;
    std::vector<RouteStep> steps;
};

inline RouteResult find_fastest_route(const TransitGraph& graph,
                                      const std::string& start_id,
                                      const std::string& end_id,
                                      const DisruptionConfig& disruptions = {}) {
    RouteResult result;
    if (graph.stops.find(start_id) == graph.stops.end() ||
        graph.stops.find(end_id) == graph.stops.end()) {
        return result;
    }

    // Fail-safe: if start or end station itself is closed, no valid route exists
    if (disruptions.closed_stops.count(start_id) ||
        disruptions.closed_stops.count(end_id)) {
        return result;
    }

    using Node = std::pair<int, std::string>; // (cost_sec, stop_id)
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::pair<std::string, std::string>> parent;

    dist[start_id] = 0;
    pq.push({0, start_id});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;
        if (u == end_id) break;

        auto it = graph.adj.find(u);
        if (it == graph.adj.end()) continue;

        for (const auto& edge : it->second) {
            // Skip edges leading into a closed station
            if (disruptions.closed_stops.count(edge.to_stop_id)) {
                continue;
            }

            // Apply any active route delay penalty (substring match, e.g., "LE" matches all Lakeshore East routes)
            int penalty = 0;
            for (const auto& [pattern, delay_sec] : disruptions.route_delays_sec) {
                if (edge.route_id.find(pattern) != std::string::npos) {
                    penalty += delay_sec;
                }
            }

            int next_dist = d + edge.travel_time_sec + penalty;
            if (dist.find(edge.to_stop_id) == dist.end() || next_dist < dist[edge.to_stop_id]) {
                dist[edge.to_stop_id] = next_dist;
                parent[edge.to_stop_id] = {u, edge.route_id};
                pq.push({next_dist, edge.to_stop_id});
            }
        }
    }

    if (dist.find(end_id) == dist.end()) {
        return result;
    }

    result.found = true;
    result.total_time_sec = dist[end_id];

    std::string curr = end_id;
    while (curr != start_id) {
        auto [prev, route_id] = parent[curr];
        result.steps.push_back({curr, route_id, dist[curr]});
        curr = prev;
    }
    result.steps.push_back({start_id, "START", 0});
    std::reverse(result.steps.begin(), result.steps.end());

    return result;
}

#endif