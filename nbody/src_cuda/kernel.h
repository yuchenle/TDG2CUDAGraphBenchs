#include"nbody.h"

__global__ void calculate_force_func_cuda(
	int size, 
	float time_interval, 
	int number_of_particles, 
	Particle* d_particles, 
	Particle *output, 
	int first_local, 
	int last_local );
