#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main()
{
	int i,n,j,k,s; //k denotes the no of threads which will be used at sth stage
	printf("Enter the size of the array: ");
	scanf("%d",&n);
	int arr[n+1];
	printf("Enter the elements: ");
	for(i=1;i<n+1;i++)
	{
		scanf("%d",&arr[i]);
	}

	k=n;
	s=1;
	while(k!=1)
	{
		s*=2;
		for(j=1;j<=n;j=j+s)
		{
			if(j!=n) arr[j]=arr[j]+arr[j+(s/2)];
			printf("%d ",arr[j]);
		}
		printf("\n");
		k=(k+1)/2;
		//printf("k=%d\n",k);
	}

}