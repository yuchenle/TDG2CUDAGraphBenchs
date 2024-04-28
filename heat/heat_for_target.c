//
// This file is part of Heat Gauss Seidel  and is licensed under the terms
// contained in the COPYING file. Copyright (C) 2015-2020 Barcelona
// Supercomputing Center (BSC)
//

/*
 * Iterative solver for heat distribution
 */
#include "heat.h"
#include "timer.h"

#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // to measure time without parallel
int Num_B;

double **u;

#ifndef _OPENMP
int omp_get_max_threads() { return 1; }
#endif

void usage(char *s) {
  fprintf(stderr, "Usage: %s <input file> [result file]\n\n", s);
}

double relax_gauss(double *u1, unsigned sizex, unsigned sizey) {
  double sum = 0.0;
  int nbx, bx, nby, by;
  u = (double(*)[sizey])u1;

  nbx = Num_B;
  nby = Num_B;
  bx = sizex / nbx;
  by = sizey / nby;

  for (int ii = 0; ii < nbx; ii++) {
    for (int jj = 0; jj < nby; jj++) {
      int inf_i = 1 + ii * bx;
      int sup_i = ((inf_i + bx) < sizex - 1) ? inf_i + bx : sizex - 1;
      int inf_j = 1 + jj * by;
      int sup_j = ((inf_j + by) < sizey - 1) ? inf_j + by : sizey - 1;

#pragma omp task depend(in                                                     \
                        : u[inf_i - bx][inf_j], u[sup_i][inf_j],               \
                          u[inf_i][inf_j - by], u[inf_i][sup_j])               \
    depend(inout                                                               \
           : u[inf_i][inf_j]) firstprivate(sizex, sizey, u)
      {
        // SMATEO: extra declaration because Intel OpenMP has some problems with
        // VLAs...
        double(*u2)[sizey] = (double(*)[sizey])u;
        for (int i = inf_i; i < sup_i; ++i) {
          for (int j = inf_j; j < sup_j; ++j) {
            double unew;
            unew = 0.25 *
                   (u2[i][j - 1] + u2[i][j + 1] + u2[i - 1][j] + u2[i + 1][j]);
            u2[i][j] = unew;
            // if(unew!=0) printf("Es %d %d \n", i, j);
          }
        }
      }
    }
  }
  return sum;
}

