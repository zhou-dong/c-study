/* C program for merge sort */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


/* Function to merge the two haves arr[l..m] and arr[m+1..r] of array arr[] */
void merge(int arr[], int l, int m, int r, int shmid)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
    
    /* create temp arrays */
    int L[n1], R[n2];
    
    /* Copy data to temp arrays L[] and R[] */
    for(i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for(j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
    
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    
    /* Copy the remaining elements of L[], if there are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }
    
    /* Copy the remaining elements of R[], if there are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

/* l is for left index and r is right index of the sub-array
 of arr to be sorted */
void mergeSort(int arr[], int l, int r, int shmid)
{
    if (l < r)
    {
        int m = l+(r-l)/2; //Same as (l+r)/2, but avoids overflow for large l and h
        mergeSort(arr, l, m, shmid);
        mergeSort(arr, m+1, r, shmid);
        merge(arr, l, m, r, shmid);
    }
}


/* UITLITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size)
{
    int i;
    for (i=0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}

 #define BUFFER_SIZE 2048

/* Driver program to test above functions */
int main()
{
    int arr[] = {12, 11, 13, 5, 6, 7};
    int arr_size = sizeof(arr)/sizeof(arr[0]);
    
    printf("Given array is \n");
    printArray(arr, arr_size);
    
    /*Implment with shared memory*/
    
    //创建共享内存
    int shmid = shmget((key_t)1234, BUFFER_SIZE, 0666|IPC_CREAT);
    if(shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    
    void *shm = NULL; //分配的共享内存的原始首地址
    shm = shmat(shmid, 0, 0);
    if(shm == (void*)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid;
    if((pid=fork())<0)
    {
        perror("fork");
        exit(1);
    }
    
    if (pid ==0) // child process
    {
        printf("child process: [%d]\n", pid) ;
    }
    else //Parent process
    {
        printf("parent process: [%d]\n", pid) ;
    }
    
    printf("%d\n", shmid);
    
    mergeSort(arr, 0, arr_size - 1, shmid);
    
    printf("\nSorted array is \n");
    printArray(arr, arr_size);

    
    return 0;
}
