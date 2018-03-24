#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

void *sum(void *param); /* void *func() for pthread */
void sumOfElements(int arr[], int n); /* array is passed to the function, in turns passed to structure as well as indexes */
pthread_mutex_t arr_mutex;

struct argArr
{
	int a; /* holds index of left */
	int b; /* holds index of right */
	int size; /* size of array */
	int *arr; /* array */
};

int main()
{
	printf("\n> Enter size of array: ");
	int n; scanf("%d", &n);
	printf("\n> Enter elements: ");
	int arr[n]; for(int i = 0; i < n; i++) scanf("%d",&arr[i]);
	sumOfElements(arr,n);
	return 0;
}

void sumOfElements(int arr[], int n)
{
	pthread_attr_t attr; /*set of thread attributes to be passed in pthread_create() */
	pthread_attr_init(&attr); /* int pthread_attr_init(pthread_attr_t *attr); */
	pthread_mutex_init(&arr_mutex, NULL);
	int start=2;
	int levels = floor(log2(n))+1; /* levels calcutation */
	printf("\n");

	for(int k=0; k<levels; k++) /* level wise addition */
	{
		pthread_t tid[n]; /* array of threads */
		pthread_mutex_lock(&arr_mutex); /* thread mutex lock */
		printf("(+) Level %d: ", k+1);
		for(int i = 0; i < n; i += start)
		{
			struct argArr *data;
			data = (struct argArr *)malloc(sizeof(struct argArr));
			
			data->a = i; /* index left */
			data->b = i + start/2; /* adjacent index, right */
			data->size = n; /* size of array */
			data->arr = arr; /* data -> arr = &arr[0] */
			
			/*int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg); */
			if (pthread_create(&tid[i], &attr, sum, data) != 0) exit(1); /* if pthread_create fails */
			sleep(0.5); /* introduces so that thread doesn't mix up */
		}
		pthread_mutex_unlock(&arr_mutex); /* thread mutex unlock */
		for(int i = 0; i < n; i += start) if(pthread_join(tid[i], NULL) != 0) exit(1); /* joins every thread of the given level */
		start *= 2;
		printf("\n");
	}
	printf("\n>Result = %d\n", arr[0]); /* result stored in leftmost index */
	pthread_mutex_destroy(&arr_mutex); /* destroy mutex */
}

void *sum(void *param)
{
	struct argArr *data;
	data = (struct argArr *)param;
	if(data->b >= data->size) *((data->arr)+(data->a))=*((data->arr)+(data->a));
	else *((data->arr)+(data->a))=*((data->arr)+(data->a))+*((data->arr)+(data->b)); /* stores the result in left index of each operation */
	printf("%d ", *((data->arr)+(data->a)));
    free(param);
	pthread_exit(0);
}