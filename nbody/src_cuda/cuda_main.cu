/* nbody.c */

#include "nbody.h"
#include "cudaErr.h"
#include "timer.h"
extern void Particle_array_calculate_forces(
	Particle* this_particle_array, 
	Particle *output_array, 
	int number_of_particles, 
	float time_interval , 
	int timestep);

double wall_time ( )
{
	struct timespec ts;
	double res;
	
	clock_gettime(CLOCK_MONOTONIC,&ts);
	
	res = (double) (ts.tv_sec)  + (double) ts.tv_nsec * 1.0e-9;
	
	return res;
}

int main (int argc, char** argv)
{ 
	if (argc != 4) {
		fprintf(stderr,
				"usage: %s <input file> <particle number> <iteration number>\n",
				argv[0]);
		exit(-1);
	}

	Particle* particle_array = (Particle*) NULL;
	Particle* particle_array2 = (Particle*) NULL;
	int timestep;

	FILE *input_data = fopen(argv[1], "r");
	Particle_input_arguments(input_data);
	
	number_of_timesteps = atoi(argv[3]);
	printf("executing the program %d iterations \n", number_of_timesteps);

	particle_array = Particle_array_construct(number_of_particles);
	particle_array2 = Particle_array_construct(number_of_particles);

	Particle_array_initialize(particle_array, number_of_particles);

	FILE * fileptr = fopen("nbody_out.xyz", "w");
	Particle_array_output_xyz(fileptr, particle_array, number_of_particles);
	
	if (number_of_particles <= 1) {
		printf("Returning\n");		
		return 0;
	}
	
	Particle_array_initialize(particle_array, number_of_particles);

    double makespan = 0;

	for (timestep = 1; timestep <= number_of_timesteps; timestep++) {
		// if ((timestep % timesteps_between_outputs) == 0 ) fprintf(stderr, "Starting timestep #%d.\n", timestep);
		START_TIMER;

		Particle_array_calculate_forces(particle_array, particle_array2, number_of_particles, time_interval , timestep);
	    gpuErrchk (cudaDeviceSynchronize());

		END_TIMER;
		makespan += TIMER;
		Particle * tmp = particle_array;
		particle_array = particle_array2;
		particle_array2 = tmp;
	}

    printf("%g ms passed\n", makespan);

	if ((number_of_timesteps % timesteps_between_outputs) != 0) {
		Particle_array_output_xyz(fileptr, particle_array, number_of_particles);
	}
	
	Particle_array_output_xyz(fileptr, particle_array, number_of_particles);

	particle_array = Particle_array_destruct(particle_array, number_of_particles);
	
	if (fclose(fileptr) != 0) {
		fprintf(stderr, "ERROR: can't close the output file.\n");
		exit(program_failure_code);
	}
	
	return program_success_code;
}
