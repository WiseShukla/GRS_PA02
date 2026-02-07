#!/bin/bash
###############################################################################
# PhD25001_Part_C_run_experiments.sh
# Roll No: PhD25001
#
# This script:
# 1. Compiles implementations.
# 2. Runs experiments with robust synchronization.
# 3. Hides messy logs in 'PhD25001_Part_C_Logs/'.
# 4. Saves final CSVs in the root directory.
###############################################################################

set -e

# ======================== CONFIGURATION ========================
SERVER_IP="192.168.226.224"
CLIENT_IP="192.168.23.48"
BASE_PORT=5000
DURATION=10 

DEFAULT_MSG_SIZES="64 1024 16384 65536"
DEFAULT_THREAD_COUNTS="1 2 4 8"
IMPLEMENTATIONS="A1 A2 A3" 

LOG_DIR="PhD25001_Part_C_Logs"
CSV_FILE="PhD25001_Part_C_measurements.csv"
PERF_CSV="PhD25001_Part_C_perf_measurements.csv"

# ======================== FUNCTIONS ========================

compile_all() {
    echo "=============================="
    echo "Compiling all implementations..."
    echo "=============================="
    make clean
    make all
    echo "Compilation successful."
}

init() {
    local mode=$1
    mkdir -p "$LOG_DIR"
    
    # Initialize CSVs based on MODE
    if [ "$mode" == "server" ]; then
        if [ ! -f "$PERF_CSV" ]; then
            echo "implementation,msg_size,num_threads,cpu_cycles,L1_misses,LLC_misses,context_switches,cpu_clock_ms" > "$PERF_CSV"
        fi
    fi

    if [ "$mode" == "client" ]; then
        # Client needs a fresh start for measurements if running from scratch
        if [ ! -f "$CSV_FILE" ]; then
            echo "implementation,msg_size,num_threads,throughput_gbps,avg_latency_us,total_bytes" > "$CSV_FILE"
        fi
        if [ ! -f "$PERF_CSV" ]; then
            echo "implementation,msg_size,num_threads,cpu_cycles,L1_misses,LLC_misses,context_switches,cpu_clock_ms" > "$PERF_CSV"
        fi
    fi
}

parse_perf_output() {
    local perf_file=$1
    local impl=$2
    local msg_size=$3
    local threads=$4

    local cycles=$(grep -E "cycles" "$perf_file" | grep -v "stalled" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local l1_misses=$(grep -iE "L1-dcache-load-misses|L1.*miss" "$perf_file" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local llc_misses=$(grep -iE "LLC-load-misses|LLC.*miss|cache-misses" "$perf_file" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local ctx_switches=$(grep -iE "context-switch" "$perf_file" | head -1 | awk '{gsub(/,/,"",$1); print $1}')
    local cpu_clock=$(grep -iE "task-clock|cpu-clock" "$perf_file" | head -1 | awk '{gsub(/,/,"",$1); print $1}')

    echo "${impl},${msg_size},${threads},${cycles:-0},${l1_misses:-0},${llc_misses:-0},${ctx_switches:-0},${cpu_clock:-0}" >> "$PERF_CSV"
}

run_server_mode() {
    local msg_sizes="${1:-$DEFAULT_MSG_SIZES}"
    local thread_counts="${2:-$DEFAULT_THREAD_COUNTS}"

    compile_all
    init "server"

    for impl in $IMPLEMENTATIONS; do
        # RESET PORT for each implementation to keep sync with Client
        local port=$BASE_PORT 
        
        for msg_size in $msg_sizes; do
            for threads in $thread_counts; do
                local perf_file="${LOG_DIR}/PhD25001_Part_${impl}_Server_msg${msg_size}_t${threads}_perf.txt"
                
                echo "Running: Part ${impl} Server | msg=${msg_size} | threads=${threads} | port=${port}"
                
                perf stat -e cpu-clock,context-switches,cpu-cycles,L1-dcache-load-misses,LLC-load-misses \
                    -o "$perf_file" \
                    "./PhD25001_Part_${impl}_Server" "$port" "$msg_size" "$threads"

                parse_perf_output "$perf_file" "$impl" "$msg_size" "$threads"
                port=$((port + 1))
            done
        done
    done
    
    echo "Server Experiments Complete!"
}

run_client_mode() {
    local msg_sizes="${1:-$DEFAULT_MSG_SIZES}"
    local thread_counts="${2:-$DEFAULT_THREAD_COUNTS}"

    compile_all
    init "client"

    for impl in $IMPLEMENTATIONS; do
        # RESET PORT for each implementation
        local port=$BASE_PORT 

        for msg_size in $msg_sizes; do
            for threads in $thread_counts; do
                local perf_file="${LOG_DIR}/PhD25001_Part_${impl}_Client_msg${msg_size}_t${threads}_perf.txt"
                local output_file="${LOG_DIR}/PhD25001_Part_${impl}_Client_msg${msg_size}_t${threads}_output.txt"

                echo "Running: Part ${impl} Client | msg=${msg_size} | threads=${threads} | port=${port}"

                # --- CRITICAL FIX: Wait for Server to start ---
                sleep 2 

                perf stat -e cpu-clock,context-switches,cpu-cycles,L1-dcache-load-misses,LLC-load-misses \
                    -o "$perf_file" \
                    "./PhD25001_Part_${impl}_Client" "$SERVER_IP" "$port" "$msg_size" "$threads" "$DURATION" \
                    > "$output_file" 2>&1

                local result_line=$(grep "^RESULT," "$output_file" 2>/dev/null || true)
                if [ -n "$result_line" ]; then
                    echo "${result_line#RESULT,}" >> "$CSV_FILE"
                else
                    # Fallback if connection failed
                    echo "${impl},${msg_size},${threads},0,0,0" >> "$CSV_FILE"
                fi

                parse_perf_output "$perf_file" "$impl" "$msg_size" "$threads"
                port=$((port + 1))
            done
        done
    done
    
    echo "Client Experiments Complete!"
}

# Main execution logic
MODE="${1:-}"
case "$MODE" in
    server) run_server_mode "$2" "$3" ;;
    client) run_client_mode "$2" "$3" ;;
    *) echo "Usage: $0 {server|client}"; exit 1 ;;
esac