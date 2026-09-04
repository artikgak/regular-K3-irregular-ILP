import re
import matplotlib.pyplot as plt
import numpy as np
from datetime import timedelta

import sys
import os

log_file = sys.argv[1] if len(sys.argv) > 1 else r"N21_R8_K3_2_22_split_22_fix2B_4_wo_lem_new.log"
log_basename = os.path.basename(log_file)
log_name = os.path.splitext(log_basename)[0]

times_sec = []
nodes = []
left_nodes = []
lp_iters = []
lp_per_node = []
mem_mb = []
max_depth = []
conflicts = []
completion = []
cuts_list = []

with open(log_file, 'r') as f:
    for line in f:
        line = line.strip()
        # Skip header lines and empty lines
        if not line or line.startswith('time') or line.startswith('SCIP') or line.startswith('Copyright'):
            continue
        
        # Match data lines - time can be like "0.0s", "123s", "123m", "1234s"
        # Pattern: time|node|left|LP iter|LP it/n|mem|mdpt|vars|cons|rows|cuts|sepa|confs|strbr|dual|primal|gap|compl
        m = re.match(
            r'\s*([\d.]+)(s|m)\s*\|\s*([\d.]+[A-Za-z]?)\s*\|\s*(\d+)\s*\|\s*([\d.]+[A-Za-z]?)\s*\|\s*([\d.]+|-)\s*\|\s*(\d+)M'
            r'.*?\|\s*(\d+)\s*\|.*?\|.*?\|.*?\|([\s\d.]+[A-Za-z]?)\s*\|\s*\d+\s*\|([\s\d.]+[A-Za-z]?)\s*\|.*?\|.*?\|.*?\|.*?\|\s*([\d.]+%|unknown.*?)',
            line
        )
        if m:
            t_val = float(m.group(1))
            t_unit = m.group(2)
            if t_unit == 'm':
                t_sec = t_val * 60
            else:
                t_sec = t_val
            
            node_str = m.group(3).strip()
            if node_str[-1].isalpha():
                mult = 1000 if node_str[-1].lower() == 'k' else 1000000 if node_str[-1].lower() == 'm' else 1
                node = int(float(node_str[:-1]) * mult)
            else:
                node = int(node_str)
                
            left = int(m.group(4))
            
            lp_str = m.group(5).strip()
            if lp_str[-1].isalpha():
                mult = 1000 if lp_str[-1].lower() == 'k' else 1000000 if lp_str[-1].lower() == 'm' else 1
                lp_val = float(lp_str[:-1]) * mult
            else:
                lp_val = float(lp_str)
            
            lpn = m.group(6)
            lpn_val = float(lpn) if lpn != '-' else 0
            
            mem = int(m.group(7))
            
            mdpt_val = int(m.group(8))
            
            cuts_str = m.group(9).strip()
            if cuts_str[-1].isalpha():
                mult = 1000 if cuts_str[-1].lower() == 'k' else 1000000 if cuts_str[-1].lower() == 'm' else 1
                cuts_val = float(cuts_str[:-1]) * mult
            else:
                cuts_val = float(cuts_str)
            
            confs_str = m.group(10).strip()
            if confs_str[-1].isalpha():
                mult = 1000 if confs_str[-1].lower() == 'k' else 1000000 if confs_str[-1].lower() == 'm' else 1
                confs_val = float(confs_str[:-1]) * mult
            else:
                confs_val = float(confs_str)
            
            compl_str = m.group(11)
            if compl_str == 'unknown':
                compl_val = 0.0
            else:
                compl_val = float(compl_str.replace('%', ''))
            
            times_sec.append(t_sec)
            nodes.append(node)
            left_nodes.append(left)
            lp_iters.append(lp_val)
            lp_per_node.append(lpn_val)
            mem_mb.append(mem)
            max_depth.append(mdpt_val)
            conflicts.append(confs_val)
            completion.append(compl_val)
            cuts_list.append(cuts_val)

times_sec = np.array(times_sec)
nodes = np.array(nodes)
left_nodes = np.array(left_nodes)
lp_iters = np.array(lp_iters)
lp_per_node = np.array(lp_per_node)
mem_mb = np.array(mem_mb)
max_depth = np.array(max_depth)
conflicts = np.array(conflicts)
completion = np.array(completion)
cuts_list = np.array(cuts_list)

times_min = times_sec / 60
times_hr = times_sec / 3600

# Print summary statistics
print("=" * 70)
print("SCIP Log Analysis Summary")
print("=" * 70)
print(f"Problem: {log_name}")
print(f"Total runtime: {times_sec[-1]:.0f}s = {times_min[-1]:.1f}min = {times_hr[-1]:.2f}hr")
print(f"Nodes explored: {nodes[-1]:,}")
print(f"Open nodes (left): {left_nodes[-1]:,}")
print(f"LP iterations: {lp_iters[-1]:,.0f}")
print(f"LP iter/node (current): {lp_per_node[-1]:.1f}")
print(f"Memory: {mem_mb[-1]} MB")
print(f"Conflicts: {conflicts[-1]:,.0f}")
print(f"Cuts: {cuts_list[-1]:,.0f}")
print(f"Completion: {completion[-1]:.2f}%")
print(f"Max tree depth: {max_depth[-1]}")
print()

# Estimate completion
# Use last N data points to compute rate of completion change
N = min(200, len(completion))
recent_compl = completion[-N:]
recent_time = times_sec[-N:]

