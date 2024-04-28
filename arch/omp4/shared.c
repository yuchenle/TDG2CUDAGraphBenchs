#include "../shared.h"
#include <stdio.h>
#include <omp.h>

void initialise_devices(int rank) {
  int ndevices = omp_get_num_devices();
  printf("omp4:initialise_devices: %d devices found\n", ndevices);
  // Assume uniform distribution of devices on nodes
  omp_set_default_device(rank % ndevices);
}
