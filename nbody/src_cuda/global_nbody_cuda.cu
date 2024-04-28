#include "kernel.h"
#include "cudaErr.h"

void Particle_array_calculate_forces(
	Particle* this_particle_array, 
	Particle *output_array, 
	int number_of_particles, 
	float time_interval, 
	int timestep) 
{
    int THREADS_PER_BLOCK = 128;
	int num_blocks = 1;

	if (number_of_particles % THREADS_PER_BLOCK) {
		num_blocks = number_of_particles / THREADS_PER_BLOCK + 1;
	} else {
		num_blocks = number_of_particles / THREADS_PER_BLOCK;
	}
	calculate_force_func_cuda<<<num_blocks, THREADS_PER_BLOCK>>> (number_of_particles, time_interval, number_of_particles, this_particle_array, &output_array[0], 0, number_of_particles-1);
}
