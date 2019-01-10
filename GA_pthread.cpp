#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

#define STR_LENGTH 60
#define CHILD_NUM_PER_GENERATION 20000
#define PTHREAD_NUMBER 4


//pthread
long thread_count;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;





/*please note that this program treat type char as signed, different platform may produce different result */ 
typedef struct{
	char str[STR_LENGTH];
	int score;
}offspring;
offspring global_child[2][CHILD_NUM_PER_GENERATION];




char* target = "Hello World!Hello World!Hello World!Hello World!Hello World!";
const int mutation_rate = 100;/*produces mutation with a probability of 1/100*/


void fitness(offspring* input){/*this is the fitness function that will score the child */
	int i;
	input->score = 0;
	for(i=0;i<STR_LENGTH;i++){
		input->score -= abs(input->str[i] - target[i]);
	}

	return;
}


void string_random_create(offspring* input){
	int i;
	for(i=0;i<STR_LENGTH;i++){
		input->str[i] = (rand()%95 + 32);/*choose character from 32 to 126*/
	}
	return;
}

void mutation(offspring* input,unsigned int* seed){
	int i;
	//unsigned int seed = time(NULL);
	for(i=0;i<STR_LENGTH;i++){
		if(rand_r(seed)%mutation_rate == 0){
			/*mutate*/
			input->str[i] = rand_r(seed)%95 + 32;
		}
		/*don't mutate*/
	}
}


void* mate(void* data){
	//int pool[1000];/* We'll choose in this pool to see whether the parent has been chosen yet */
	//memset(pool, 0, sizeof(pool));
	//int not_chosen = 1000;
	
	//pthread
	long long int num_start = (long long int)data;
	unsigned int seed = time(NULL) + (unsigned int)pthread_self();
	
	
	int i;
	int j;
	int num;
	int parent1 = -1;
	int parent2 = -1;
	int parent3 = -1;
	int parent4 = -1;
	int better1;
	int better2;
	for (num = num_start; num< num_start + CHILD_NUM_PER_GENERATION/PTHREAD_NUMBER;num+=4){
		/*choose parent 1 with the number of not chosen numbers */
		//printf("%d\n",child_num);
		parent1 = rand_r(&seed)%CHILD_NUM_PER_GENERATION + 1;
		/*choose parent 2 with the number of not chosen numbers*/
		parent2 = rand_r(&seed)%CHILD_NUM_PER_GENERATION + 1;
		while(parent2 == parent1){
			parent2 = rand_r(&seed)%CHILD_NUM_PER_GENERATION + 1;
		}
		/*choose parent 3 with the number of not chosen numbers */
		/*choose parent 4 with the number of not chosen numbers*/
		parent3 = rand_r(&seed)%CHILD_NUM_PER_GENERATION + 1;
		parent4 = rand_r(&seed)%CHILD_NUM_PER_GENERATION + 1;
		while(parent4 == parent3){
			parent4 = rand_r(&seed)%CHILD_NUM_PER_GENERATION + 1;
		}
		/*now that we have selected 4 parents, we are going to choose the better one out of parent1 and parent2, and choose one out of parent3 and parent4*/
		if(global_child[0][parent1].score > global_child[0][parent2].score){
			better1 = parent1;
		}
		else{
			better1 = parent2;
		}
		if(global_child[0][parent3].score > global_child[0][parent4].score){
			better2 = parent3;
		}
		else{
			better2 = parent4;
		}
		/*now we are going to do crossover with the selected parents */
		i = rand_r(&seed)%(STR_LENGTH+1);/*here i will be used as a separator of how we are going to do crossover with the parents */
		int i2 = i;
		//printf("%d  %d |  ",num,i);
		for(j=0;j<i;j++){
			global_child[1][num].str[j] = global_child[0][better1].str[j];
			global_child[1][num+1].str[j] = global_child[0][better2].str[j];
		}
		for(j=i;j<STR_LENGTH;j++){
			global_child[1][num].str[j] = global_child[0][better2].str[j];
			global_child[1][num+1].str[j] = global_child[0][better1].str[j];
		}
		
		i = rand_r(&seed)%(STR_LENGTH+1);
    //printf("%d | %d  %d\n",num,i2,i);
		for(j=0;j<i;j++){
			global_child[1][num+2].str[j] = global_child[0][better1].str[j];
			global_child[1][num+3].str[j] = global_child[0][better2].str[j];
		}
		for(j=i;j<STR_LENGTH;j++){
			global_child[1][num+2].str[j] = global_child[0][better1].str[j];
			global_child[1][num+3].str[j] = global_child[0][better2].str[j];
		}
		mutation(&global_child[1][num],&seed);
		mutation(&global_child[1][num+1],&seed);
		mutation(&global_child[1][num+2],&seed);
		mutation(&global_child[1][num+3],&seed);
		fitness(&global_child[1][num]);
		fitness(&global_child[1][num+1]);
		fitness(&global_child[1][num+2]);
		fitness(&global_child[1][num+3]);
		//num should be lock
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

int print_best_result_end(int generation, offspring* child){
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

int main(){
	//PTHREAD
	long thread;
	pthread_t* thread_handles;
	thread_count = PTHREAD_NUMBER;
	int children_per_CPU =CHILD_NUM_PER_GENERATION /thread_count ;
	thread_handles = (pthread_t*) malloc (thread_count*sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);
	/////////////////////////
	
	srand(time(NULL));
	int i;
	int j;
	int result;
	printf("See how the string evolves into \"Hello World!Hello World!Hello World!\"\n");
	/*for(i=0;i<2;i++){
		for(j=0;j<CHILD_NUM_PER_GENERATION;j++){
			memset(&global_child[i][j], 0, sizeof(offspring));
		}
	}*/
	memset(&global_child, 0, 2*CHILD_NUM_PER_GENERATION*sizeof(offspring));
	for(i=0;i<CHILD_NUM_PER_GENERATION;i++){
		string_random_create(&global_child[0][i]);/*these will be the first parents*/
		fitness(&global_child[0][i]);
	}
	print_best_result(0,global_child[0]);
	for(i=1;i<10000;i++){/*generation*/
		int start_num;
		for (thread = 0; thread < thread_count - 1; thread++)
		{
			start_num = (children_per_CPU*thread);
			pthread_create(&thread_handles[thread], NULL,mate, (void*)start_num);
		}
		start_num = (children_per_CPU*thread);
		mate((void*)start_num);
		for (thread = 0; thread < thread_count-1; thread++)
			pthread_join(thread_handles[thread], NULL);

		
		//mate(global_child[0], global_child[1]);
		result = print_best_result(i,global_child[1]);/*print the best result of the ith generation */
		if(result == 0){
			//break;
		}
		for(j=0;j<CHILD_NUM_PER_GENERATION;j++){
			global_child[0][j] = global_child[1][j];
		}
	}
   result = print_best_result_end(i,global_child[0]);
	pthread_mutex_destroy(&mutex);
	free(thread_handles);
	return 0;
}
