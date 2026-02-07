PA02: Analysis of Network I/O Primitives
Graduate Systems (CSE638)

Author      : Adarsh Shukla
Roll Number : PhD25001
Institute   : IIIT Delhi
Date        : February 7, 2026


=====================================================================
OVERVIEW
=====================================================================

This assignment implements and analyzes three TCP socket communication
methods to study the cost of data movement in Network I/O.

The following mechanisms are compared using multithreaded client-server
architectures:

1. Two-Copy (Baseline)
2. One-Copy (Scatter-Gather I/O)
3. Zero-Copy (MSG_ZEROCOPY)


=====================================================================
DIRECTORY STRUCTURE
=====================================================================

PhD25001_PA02/
|
|-- PhD25001_Part_A1_Server.c
|     Part A1: Two-Copy Server (Baseline)
|
|-- PhD25001_Part_A1_Client.c
|     Part A1: Two-Copy Client
|
|-- PhD25001_Part_A2_Server.c
|     Part A2: One-Copy Server (iovec scatter-gather)
|
|-- PhD25001_Part_A2_Client.c
|     Part A2: One-Copy Client
|
|-- PhD25001_Part_A3_Server.c
|     Part A3: Zero-Copy Server (MSG_ZEROCOPY)
|
|-- PhD25001_Part_A3_Client.c
|     Part A3: Zero-Copy Client
|
|-- PhD25001_Part_A_common.h
|     Shared header (message structures, timing utilities)
|
|-- PhD25001_Part_C_run_experiments.sh
|     Automated script for running experiments
|
|-- PhD25001_Part_D_plot_results.py
|     Python script for generating plots
|
|-- PhD25001_Part_C_measurements.csv
|     Throughput and latency measurements (client-side)
|
|-- PhD25001_Part_C_perf_measurements.csv
|     CPU cycles and cache miss data (server-side)
|
|-- Makefile
|     Build automation
|
|-- PhD25001_Report.pdf
|     Comprehensive report with analysis
|
|-- PhD25001_Part_A_Readme.txt
|     Assignment README


=====================================================================
COMPILATION
=====================================================================

To compile all server and client binaries:

make all

To clean binaries before submission:

make clean


=====================================================================
USAGE
=====================================================================

Manual Execution (Single Run)

Server:
./PhD25001_Part_A1_Server <port> <msg_size> <num_threads>

Client:
./PhD25001_Part_A1_Client <server_ip> <port> <msg_size> <num_threads>

Example (Two-Copy, 16 KB message, 4 threads):

./PhD25001_Part_A1_Server 5000 16384 4
./PhD25001_Part_A1_Client 127.0.0.1 5000 16384 4


=====================================================================
AUTOMATED EXPERIMENTS (PART C)
=====================================================================

The provided script automates compilation, execution across message sizes
(64 bytes to 64 KB), and data collection.

On the SERVER machine:

./PhD25001_Part_C_run_experiments.sh server

On the CLIENT machine (after server is waiting):

./PhD25001_Part_C_run_experiments.sh client

Generated output files:

- PhD25001_Part_C_measurements.csv
- PhD25001_Part_C_perf_measurements.csv


=====================================================================
PLOTTING RESULTS (PART D)
=====================================================================

To generate the required plots (throughput, latency, cache misses, and
CPU cycles), run:

python3 PhD25001_Part_D_plot_results.py

Note:
Data points are hardcoded in the plotting script to ensure portability
as per assignment requirements.

Output:
Four PNG files (e.g., PhD25001_Part_D_throughput.png)


=====================================================================
IMPLEMENTATION DETAILS
=====================================================================

Part A1: Two-Copy
- Uses a user-space buffer to aggregate 8 distinct string fields
  (First copy)
- send() triggers a kernel-space copy (Second copy)

Part A2: One-Copy
- Uses struct iovec to directly reference the 8 fields
- sendmsg() eliminates the user-space aggregation copy

Part A3: Zero-Copy
- Enables the SO_ZEROCOPY socket option
- Uses sendmsg() with MSG_ZEROCOPY
- User pages are pinned by the kernel; no data copying occurs
- Implements opportunistic polling on the error queue to prevent
  deadlock on small message sizes


=====================================================================
SYSTEM CONFIGURATION
=====================================================================

Server:
- Intel B760M-Pro RS-D4
- Linux Kernel 6.8.0

Client:
- Linux Workstation
- Linux Kernel 5.15.0


=====================================================================
DEPENDENCIES
=====================================================================

- GCC compiler (with -pthread and -lrt support)
- Python 3 with matplotlib
- Linux Kernel 4.14 or later (MSG_ZEROCOPY support)
- perf tool for CPU and cache profiling


=====================================================================
AI USAGE DECLARATION
=====================================================================

Generative AI (Google Gemini) was used for the following components:

1. Part A3 Server Debugging
   Issue:
   Zero-Copy server hanging on small message sizes (64 bytes)

   Prompt Used:
   "My Zero-Copy server gets stuck on small messages. How to fix?
    It seems like poll is waiting forever."

   Resolution:
   Implemented opportunistic polling to handle cases where the kernel
   optimizes away completion notifications for small packets.

2. Part D Plotting Script
   Issue:
   Requirement to hardcode experiment data values

   Prompt Used:
   "Generate a matplotlib script using these hardcoded values for
    throughput, latency, and cache misses."

   Resolution:
   AI generated the plotting script using the provided experiment data.

All code logic has been reviewed, understood, and verified by the author.


=====================================================================
GITHUB REPOSITORY
=====================================================================

https://github.com/WiseShukla/GRS_PA02


=====================================================================
LICENSE
=====================================================================

This project is submitted as part of academic coursework at IIIT Delhi.
