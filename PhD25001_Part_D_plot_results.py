# PhD25001
# Roll No: PhD25001
# Part D: Plotting and Visualization

import matplotlib
matplotlib.use('Agg') # Non-interactive backend
import matplotlib.pyplot as plt
import os

# ======================== CONFIGURATION ========================

PLOTS_DIR = "."  
SYSTEM_CONFIG = "Server: Intel B760M-Pro RS-D4 (Kernel 6.8.0)\nClient: arani@gamma-pc (Kernel 5.15.0)"

msg_sizes = [64, 1024, 16384, 65536]
thread_counts = [1, 2, 4, 8]

# ======================== VERIFIED HARDCODED DATA ========================

# 1. Throughput (Gbps) - Thread=1

tp_twocopy  = [0.855762, 0.865371, 0.877737, 0.902509]
tp_onecopy  = [0.879652, 0.862034, 0.860619, 0.897162]
tp_zerocopy = [0.042701, 0.899300, 0.905262, 0.901880]

# 2. Latency (us) - Msg Size=64B

lat_twocopy  = [0.58, 1.12, 2.35, 4.53]
lat_onecopy  = [0.57, 1.18, 2.33, 4.52]
lat_zerocopy = [11.97, 11.88, 12.13, 12.10]

# 3. LLC Cache Misses - Thread=1
llc_twocopy  = [91508, 24225, 23662, 19909]
llc_onecopy  = [53125, 20218, 27777, 30535]
llc_zerocopy = [2937, 28905, 27450, 35114]

# 4. CPU Cycles Per Byte - Thread=1
cpb_twocopy  = [19.01, 4.48, 3.83, 4.28]
cpb_onecopy  = [19.00, 4.48, 3.67, 4.19]
cpb_zerocopy = [24.86, 4.52, 4.01, 3.99]

# ======================== PLOTTING ENGINE ========================

def apply_scientific_style(title, xlabel, ylabel, is_log_y=True, is_log_x=True):
    plt.title(title, fontsize=14, fontweight='bold', pad=20)
    plt.xlabel(xlabel, fontsize=12, labelpad=10)
    plt.ylabel(ylabel, fontsize=12, labelpad=10)
    
    if is_log_x:
        plt.xscale('log', base=2)
        plt.xticks(msg_sizes, ['64B', '1KB', '16KB', '64KB'])
    
    if is_log_y:
        plt.yscale('log')
        
    plt.legend(loc='best', frameon=True, shadow=True, fontsize=10)
    plt.grid(True, which="both", ls="--", alpha=0.5)
    
    plt.figtext(0.5, 0.01, SYSTEM_CONFIG, ha="center", fontsize=9, 
                style='italic', bbox={"facecolor":"white", "edgecolor":"black", "alpha":0.8, "pad":5})
    plt.tight_layout(rect=[0, 0.08, 1, 0.95])

def generate_plots():
    # Plot 1: Throughput
    plt.figure(figsize=(10, 7))
    plt.plot(msg_sizes, tp_twocopy, color='red', linestyle='-', linewidth=4, marker='o', markersize=10, label='Two-Copy (Baseline)')
    plt.plot(msg_sizes, tp_onecopy, color='blue', linestyle='--', linewidth=2, marker='s', markersize=8, label='One-Copy')
    plt.plot(msg_sizes, tp_zerocopy, color='green', linestyle=':', linewidth=2, marker='^', markersize=8, label='Zero-Copy')
    apply_scientific_style('Network Throughput vs Message Size (Thread=1)', 
                           'Message Size (Bytes)', 'Throughput (Gbps)', is_log_y=False, is_log_x=True)
    
    fname = os.path.join(PLOTS_DIR, "PhD25001_Part_D_throughput.png")
    plt.savefig(fname, dpi=300)
    print(f"Generated {fname}")

    # Plot 2: Latency
    plt.figure(figsize=(10, 7))
    plt.plot(thread_counts, lat_twocopy, color='red', linestyle='-', linewidth=4, marker='o', label='Two-Copy')
    plt.plot(thread_counts, lat_onecopy, color='blue', linestyle='--', linewidth=2, marker='s', label='One-Copy')
    plt.plot(thread_counts, lat_zerocopy, color='green', linestyle=':', linewidth=2, marker='^', label='Zero-Copy')
    plt.xticks(thread_counts)
    apply_scientific_style('Latency Scaling vs Thread Count (64B)', 
                           'Number of Threads', 'Latency (µs)', is_log_y=True, is_log_x=False)
    
    fname = os.path.join(PLOTS_DIR, "PhD25001_Part_D_latency.png")
    plt.savefig(fname, dpi=300)
    print(f"Generated {fname}")

    # Plot 3: LLC Misses
    plt.figure(figsize=(10, 7))
    plt.plot(msg_sizes, llc_twocopy, color='red', linestyle='-', linewidth=3, marker='o', label='Two-Copy')
    plt.plot(msg_sizes, llc_onecopy, color='blue', linestyle='--', linewidth=2, marker='s', label='One-Copy')
    plt.plot(msg_sizes, llc_zerocopy, color='green', linestyle=':', linewidth=2, marker='^', label='Zero-Copy')
    apply_scientific_style('LLC Cache Misses vs Message Size (Thread=1)', 
                           'Message Size (Bytes)', 'Miss Count', is_log_y=True, is_log_x=True)
    
    fname = os.path.join(PLOTS_DIR, "PhD25001_Part_D_cache_misses.png")
    plt.savefig(fname, dpi=300)
    print(f"Generated {fname}")

    # Plot 4: CPU Cycles
    plt.figure(figsize=(10, 7))
    plt.plot(msg_sizes, cpb_twocopy, color='red', linestyle='-', linewidth=3, marker='o', label='Two-Copy')
    plt.plot(msg_sizes, cpb_onecopy, color='blue', linestyle='--', linewidth=2, marker='s', label='One-Copy')
    plt.plot(msg_sizes, cpb_zerocopy, color='green', linestyle=':', linewidth=2, marker='^', label='Zero-Copy')
    apply_scientific_style('CPU Cycles per Byte vs Message Size (Thread=1)', 
                           'Message Size (Bytes)', 'Cycles / Byte', is_log_y=False, is_log_x=True)
    
    fname = os.path.join(PLOTS_DIR, "PhD25001_Part_D_cycles_per_byte.png")
    plt.savefig(fname, dpi=300)
    print(f"Generated {fname}")

if __name__ == "__main__":
    generate_plots()