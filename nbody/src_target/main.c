/* nbody.c */

#include "nbody.h"
#include "timer.h"

#define TARGET_NB 32

extern void calculate_force_func(float, int, Particle *, Particle *, int);

double wall_time() {
  struct timespec ts;
  double res;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  res = (double)(ts.tv_sec) + (double)ts.tv_nsec * 1.0e-9;

  return res;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s <input file> <particle number> <iteration number>\n",
            argv[0]);
    exit(-1);
  }

  Particle *particle_array = (Particle *)NULL;
  Particle *particle_array2 = (Particle *)NULL;
  int timestep;
  int i;

  FILE *input_data = fopen(argv[1], "r");
  Particle_input_arguments(input_data);

  number_of_timesteps = atoi(argv[3]);
  printf("executing the program %d iterations \n", number_of_timesteps);

  particle_array = Particle_array_construct(number_of_particles);
  particle_array2 = Particle_array_construct(number_of_particles);

  Particle_array_initialize(particle_array, number_of_particles);

  FILE *fileptr = fopen("nbody_out.xyz", "w");
  Particle_array_output_xyz(fileptr, particle_array, number_of_particles);

  if (number_of_particles <= 1) {
    printf("Returning\n");
    return 0;
  }

  Particle_array_initialize(particle_array, number_of_particles);

  int BLOCK_SIZE = number_of_particles / TARGET_NB;
  if (BLOCK_SIZE < 1) {
    printf("There are fewer particles (%d) than the target number of blocks (64), "
           "consider bigger input files\n", number_of_particles);
    return -1;
  }

  #pragma omp target enter data map(to: particle_array[0:number_of_particles])
  #pragma omp target enter data map(to: particle_array2[0:number_of_particles])

  double makespan = 0;

#pragma omp parallel
#pragma omp single
  {
    for (timestep = 1; timestep <= number_of_timesteps; timestep++) {

      START_TIMER;

#ifdef TDG
#pragma omp taskgraph target_graph
      {
#endif

#ifdef BLOCKING
      for (int i = 0; i < number_of_particles; i+=BLOCK_SIZE) {
        #pragma omp target nowait
        #pragma omp teams distribute parallel for
        for (int j = i; j < i + BLOCK_SIZE; j++) {
          calculate_force_func(time_interval, number_of_particles,
                               particle_array, &particle_array2[j], j);
        }
      }
#else
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < number_of_particles; i++) {
          calculate_force_func(time_interval, number_of_particles,
                                particle_array, &particle_array2[i], i);
        }
#endif

#ifdef TDG
      }
#else
#pragma omp taskwait
#endif

      END_TIMER;

      makespan += TIMER;
      printf("iteration %d took %f ms\n", timestep, TIMER);
      Particle *tmp = particle_array;
      particle_array = particle_array2;
      particle_array2 = tmp;

      // do we need to update the target pointers?
      // #pragma omp target update(particle_array[0:number_of_particles])
      // #pragma omp target enter data map(to: particle_array2[0:number_of_particles])


    } // for
    printf("%g ms passed\n", makespan);
  } // single,parallel

  #pragma omp target exit data map(from: particle_array[0:number_of_particles])

  Particle_array_output_xyz(fileptr, particle_array, number_of_particles);

  particle_array = Particle_array_destruct(particle_array, number_of_particles);

  if (fclose(fileptr) != 0) {
    fprintf(stderr, "ERROR: can't close the output file.\n");
    exit(program_failure_code);
  }

  return program_success_code;
}
