#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <omp.h>

#pragma omp requires unified_shared_memory 
using namespace std;
struct vec {
  float v[2];
};


inline  vec mult(vec x, vec y){
  vec out;
  out.v[0]=x.v[0]*y.v[0];
  out.v[1]=x.v[1]*y.v[1];
  printf("x.v[0]=%f, y.v[0]=%f\n",x.v[0],y.v[0]); 
  return out;
}
int main(int argc, char* argv[]){

	int N=10;
        float x=1.0;
	float y=2.0;
	vec in1,in2;
	in1.v[0]=x;
	in1.v[1]=x;
	in2.v[0]=y;
	in2.v[1]=y;
const int device_id = (omp_get_num_devices() > 0) ? omp_get_default_device() : omp_get_initial_device(); 
	vec *out=(vec *)omp_target_alloc(N*sizeof(vec), device_id);
printf("CPU calculation...\n");
	vec expected=mult(in1,in2);

printf("GPU offloading...\n");



#pragma omp target teams distribute parallel for //map(to:in1,in2) map(from:out[0:N])
	for(int n=0;n<N;n++) {
		out[n]=mult(in1,in2);
		printf("Got: %f %f\n",out[n].v[0],out[n].v[1]);
	}
	printf("Expected: %f %f\n",expected.v[0],expected.v[1]);

return 0;

}

