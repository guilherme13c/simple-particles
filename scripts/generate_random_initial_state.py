import random
import math

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

generate_simulation_state("init_state.txt")
