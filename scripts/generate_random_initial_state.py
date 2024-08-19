import random
import math
import sys

def generate_simulation_state(filename, num_particles=3, time_steps=2000, dt=0.01, width=1200, height=800):
    with open(filename, 'w') as file:
        # Write the first line (simulation configuration)
        file.write(f"{num_particles} {time_steps} {dt} {width} {height}\n")
        
        # Generate and write particle data
        for _ in range(num_particles):
            position = [random.uniform(-2, 2) for _ in range(3)]
            velocity = [random.uniform(-3, 3) for _ in range(3)]
            mass = math.floor(random.uniform(1e6, 1e12))
            file.write(f"{position[0]} {position[1]} {position[2]} "
                       f"{velocity[0]} {velocity[1]} {velocity[2]} "
                       f"{mass}\n")

if __name__ == "__main__":
    N = ""
    if len(sys.argv) >= 6: N = sys.argv[1]
    else: N = 3
    
    t = ""
    if len(sys.argv) >= 6: t = sys.argv[2]
    else: t = 100
    
    dt = ""
    if len(sys.argv) >= 6: dt = sys.argv[3]
    else: dt = 0.01
    
    w = ""
    if len(sys.argv) >= 6: w = sys.argv[4]
    else: w = 1200
    
    h = ""
    if len(sys.argv) >= 6: h = sys.argv[5]
    else: h = 800
    
    generate_simulation_state("init_state.txt", int(N), int(t), float(dt), int(w), int(h))
