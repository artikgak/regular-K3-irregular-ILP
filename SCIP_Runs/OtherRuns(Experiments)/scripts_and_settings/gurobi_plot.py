import re
import sys
import matplotlib.pyplot as plt
import argparse

def parse_gurobi_log(log_path):
    times, explored, unexplored, depths, intinfs, it_nodes = [], [], [], [], [], []
    
    with open(log_path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            tokens = line.split()
            if len(tokens) >= 5 and tokens[-1].endswith('s'):
                idx = 0
                if tokens[0] in ['*', 'H']:
                    idx = 1
                
                if idx < len(tokens) and tokens[idx].isdigit() and tokens[idx+1].isdigit():
                    try:
                        time_str = tokens[-1][:-1]
                        if not time_str.isdigit():
                            continue
                        time_val = int(time_str)
                        
                        exp = int(tokens[idx])
                        unexp = int(tokens[idx+1])
                        
                        # depth
                        depth = 0
                        if idx + 3 < len(tokens) and tokens[idx+3].isdigit():
                            depth = int(tokens[idx+3])
                            
                        # intinf
                        intinf = 0
                        if tokens[idx+2] not in ['infeasible', 'cutoff']:
                            if idx + 4 < len(tokens) and tokens[idx+4].isdigit():
                                intinf = int(tokens[idx+4])
                                
                        # it_node
                        it_node = 0.0
                        if tokens[-2] != '-':
                            it_node = float(tokens[-2])
                            
                        times.append(time_val)
                        explored.append(exp)
                        unexplored.append(unexp)
                        depths.append(depth)
                        intinfs.append(intinf)
                        it_nodes.append(it_node)
                    except ValueError:
                        pass
    return times, explored, unexplored, depths, intinfs, it_nodes

def plot_progress(times, explored, unexplored, depths, intinfs, it_nodes, output_path):
    fig, axs = plt.subplots(5, 1, figsize=(12, 20), sharex=True)

    # Plot 1: Explored Nodes
    axs[0].plot(times, explored, color='tab:blue')
    axs[0].set_ylabel('Explored Nodes', color='tab:blue')
    axs[0].grid(True)
    axs[0].set_title('Total Explored Nodes')
    
    # Plot 2: Unexplored Nodes
    axs[1].plot(times, unexplored, color='tab:red')
    axs[1].set_ylabel('Unexplored (Open) Nodes', color='tab:red')
    axs[1].grid(True)
    axs[1].set_title('Open Nodes Queue')

    # Plot 3: Depth
    axs[2].plot(times, depths, color='tab:green')
    axs[2].set_ylabel('Tree Depth')
    axs[2].grid(True)
    axs[2].set_title('Current Node Depth')

    # Plot 4: IntInf
    axs[3].plot(times, intinfs, color='tab:purple')
    axs[3].set_ylabel('Integer Infeasibilities')
    axs[3].grid(True)
    axs[3].set_title('Fractional Variables (IntInf)')

    # Plot 5: It/Node
    axs[4].plot(times, it_nodes, color='tab:orange')
    axs[4].set_xlabel('Time (s)')
    axs[4].set_ylabel('Iterations / Node')
    axs[4].grid(True)
    axs[4].set_title('Simplex Iterations per Node')

    fig.tight_layout()
    
    if output_path:
        plt.savefig(output_path)
        print(f"Plot saved to {output_path}")
    else:
        plt.show()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Parse Gurobi log and plot progress')
    parser.add_argument('log_file', help='Path to the Gurobi log file')
    parser.add_argument('--output', '-o', help='Output image path', default='gurobi_progress.png')
    
    args = parser.parse_args()
    
    t, e, u, d, i, it = parse_gurobi_log(args.log_file)
    if not t:
        print("No progress data found in the log.")
    else:
        print(f"Parsed {len(t)} data points.")
        plot_progress(t, e, u, d, i, it, args.output)
