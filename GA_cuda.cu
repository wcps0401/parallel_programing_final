#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <cuda.h>
#include <curand.h>
#include <curand_kernel.h>

#define STR_LENGTH 60
#define CHILD_NUM_PER_GENERATION 20000


/*please note that this program treat type char as signed, different platform may produce different result */
typedef struct offspring{
	char str[STR_LENGTH];
	int score;
}offspring;
offspring pa[CHILD_NUM_PER_GENERATION];
offspring ch[CHILD_NUM_PER_GENERATION];
offspring *tmp,*out;

const int mutation_rate = 100;/*produces mutation with a probability of 1/100*/

void string_random_create(offspring* input){
	int i;
	for(i=0;i<STR_LENGTH;i++){
		input->str[i] = (rand()%95 + 32);/*choose character from 32 to 126*/
	}
	return;
}

__host__ __device__ void fitness(offspring* input){/*this is the fitness function that will score the child */
	int i;
	char target[STR_LENGTH+1] = "Hello World!Hello World!Hello World!Hello World!Hello World!";
	input->score = 0;
	for(i=0;i<STR_LENGTH;i++){
		input->score -= abs(input->str[i] - target[i]);
	}

	return;
}


__device__ void mutation(offspring* input,int data){
	int i;
	for(i=0;i<STR_LENGTH;i++){
		if(data%mutation_rate == 0){
			/*mutate*/
			input->str[i] = data%95 + 32;
		}
		/*don't mutate*/
	}
}


int print_best_result(int generation, offspring* child){
	int result = -1;
	int i;
	int score = -1000000;/*setting to -1000000 as it is an impossible score to reach */
	for(i=0;i<CHILD_NUM_PER_GENERATION;i++){
		if(child[i].score > score){
			score = child[i].score;
			result = i;
		}
	}
	printf("The best score in %d turn is %d : str= ", generation, score);
	for(i=0;i<STR_LENGTH;i++){
		printf("%c",child[result].str[i]);
	}
	printf("\n");
	return score;
}

__global__ void setup_kernel ( curandState * state, unsigned long seed)
{
    int id = blockIdx.x * blockDim.x + threadIdx.x;
    curand_init ( seed, id, 0, &state[id] );
}


__device__ char * my_strcpy(char *dest, const char *src){
  int i = 0;
  do {
    dest[i] = src[i];}
  while (src[i++] != 0);
  return dest;
}

__global__ void kernel4(curandState* State,offspring *famo,offspring *output)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int i;
	int j;
	int parent1 = -1;
	int parent2 = -1;
	int parent3 = -1;
	int parent4 = -1;
	int better1;
	int better2;
	
    parent1=fabsf(curand(&State[idx*4])%CHILD_NUM_PER_GENERATION);
    parent2=fabsf(curand(&State[idx*4+1])%CHILD_NUM_PER_GENERATION);
    parent3=fabsf(curand(&State[idx*4+2])%CHILD_NUM_PER_GENERATION);
    parent4=fabsf(curand(&State[idx*4+3])%CHILD_NUM_PER_GENERATION);

    if(famo[parent1].score > famo[parent2].score){
		better1 = parent1;
	}
	else{
		better1 = parent2;
	}
	if(famo[parent3].score > famo[parent4].score){
		better2 = parent3;
	}
	else{
		better2 = parent4;
	}
	
    i = fabsf(curand(&State[idx*2])%(STR_LENGTH+1));
    for(j=0;j<i;j++){
		output[idx*4].str[j] = famo[better1].str[j];
		output[idx*4+1].str[j] = famo[better2].str[j];
	}
	for(j=i;j<STR_LENGTH;j++){
		output[idx*4].str[j] = famo[better2].str[j];
		output[idx*4+1].str[j] = famo[better1].str[j];
	}
	i = fabsf(curand(&State[idx*2+1])%(STR_LENGTH+1));
	for(j=0;j<i;j++){
		output[idx*4+2].str[j] = famo[better1].str[j];
		output[idx*4+3].str[j] = famo[better2].str[j];
	}
	for(j=i;j<STR_LENGTH;j++){
		output[idx*4+2].str[j] = famo[better2].str[j];
		output[idx*4+3].str[j] = famo[better1].str[j];
	}
    mutation(&output[idx*4],parent1);
    mutation(&output[idx*4+1],parent2);
    mutation(&output[idx*4+2],parent3);
    mutation(&output[idx*4+3],parent4);
    fitness(&output[idx*4]);
    fitness(&output[idx*4+1]);
    fitness(&output[idx*4+2]);
    fitness(&output[idx*4+3]);
}

int main(){
	srand(time(NULL));
	int i;
	printf("See how the string evolves into \"Hello World!Hello World!Hello World!Hello World!Hello World!\"\n");

	memset(&pa, 0, CHILD_NUM_PER_GENERATION*sizeof(offspring));
	memset(&ch, 0, CHILD_NUM_PER_GENERATION*sizeof(offspring));
	for(i=0;i<CHILD_NUM_PER_GENERATION;i++){
		string_random_create(&pa[i]);/*these will be the first parents*/
		fitness(&pa[i]);
	}
	print_best_result(0,pa);

	////cuda
    curandState* devStates;
    cudaMalloc ( &devStates, 25000*sizeof( curandState ) );
	cudaMalloc((void**)&tmp, CHILD_NUM_PER_GENERATION*sizeof(offspring));
	cudaMalloc((void**)&out, CHILD_NUM_PER_GENERATION*sizeof(offspring));

    cudaMemcpy(tmp, pa, CHILD_NUM_PER_GENERATION*sizeof(offspring), cudaMemcpyHostToDevice);
	for(i=1;i<10001;i++){
		setup_kernel <<< 40, 500 >>> (devStates,clock());
		kernel4 << <10, 500 >> >(devStates,tmp,out);
        cudaDeviceSynchronize();
		cudaMemcpy(tmp, out, CHILD_NUM_PER_GENERATION*sizeof(offspring), cudaMemcpyDeviceToDevice);
		//cudaMemcpy(ch, out, CHILD_NUM_PER_GENERATION*sizeof(offspring), cudaMemcpyDeviceToHost);
        //print_best_result(i,ch);
	}
	cudaMemcpy(ch, out, CHILD_NUM_PER_GENERATION*sizeof(offspring), cudaMemcpyDeviceToHost);
	cudaFree(&tmp);
	cudaFree(&out);
	////////////////////////////
	print_best_result(10000,ch);
	return 0;
}
