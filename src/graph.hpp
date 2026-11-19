#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct Stop {
    std::string id;
    std::string name;
    double lat;
    double lon;
};

struct Edge {
    std::string to_stop_id;
    int travel_time_sec;
    std::string route_id;
};

class TransitGraph {
public:
    std::unordered_map<std::string, Stop> stops;
    std::unordered_map<std::string, std::vector<Edge>> adj;

    // Helper to strip Windows '\r' line endings if present
    static std::string clean(std::string s) {
        if (!s.empty() && s.back() == '\r') {
            s.pop_back();
        }
        return s;
    }

    bool load_stops(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        std::string line;
        std::getline(file, line); // skip header

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string id, name, lat_s, lon_s;

            std::getline(ss, id, ',');
            std::getline(ss, name, ',');
            std::getline(ss, lat_s, ',');
            std::getline(ss, lon_s, ',');

            id = clean(id);
            stops[id] = {id, clean(name), std::stod(clean(lat_s)), std::stod(clean(lon_s))};
        }
        return true;
    }

    bool load_edges(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        std::string line;
        std::getline(file, line); // skip header

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string from_id, to_id, time_s, route_id;

            std::getline(ss, from_id, ',');
            std::getline(ss, to_id, ',');
            std::getline(ss, time_s, ',');
            std::getline(ss, route_id, ',');

            adj[clean(from_id)].push_back({
                clean(to_id),
                std::stoi(clean(time_s)),
                clean(route_id)
            });
        }
        return true;
    }
};

#endif