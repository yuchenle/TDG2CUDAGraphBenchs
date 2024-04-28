TDG=output_TDG
BLOCK=output_original_blocking
THREAD=output_original_threading
for i in {1..50};do
  ../original-blocking-llvm-Benchmark_su3.x | grep "average time" >> $BLOCK;
  ../taskgraph-llvm-Benchmark_su3.x | grep "average time" >> $TDG;
  ../original-threading-llvm-Benchmark_su3.x | grep "average time" >> $THREAD;
done;
