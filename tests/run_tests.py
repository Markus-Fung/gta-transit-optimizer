import os
import subprocess
import sys

BINARY = "./router.exe" if os.name == "nt" else "./router"
LOG_FILE = "router.log"

def run_router(args):
    cmd = [BINARY] + args
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode, result.stdout, result.stderr

def parse_last_log():
    with open(LOG_FILE, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]
    last = lines[-1]
    fields = {}
    for part in last.split(" | ")[1:]:
        k, v = part.split("=", 1)
        fields[k] = v
    return fields

def main():
    if not os.path.exists(BINARY):
        print(f"ERROR: {BINARY} not found. Compile src/main.cpp first.")
        sys.exit(1)

    # Clear old log for a clean test run
    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

    tests = [
        {
            "name": "Normal Route (Whitby -> Union)",
            "args": ["WH", "UN"],
            "expect_status": "OK",
            "expect_min_sec": 3000,
        },
        {
            "name": "Fault Injection: Station Closure (Close Pickering)",
            "args": ["WH", "UN", "--close-stop", "PIN"],
            "expect_status": "UNREACHABLE",
            "expect_min_sec": -1,
        },
        {
            "name": "Fault Injection: Route Delay (+600s on Lakeshore East)",
            "args": ["WH", "UN", "--delay-route", "LE", "600"],
            "expect_status": "OK",
            "expect_min_sec": 4000,
        },
        {
            "name": "Edge Case: Invalid Station ID",
            "args": ["INVALID_STOP", "UN"],
            "expect_status": "UNREACHABLE",
            "expect_min_sec": -1,
        },
        {
            "name": "Edge Case: Closed Destination Station",
            "args": ["WH", "UN", "--close-stop", "UN"],
            "expect_status": "UNREACHABLE",
            "expect_min_sec": -1,
        },
    ]

    passed = 0
    exec_times = []

    print("=== Running Automated V&V Test Suite ===\n")
    for t in tests:
        code, stdout, stderr = run_router(t["args"])
        assert code == 0, f"Process crashed on {t['name']}: {stderr}"

        log = parse_last_log()
        status_ok = log["STATUS"] == t["expect_status"]
        time_ok = int(log["TRAVEL_SEC"]) >= t["expect_min_sec"]
        exec_times.append(float(log["EXEC_MS"]))

        if status_ok and time_ok:
            print(f"[PASS] {t['name']} (STATUS={log['STATUS']}, TIME={log['TRAVEL_SEC']}s, EXEC={log['EXEC_MS']}ms)")
            passed += 1
        else:
            print(f"[FAIL] {t['name']} -> Got {log}")

    avg_ms = sum(exec_times) / len(exec_times)
    print(f"\n=== Summary: {passed}/{len(tests)} Passed | Avg Query Time: {avg_ms:.3f} ms ===")

    if passed != len(tests):
        sys.exit(1)

if __name__ == "__main__":
    main()