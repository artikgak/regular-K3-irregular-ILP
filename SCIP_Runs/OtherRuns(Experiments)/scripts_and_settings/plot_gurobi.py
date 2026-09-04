import sys
import os
import numpy as np
import matplotlib.pyplot as plt

def parse_log(filepath):
    data = []
    with open(filepath, 'r') as f:
        for line in f:
            parts = line.split()
            # check if line looks like a log node line ending with 's'
            if len(parts) >= 8 and parts[-1].endswith('s') and parts[0].isdigit() and parts[1].isdigit():
                try:
                    expl = int(parts[0])
                    unexpl = int(parts[1])
                    time_s = int(parts[-1][:-1])
                    depth = int(parts[3])
                    it_node = float(parts[-2])
                    
                    if not data or data[-1][2] != time_s:
                        data.append((expl, unexpl, time_s, depth, it_node))
                except ValueError:
                    pass # skip lines that fail to parse
    return data

def plot_data(data, output_path):
    if len(data) < 2:
        print("Not enough data to plot.")
        return
        
    expl = np.array([d[0] for d in data])
    unexpl = np.array([d[1] for d in data])
    times = np.array([d[2] for d in data])
    depths = np.array([d[3] for d in data])
    it_nodes = np.array([d[4] for d in data])
    
    plt.figure(figsize=(14, 10))
    
    # 1. Unexplored nodes (tree size)
    plt.subplot(2, 2, 1)
    plt.plot(times, unexpl, label='Unexplored Nodes', color='red')
    plt.title('Gurobi Unexplored Nodes (Tree Size)')
    plt.xlabel('Time (s)')
    plt.ylabel('Count')
    plt.grid(True)
    plt.legend()
    
    # 2. Speeds
    dt = np.diff(times)
    valid = dt > 0
    dt = dt[valid]
    v_expl = np.diff(expl)[valid] / dt
    v_unexpl = np.diff(unexpl)[valid] / dt
    t_mid = times[:-1][valid] + dt / 2.0
    
    window = min(50, len(v_expl))
    if window > 0:
        v_expl_smooth = np.convolve(v_expl, np.ones(window)/window, mode='valid')
        v_unexpl_smooth = np.convolve(v_unexpl, np.ones(window)/window, mode='valid')
        t_mid_smooth = t_mid[window-1:]
        
        plt.subplot(2, 2, 2)
        plt.plot(t_mid_smooth, v_expl_smooth, label='Explored Speed (nodes/s)', color='blue')
        plt.plot(t_mid_smooth, v_unexpl_smooth, label='Unexplored Speed (nodes/s)', color='orange')
        plt.title(f'Processing Speed (Smoothed, w={window})')
        plt.xlabel('Time (s)')
        plt.ylabel('Nodes/sec')
        plt.grid(True)
        plt.legend()
        
    # 3. Tree Depth
    plt.subplot(2, 2, 3)
    plt.plot(times, depths, label='Current Depth', color='purple', alpha=0.6)
    
    # Add smoothed depth trend
    window_depth = min(100, len(depths))
    if window_depth > 0:
        depths_smooth = np.convolve(depths, np.ones(window_depth)/window_depth, mode='valid')
        times_smooth = times[window_depth-1:]
        plt.plot(times_smooth, depths_smooth, label='Depth Trend (MA)', color='darkviolet', linewidth=2)
        
    plt.title('Exploration Depth Over Time')
    plt.xlabel('Time (s)')
    plt.ylabel('Depth')
    plt.grid(True)
    plt.legend()
    
    # 4. LP Iterations per Node
    plt.subplot(2, 2, 4)
    plt.plot(times, it_nodes, label='It/Node', color='green', alpha=0.6)
    
    # Add smoothed It/Node trend
    window_it = min(100, len(it_nodes))
    if window_it > 0:
        it_smooth = np.convolve(it_nodes, np.ones(window_it)/window_it, mode='valid')
        it_times_smooth = times[window_it-1:]
        plt.plot(it_times_smooth, it_smooth, label='It/Node Trend (MA)', color='darkgreen', linewidth=2)
        
    plt.title('LP Iterations per Node')
    plt.xlabel('Time (s)')
    plt.ylabel('Iterations')
    plt.grid(True)
    plt.legend()
        
    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved plot to {output_path}")

if __name__ == '__main__':
    filepath = sys.argv[1]
    if len(sys.argv) > 2:
        outpath = sys.argv[2]
    else:
        outpath = os.path.join(os.path.dirname(filepath), 'gurobi_stats.png')
        
    data = parse_log(filepath)
    plot_data(data, outpath)