int main(int argc, char *argv[]) {
  FILE *infile, *resfile;

  // algorithmic parameters
  algoparam_t param;
  int np;

  // double residual=0.0;
  unsigned iter = 0;

  // check arguments
  if (argc < 4) {
    printf("usage: <input file> <block number> <iteration number>\n");
    return 1;
  }

  Num_B = atoi(argv[2]);
  int num_iter = atoi(argv[3]);

  if (num_iter < 0) {
    printf("num_iter must be positif\n");
    exit(-1);
  }

  // check input file
  if (!(infile = fopen(argv[1], "r"))) {
    fprintf(stderr, "\nError: Cannot open \"%s\" for reading.\n\n", argv[1]);

    usage(argv[0]);
    return 1;
  }

  // check input
  if (!read_input(infile, &param)) {
    fprintf(stderr, "\nError: Error parsing input file.\n\n");
    usage(argv[0]);
    return 1;
  }

#ifdef VERBOSE
  // check result file
  unsigned size = 128;
  char resfilename[size];
  resfilename[0] = '\0';
  strcat(resfilename, argv[0]);
  strcat(resfilename, ".ppm");

  if (!(resfile = fopen(resfilename, "w"))) {
    fprintf(stderr, "\nError: Cannot open \"%s\" for writing.\n\n",
            resfilename);
    usage(argv[0]);
    return 1;
  }
#endif
  // print_params(&param);

  assert((param.algorithm == 1) &&
         "Only Gauss-seidel is currently implemented\n");

  if (!initialize(&param)) {
    fprintf(stderr, "Error in Solver initialization.\n\n");
    usage(argv[0]);
    return 1;
  }

  if (Num_B > param.resolution) {
    printf("matrix size = %d, NB = %d, NB must be smaller or equal to matrix "
           "size\n",
           param.resolution, Num_B);
    exit(-1);
  }


  printf("This implementation is mimicking the naive implementation of two nested loop with nx and ny\n");
  printf("Hence, setting the number of blocks to 1!\n");
  int nbx = 1;
  int nby = 1;
  // full size (param.resolution are only the inner points)
  np = param.resolution + 2;
  // np = RESOLUTION + 2;

  double *u_tmp = (double *)malloc(sizeof(double) * np * np);
  memcpy(u_tmp, param.u, sizeof(double) * np * np);

  #pragma omp target enter data map(to: param.u[0:np*np])

  // double makespan[50] = {0};
  double makespan = 0;
#pragma omp parallel
#pragma omp single
  for (int i = 0; i < num_iter; i++) {
    double sum = 0.0;
    int bx, by;
    int sizex = np;
    int sizey = np;
    u = (double(*)[sizey])param.u;
    // nbx = Num_B;
    // nby = Num_B;
    bx = sizex / nbx;
    by = sizey / nby;

    START_TIMER;
    {
      for (unsigned iter = 0; iter < param.maxiter; ++iter) {
        // number of diagonals
        for (int diag = 0; diag < nbx; diag++) {
// #pragma omp target nowait
          for (int I = 0; I <= diag; I++) {
            int ii = I;
            int jj = diag - I;

            int inf_i = 1 + ii * bx;
            int sup_i = ((inf_i + bx) < sizex - 1) ? inf_i + bx : sizex - 1;
            int inf_j = 1 + jj * by;
            int sup_j = ((inf_j + by) < sizey - 1) ? inf_j + by : sizey - 1;

            // printf("inf_i = %d, sup_i %d inf_j %d, sup_j %d\n", inf_i, sup_i,
            // inf_j, sup_j);
            #pragma omp target teams distribute parallel for collapse(2)
            for (int i = inf_i; i < sup_i; i++) {
              for (int j = inf_j; j < sup_j; j++) {
                // printf("i %d\n", i);
                // printf("j %d\n", j);
                // printf("param.u %p\n", (void *)param.u);
                // printf("param.u[%d][%d] = %f\n", i, j, param.u[i*sizex+j]);
                double unew;
                unew = 0.25 * (param.u[i * sizex + j - 1] +
                               param.u[i * sizex + j + 1] +
                               param.u[(i - 1) * sizex + j] +
                               param.u[(i + 1) * sizex + j]);
                param.u[i * sizex + j] = unew;
              }
            }
          }
        }

        for (int diag = nbx - 1; diag >= 0; diag--) {
// #pragma omp teams distribute parallel for
          for (int I = nbx - 1; I >= nbx - diag; I--) {
            int ii = nby - I;
            int jj = nby - ii;
            ii += nbx - 1 - diag;

            int inf_i = 1 + ii * bx;
            int sup_i = ((inf_i + bx) < sizex - 1) ? inf_i + bx : sizex - 1;
            int inf_j = 1 + jj * by;
            int sup_j = ((inf_j + by) < sizey - 1) ? inf_j + by : sizey - 1;

            // printf("inf_i = %d, sup_i %d inf_j %d, sup_j %d\n", inf_i, sup_i,
            // inf_j, sup_j);
            #pragma omp target teams distribute parallel for collapse(2)
            for (int i = inf_i; i < sup_i; i++) {
              for (int j = inf_j; j < sup_j; j++) {
                double unew;
                unew = 0.25 * (param.u[i * sizex + j - 1] +
                               param.u[i * sizex + j + 1] +
                               param.u[(i - 1) * sizex + j] +
                               param.u[(i + 1) * sizex + j]);
                param.u[i * sizex + j] = unew;
              }
            }
          }
        }
      }
    }
    END_TIMER;
    makespan += TIMER;
#ifdef VERBOSE
    if (i == 1)
#endif
    {
      // memcpy(param.u, u_tmp, sizeof(double) * np * np);
      // #pragma omp target update to(param.u[:np*np])
    }
  } // num_iter loop

  printf("without parallel, we spent %.4f ms\n", makespan);
  #pragma omp target exit data map(from: param.u[:np*np])

#ifdef VERBOSE
  coarsen(param.u, np, np, param.uvis, param.visres + 2, param.visres + 2);
  write_image(resfile, param.uvis, param.visres + 2, param.visres + 2);
#endif
  finalize(&param);
  return 0;
}
