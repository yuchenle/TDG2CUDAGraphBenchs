#include "shared_data.h"
#include "shared.h"
#include <stdio.h>

// Initialises the shared_data variables
void initialise_shared_data_2d(const int local_nx, const int local_ny,
                               const int pad, const double mesh_width,
                               const double mesh_height,
                               const char* problem_def_filename,
                               const double* edgex, const double* edgey,
                               SharedData* shared_data) {
  const int ndims = 2;

  // Shared shared_data
  allocate_data(&shared_data->density, local_nx * local_ny);
  allocate_data(&shared_data->energy, local_nx * local_ny);

  // Currently flattening the capacity by sharing some of the shared_data
  // containers
  // between the different solves for different applications. This might not
  // be maintainable and/or desirable but seems like a reasonable optimisation
  // for now...
  allocate_data(&shared_data->density_old, local_nx * local_ny);
  shared_data->Ap = shared_data->density_old;

  allocate_data(&shared_data->s_x, (local_nx + 1) * (local_ny + 1));
  shared_data->Qxx = shared_data->s_x;

  allocate_data(&shared_data->s_y, (local_nx + 1) * (local_ny + 1));
  shared_data->Qyy = shared_data->s_y;

  allocate_data(&shared_data->r, local_nx * local_ny);
  shared_data->pressure = shared_data->r;

  allocate_data(&shared_data->temperature, (local_nx + 1) * (local_ny + 1));
  shared_data->u = shared_data->temperature;

  allocate_data(&shared_data->p, (local_nx + 1) * (local_ny + 1));
  shared_data->v = shared_data->p;

  allocate_data(&shared_data->reduce_array0, (local_nx + 1) * (local_ny + 1));
  allocate_data(&shared_data->reduce_array1, (local_nx + 1) * (local_ny + 1));

  set_problem_2d(local_nx, local_ny, pad, mesh_width, mesh_height, edgex, edgey,
                 ndims, problem_def_filename, shared_data->density,
                 shared_data->energy, shared_data->temperature);
}

// Initialises the shared_data variables
void initialise_shared_data_3d(
    const int local_nx, const int local_ny, const int local_nz, const int pad,
    const double mesh_width, const double mesh_height, const double mesh_depth,
    const char* problem_def_filename, const double* edgex, const double* edgey,
    const double* edgez, SharedData* shared_data) {

  const int ndims = 3;

  // Shared shared_data
  allocate_data(&shared_data->density, local_nx * local_ny * local_nz);
  allocate_data(&shared_data->energy, local_nx * local_ny * local_nz);

  // Currently flattening the capacity by sharing some of the shared_data
  // containers between the different solves for different applications.
  // This might not be maintainable and/or desirable
  allocate_data(&shared_data->density_old, local_nx * local_ny * local_nz);
  shared_data->Ap = shared_data->density_old;

  allocate_data(&shared_data->s_x,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));
  shared_data->Qxx = shared_data->s_x;

  allocate_data(&shared_data->s_y,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));
  shared_data->Qyy = shared_data->s_y;

  allocate_data(&shared_data->s_z,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));
  shared_data->Qzz = shared_data->s_z;

  allocate_data(&shared_data->r, local_nx * local_ny * local_nz);
  shared_data->pressure = shared_data->r;

  allocate_data(&shared_data->temperature,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));
  shared_data->u = shared_data->temperature;

  allocate_data(&shared_data->pressure,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));
  shared_data->v = shared_data->pressure;

  allocate_data(&shared_data->reduce_array0,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));
  allocate_data(&shared_data->reduce_array1,
                (local_nx + 1) * (local_ny + 1) * (local_nz + 1));

  set_problem_3d(local_nx, local_ny, local_nz, pad, mesh_width, mesh_height,
                 mesh_depth, edgex, edgey, edgez, ndims, problem_def_filename,
                 shared_data->density, shared_data->energy,
                 shared_data->temperature);
}

// Deallocate all of the shared_data memory
void finalise_shared_data(SharedData* shared_data) {
  deallocate_data(shared_data->density);
  deallocate_data(shared_data->energy);

  // Only free one of the paired shared_datas
  deallocate_data(shared_data->Ap);
  deallocate_data(shared_data->s_x);
  deallocate_data(shared_data->s_y);
  deallocate_data(shared_data->r);
  deallocate_data(shared_data->temperature);
  deallocate_data(shared_data->p);
}