if len(recent_compl) > 1 and (recent_compl[-1] - recent_compl[0]) > 0:
    rate = (recent_compl[-1] - recent_compl[0]) / (recent_time[-1] - recent_time[0])  # %/sec
    remaining_pct = 100.0 - completion[-1]
    if rate > 0:
        est_remaining_sec = remaining_pct / rate
        est_remaining_hr = est_remaining_sec / 3600
        est_total_sec = times_sec[-1] + est_remaining_sec
        est_total_hr = est_total_sec / 3600
        
        print(f"--- Completion Estimation (linear extrapolation) ---")
        print(f"Recent completion rate: {rate * 3600:.4f} %/hr")
        print(f"Remaining: {remaining_pct:.2f}%")
        print(f"Estimated remaining time: {est_remaining_hr:.1f} hours ({est_remaining_hr/24:.1f} days)")
        print(f"Estimated total time: {est_total_hr:.1f} hours ({est_total_hr/24:.1f} days)")
    else:
        print("WARNING: Completion rate is zero or negative - cannot estimate.")
else:
    print("Not enough data to estimate completion rate.")

# Also estimate with different windows
print(f"\n--- Estimation with different time windows ---")
for window_name, window_size in [("Last 50 pts", 50), ("Last 100 pts", 100), ("Last 500 pts", 500), ("All data", len(completion))]:
    ws = min(window_size, len(completion))
    c = completion[-ws:]
    t = times_sec[-ws:]
    if len(c) > 1 and (c[-1] - c[0]) > 0:
        r = (c[-1] - c[0]) / (t[-1] - t[0])
        rem = (100.0 - c[-1]) / r
        print(f"  {window_name}: rate={r*3600:.4f}%/hr, remaining={rem/3600:.1f} hr ({rem/86400:.1f} days)")
    else:
        print(f"  {window_name}: insufficient data or no progress")

# Node throughput
print(f"\n--- Node Throughput ---")
for window_name, window_size in [("Last 50 pts", 50), ("Last 100 pts", 100), ("Overall", len(nodes))]:
    ws = min(window_size, len(nodes))
    n = nodes[-ws:]
    t = times_sec[-ws:]
    if len(n) > 1:
        r = (n[-1] - n[0]) / (t[-1] - t[0])
        print(f"  {window_name}: {r:.2f} nodes/sec = {r*60:.1f} nodes/min")

print("=" * 70)

# Create plots
fig, axes = plt.subplots(3, 2, figsize=(16, 14))
fig.suptitle(f'SCIP Solver Progress: {log_name}', fontsize=14, fontweight='bold')

# 1. Nodes explored & open nodes vs time
ax = axes[0, 0]
ax.plot(times_hr, nodes/1000, 'b-', linewidth=1, label='Explored (×1000)')
ax.plot(times_hr, left_nodes, 'r-', linewidth=1, alpha=0.7, label='Open nodes')
ax.set_xlabel('Time (hours)')
ax.set_ylabel('Nodes')
ax.set_title('Nodes Explored & Open Nodes')
ax.legend()
ax.grid(True, alpha=0.3)

# 2. Completion % vs time
ax = axes[0, 1]
ax.plot(times_hr, completion, 'g-', linewidth=1.5)
ax.set_xlabel('Time (hours)')
ax.set_ylabel('Completion (%)')
ax.set_title('Search Space Completion')
ax.grid(True, alpha=0.3)
ax.set_ylim(0, 100)
# Add a horizontal line at current completion
ax.axhline(y=completion[-1], color='r', linestyle='--', alpha=0.5, label=f'Current: {completion[-1]:.2f}%')
ax.legend()

# 3. LP iterations per node
ax = axes[1, 0]
valid = lp_per_node > 0
ax.plot(times_hr[valid], lp_per_node[valid], 'purple', linewidth=0.5, alpha=0.5)
# Moving average
if len(lp_per_node[valid]) > 20:
    window = 20
    ma = np.convolve(lp_per_node[valid], np.ones(window)/window, mode='valid')
    ax.plot(times_hr[valid][window-1:], ma, 'purple', linewidth=2, label=f'MA-{window}')
ax.set_xlabel('Time (hours)')
ax.set_ylabel('LP iter / node')
ax.set_title('LP Iterations per Node')
ax.legend()
ax.grid(True, alpha=0.3)

# 4. Memory usage
ax = axes[1, 1]
ax.plot(times_hr, mem_mb, 'orange', linewidth=1.5)
ax.set_xlabel('Time (hours)')
ax.set_ylabel('Memory (MB)')
ax.set_title('Memory Usage')
ax.grid(True, alpha=0.3)

# 5. Conflicts
ax = axes[2, 0]
ax.plot(times_hr, conflicts/1000, 'red', linewidth=1)
ax.set_xlabel('Time (hours)')
ax.set_ylabel('Conflicts (×1000)')
ax.set_title('Conflict Constraints Generated')
ax.grid(True, alpha=0.3)

# 6. Cuts generated
ax = axes[2, 1]
ax.plot(times_hr, cuts_list/1000, 'teal', linewidth=1)
ax.set_xlabel('Time (hours)')
ax.set_ylabel('Cuts (×1000)')
ax.set_title('Cutting Planes Generated')
ax.grid(True, alpha=0.3)

plt.tight_layout()
out_png = f'{log_name}_analysis.png'
plt.savefig(out_png, dpi=150, bbox_inches='tight')
# plt.show()
print(f"\nPlot saved to {out_png}")
