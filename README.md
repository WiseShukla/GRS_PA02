# PA02: Analysis of Network I/O Primitives

**Course:** CSE638 - Graduate Systems  
**Author:** Adarsh Shukla  
**Roll Number:** PhD25001  
**Institute:** IIIT Delhi  
**Date:** February 7, 2026

## Naming Convention

All files in this project follow a strict naming convention:

```
PhD25001_Part_<Section>_<Description>.<extension>

Where:
- PhD25001        : Roll Number
- Part_A1/A2/A3   : Implementation parts (Two-Copy/One-Copy/Zero-Copy)
- Part_C          : Experiment automation and data collection
- Part_D          : Data visualization and plotting
- Description     : Functional description (Server, Client, common, etc.)
```

**Examples:**
- `PhD25001_Part_A1_Server.c` - Two-Copy server implementation
- `PhD25001_Part_C_run_experiments.sh` - Experiment automation script
- `PhD25001_Part_D_plot_results.py` - Plotting script
- `PhD25001_Report.pdf` - Final comprehensive report

---

## Overview

This assignment implements and analyzes three TCP socket communication methods to study the cost of data movement in Network I/O. It compares standard Two-Copy, optimized One-Copy, and Zero-Copy (MSG_ZEROCOPY) mechanisms using multithreaded client-server architectures.

**Three Methods Compared:**
1. **Two-Copy (Part A1):** Standard socket programming with user-space and kernel-space copies
2. **One-Copy (Part A2):** Using iovec scatter-gather I/O to eliminate user-space aggregation
3. **Zero-Copy (Part A3):** Leveraging MSG_ZEROCOPY to pin user pages and avoid data copying

---

## Directory Structure

```
PhD25001_PA02/
├── PhD25001_Part_A1_Server.c      # Part A1: Two-Copy Server (Baseline)
├── PhD25001_Part_A1_Client.c      # Part A1: Two-Copy Client
├── PhD25001_Part_A2_Server.c      # Part A2: One-Copy Server (iovec scatter-gather)
├── PhD25001_Part_A2_Client.c      # Part A2: One-Copy Client
├── PhD25001_Part_A3_Server.c      # Part A3: Zero-Copy Server (MSG_ZEROCOPY)
├── PhD25001_Part_A3_Client.c      # Part A3: Zero-Copy Client
├── PhD25001_Part_A_common.h       # Shared header (Message structs, time utils)
├── PhD25001_Part_C_run_experiments.sh # Automated Bash script for experiments
├── PhD25001_Part_D_plot_results.py    # Python script for generating plots
├── PhD25001_Part_C_measurements.csv   # Throughput/Latency Data (Client Side)
├── PhD25001_Part_C_perf_measurements.csv # CPU/Cache Data (Server Side)
├── Makefile                       # Build automation
├── PhD25001_Report.pdf            # Comprehensive report with analysis
└── README.md                      # This file
```

---

## Compilation

To compile all server and client binaries:

```bash
make all
```

To clean up binaries before submission:

```bash
make clean
```

---

## Usage

### 1. Manual Execution (Single Run)

**Server:**
```bash
./PhD25001_Part_A1_Server <port> <msg_size> <num_threads>
```

**Client:**
```bash
./PhD25001_Part_A1_Client <server_ip> <port> <msg_size> <num_threads>
```

**Example (Two-Copy, 16KB Message, 4 Threads):**
```bash
# Terminal 1 (Server)
./PhD25001_Part_A1_Server 5000 16384 4

# Terminal 2 (Client)
./PhD25001_Part_A1_Client 127.0.0.1 5000 16384 4
```

### 2. Automated Experiments (Part C)

The provided script automates compilation, execution (across 64B to 64KB messages), and data collection.

**On the SERVER Machine:**
```bash
./PhD25001_Part_C_run_experiments.sh server
```

**On the CLIENT Machine (after Server is waiting):**
```bash
./PhD25001_Part_C_run_experiments.sh client
```

**Output:** Generates the .csv files listed in the Directory Structure.

### 3. Plotting Results (Part D)

Generate the 4 required plots (Throughput, Latency, Cache Misses, CPU Cycles):

```bash
python3 PhD25001_Part_D_plot_results.py
```

**Note:** Data points are hardcoded in the script to ensure portability as per assignment rules.

**Output:** 4 PNG files
- `PhD25001_Part_D_throughput.png`
- `PhD25001_Part_D_latency.png`
- `PhD25001_Part_D_cache_misses.png`
- `PhD25001_Part_D_cpu_cycles.png`

---

## Implementation Details

### Part A1 (Two-Copy)
- Uses a user-space buffer to aggregate 8 distinct string fields (Copy 1)
- Calls send(), prompting a kernel-space copy (Copy 2)

### Part A2 (One-Copy)
- Uses struct iovec to point to the 8 fields directly
- Calls sendmsg(), eliminating the user-space aggregation copy

### Part A3 (Zero-Copy)
- Sets the SO_ZEROCOPY socket option
- Uses sendmsg() with MSG_ZEROCOPY flag
- User pages are pinned by the kernel; no data copying occurs
- Implements opportunistic polling on the error queue to prevent deadlock on small messages

---

## System Configuration

- **Server:** Intel B760M-Pro RS-D4 (Kernel 6.8.0)
- **Client:** Linux Workstation (Kernel 5.15.0)

---

## Dependencies

- GCC compiler (with -pthread -lrt support)
- Python 3 with matplotlib (for plotting)
- Linux Kernel 4.14+ (for MSG_ZEROCOPY support)
- perf tool (for CPU/Cache profiling)

---

## AI Usage Declaration

Generative AI (Google Gemini) was used for the following specific components:

**1. Component: Part A3 Server Debugging**
- **Issue:** Zero-Copy server hanging on small message sizes (64 bytes)
- **Prompt Used:** "My Zero-Copy server gets stuck on small messages. How to fix? It seems like poll is waiting forever."
- **Resolution:** AI suggested implementing "Opportunistic Polling" to handle cases where the kernel optimizes away notifications for small packets

**2. Component: Part D Plotting Script**
- **Issue:** Requirement to hardcode data values in the plotting script
- **Prompt Used:** "Generate a matplotlib script using these hardcoded values for throughput, latency, and cache misses."
- **Resolution:** AI generated the 'PhD25001_Part_D_plot_results.py' script using my experiment data

All code logic has been understood, verified, and tested by the author.

---

## GitHub Repository

https://github.com/WiseShukla/GRS_PA02

---

## License

This project is submitted as part of academic coursework at IIIT Delhi.