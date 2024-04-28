export GOMP_CPU_AFFINITY=0-19
export OMP_NUM_THREADS=20

CUR_DIR="$(dirname "$(readlink -f "$0")")"

### TestSNAP
cd ${CUR_DIR}/TestSNAP/perf;
${CUR_DIR}/TestSNAP/perf/run.sh;
cd ${CUR_DIR};


### 3 versions of P2I
cd ${CUR_DIR}/daphne-benchmark/src/OpenMP-offload/points2image/heterogeneousGraph_70iters_timing/perf
${CUR_DIR}/daphne-benchmark/src/OpenMP-offload/points2image/heterogeneousGraph_70iters_timing/perf/run.sh
cd ${CUR_DIR}

cd ${CUR_DIR}/daphne-benchmark/src/OpenMP-offload/points2image/default_timing/perf
${CUR_DIR}/daphne-benchmark/src/OpenMP-offload/points2image/default_timing//perf/run.sh
cd ${CUR_DIR}

cd ${CUR_DIR}/daphne-benchmark/src/Cuda/points2image/perf
${CUR_DIR}/daphne-benchmark/src/Cuda/points2image/perf/run.sh
cd ${CUR_DIR}

### GridMINI
cd ${CUR_DIR}/GridMini/perf;
${CUR_DIR}/GridMini/perf/run.sh;
cd ${CUR_DIR};

### NBODY
cd ${CUR_DIR}/nbody/perf/;
${CUR_DIR}/nbody/perf/run.sh;
cd ${CUR_DIR};

### NEUTRAL
cd ${CUR_DIR}/arch/neutral/perf/;
${CUR_DIR}/arch/neutral/perf/run.sh;
cd ${CUR_DIR};

### HEAT
cd ${CUR_DIR}/heat/perf/;
${CUR_DIR}/heat/perf/run.sh;
cd ${CUR_DIR};

