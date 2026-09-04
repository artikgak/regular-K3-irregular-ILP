import re
import sys
import numpy as np

def parse_log(filepath):
    data = []
    # match lines with Explored, Unexplored ... Time
    # Example:  2762632 183412    0.00000   83  121          -    0.00000      -   283 11962s
    # Example:  2762664 183408 infeasible   84               -    0.00000      -   283 11968s
    pattern = re.compile(r'^\s*(\d+)\s+(\d+)\s+.*?\s+(\d+)s\s*$')
    
    with open(filepath, 'r') as f:
        for line in f:
            match = pattern.match(line)
            if match:
                expl = int(match.group(1))
                unexpl = int(match.group(2))
                time_s = int(match.group(3))
                if not data or data[-1][2] != time_s: # keep only one per second if multiple
                    data.append((expl, unexpl, time_s))
    return data

def analyze(data):
    if len(data) < 2:
        print("Not enough data to analyze.")
        return

    # Extract columns
    expl = np.array([d[0] for d in data])
    unexpl = np.array([d[1] for d in data])
    times = np.array([d[2] for d in data])
    
    # Calculate velocities (nodes per second)
    dt = np.diff(times)
    # avoid division by zero
    valid = dt > 0
    if not np.any(valid):
        print("No valid time differences.")
        return
        
    dt = dt[valid]
    d_expl = np.diff(expl)[valid]
    d_unexpl = np.diff(unexpl)[valid]
    
    v_expl = d_expl / dt
    v_unexpl = d_unexpl / dt
    
    t_mid = times[:-1][valid] + dt / 2.0
    
    # Calculate accelerations (nodes per second^2)
    if len(t_mid) > 1:
        dt_v = np.diff(t_mid)
        valid_v = dt_v > 0
        if np.any(valid_v):
            dt_v = dt_v[valid_v]
            a_expl = np.diff(v_expl)[valid_v] / dt_v
            a_unexpl = np.diff(v_unexpl)[valid_v] / dt_v
        else:
            a_expl = np.zeros(0)
            a_unexpl = np.zeros(0)
    else:
        a_expl = np.zeros(0)
        a_unexpl = np.zeros(0)
        
    print(f"Total time logged: {times[-1] - times[0]} seconds ({times[0]}s to {times[-1]}s)")
    print(f"Current Explored: {expl[-1]}, Current Unexplored: {unexpl[-1]}")
    
    # Recent stats (last 10% of time or at least last 10 points)
    recent_idx = max(0, len(v_expl) - max(10, int(len(v_expl)*0.1)))
    recent_v_expl = np.mean(v_expl[recent_idx:])
    recent_v_unexpl = np.mean(v_unexpl[recent_idx:])
    
    print("\n--- Recent Speed (last few samples) ---")
    print(f"Explored nodes speed: {recent_v_expl:.2f} nodes/sec")
    print(f"Unexplored nodes speed: {recent_v_unexpl:.2f} nodes/sec")
    
    if len(a_expl) > 0:
        recent_a_idx = max(0, len(a_expl) - max(10, int(len(a_expl)*0.1)))
        recent_a_expl = np.mean(a_expl[recent_a_idx:])
        recent_a_unexpl = np.mean(a_unexpl[recent_a_idx:])
        print("\n--- Recent Acceleration ---")
        print(f"Explored nodes accel: {recent_a_expl:.4f} nodes/sec^2")
        print(f"Unexplored nodes accel: {recent_a_unexpl:.4f} nodes/sec^2")
    else:
        recent_a_unexpl = 0.0
        
    print("\n--- Overall Speed ---")
    overall_v_expl = (expl[-1] - expl[0]) / (times[-1] - times[0])
    overall_v_unexpl = (unexpl[-1] - unexpl[0]) / (times[-1] - times[0])
    print(f"Explored nodes overall speed: {overall_v_expl:.2f} nodes/sec")
    print(f"Unexplored nodes overall speed: {overall_v_unexpl:.2f} nodes/sec")
    
    # Estimation
    print("\n--- Estimation ---")
    if recent_v_unexpl >= 0:
        print("Warning: Unexplored nodes are currently INCREASING or STAGNANT. Cannot estimate completion time linearly based on recent data.")
    else:
        time_left_recent = unexpl[-1] / abs(recent_v_unexpl)
        print(f"Based on RECENT speed, estimated time left: {time_left_recent:.2f} seconds ({time_left_recent/3600:.2f} hours)")
        
    if overall_v_unexpl >= 0:
        print("Warning: Overall, Unexplored nodes are INCREASING or STAGNANT. Cannot estimate completion time linearly.")
    else:
        time_left_overall = unexpl[-1] / abs(overall_v_unexpl)
        print(f"Based on OVERALL speed, estimated time left: {time_left_overall:.2f} seconds ({time_left_overall/3600:.2f} hours)")
        
    # Fit a quadratic or linear model to unexplored nodes over the whole trace to project to 0
    # Let's do linear regression on the last 20% of data
    idx = int(len(times) * 0.8)
    if idx < len(times) - 2:
        x = times[idx:]
        y = unexpl[idx:]
        coeffs = np.polyfit(x, y, 1) # y = mx + c
        m, c = coeffs
        if m < 0:
            root = -c / m
            time_left_fit = root - times[-1]
            print(f"Based on LINEAR FIT of last 20% data, estimated time left: {time_left_fit:.2f} seconds ({time_left_fit/3600:.2f} hours)")
        else:
            print("Based on LINEAR FIT, unexplored nodes are not decreasing.")

if __name__ == '__main__':
    filepath = sys.argv[1]
    data = parse_log(filepath)
    analyze(data)
