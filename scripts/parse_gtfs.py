import csv
import os
from collections import defaultdict

RAW_DIR = "data/raw_gtfs"
OUT_DIR = "data/processed"

def time_to_seconds(time_str):
    """Converts GTFS HH:MM:SS (which can exceed 24:00:00) to total seconds."""
    h, m, s = map(int, time_str.strip().split(":"))
    return h * 3600 + m * 60 + s

def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # 1. Process stops.txt -> stops.csv
    stops_in = os.path.join(RAW_DIR, "stops.txt")
    stops_out = os.path.join(OUT_DIR, "stops.csv")
    stop_count = 0

    with open(stops_in, mode="r", encoding="utf-8-sig") as fin, \
         open(stops_out, mode="w", newline="", encoding="utf-8") as fout:
        reader = csv.DictReader(fin)
        writer = csv.writer(fout)
        writer.writerow(["stop_id", "stop_name", "lat", "lon"])

        for row in reader:
            writer.writerow([
                row["stop_id"].strip(),
                row["stop_name"].strip().replace(",", ""), # avoid breaking CSV parsing in C++
                row["stop_lat"].strip(),
                row["stop_lon"].strip()
            ])
            stop_count += 1

    print(f"Wrote {stop_count} stops to {stops_out}")

    # 2. Map trip_id -> route_id from trips.txt
    trip_to_route = {}
    with open(os.path.join(RAW_DIR, "trips.txt"), mode="r", encoding="utf-8-sig") as f:
        for row in csv.DictReader(f):
            trip_to_route[row["trip_id"].strip()] = row["route_id"].strip()

    # 3. Group stop_times by trip_id
    trips = defaultdict(list)
    with open(os.path.join(RAW_DIR, "stop_times.txt"), mode="r", encoding="utf-8-sig") as f:
        for row in csv.DictReader(f):
            trips[row["trip_id"].strip()].append((
                int(row["stop_sequence"]),
                row["stop_id"].strip(),
                row["arrival_time"].strip(),
                row["departure_time"].strip()
            ))

    # 4. Build deduplicated edges: (from_stop, to_stop, route_id) -> min_travel_time_sec
    edges = {}
    for trip_id, stop_list in trips.items():
        stop_list.sort(key=lambda x: x[0]) # sort by stop_sequence
        route_id = trip_to_route.get(trip_id, "UNKNOWN")

        for i in range(len(stop_list) - 1):
            _, from_stop, _, dep_time = stop_list[i]
            _, to_stop, arr_time, _ = stop_list[i + 1]

            if from_stop == to_stop:
                continue

            travel_sec = time_to_seconds(arr_time) - time_to_seconds(dep_time)
            if travel_sec <= 0:
                continue

            key = (from_stop, to_stop, route_id)
            if key not in edges or travel_sec < edges[key]:
                edges[key] = travel_sec

    # 5. Write edges.csv
    edges_out = os.path.join(OUT_DIR, "edges.csv")
    with open(edges_out, mode="w", newline="", encoding="utf-8") as fout:
        writer = csv.writer(fout)
        writer.writerow(["from_stop_id", "to_stop_id", "travel_time_sec", "route_id"])
        for (from_stop, to_stop, route_id), sec in edges.items():
            writer.writerow([from_stop, to_stop, sec, route_id])

    print(f"Wrote {len(edges)} unique edges to {edges_out}")

if __name__ == "__main__":
    main()