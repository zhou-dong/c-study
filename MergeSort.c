/* C program for merge sort */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define ARRAY_SIZE 6

struct shared_use_st
{
    int index  ;
    int arr_size ;
    int arr[ARRAY_SIZE] ;
};

/* Function to merge the two haves arr[l..m] and arr[m+1..r] of array arr[] */
void merge(int l, int m, int r, int shmid, struct shared_use_st *shared)
{
    int *arr = shared->arr;
    
    
    printf("begin index: [%d]\n", shared->index);
    
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
    
    shared->index-- ;
    
    printf("end index: [%d]\n", shared->index);
    
}

/* l is for left index and r is right index of the sub-array
 of arr to be sorted */
void mergeSort(int l, int r, int shmid)
{
    if (l < r)
    {
        
        int m = l+(r-l)/2; //Same as (l+r)/2, but avoids overflow for large l and h
        pid_t pid;
        pid_t pid2;
        
        void *shm1 = NULL;
        void *shm2 = NULL;
        struct shared_use_st *shared1 = NULL;
        struct shared_use_st *shared2 = NULL;
        int index1 ;
        int index2 ;
        if((pid=fork())<0)
        {
            perror("fork1");
            exit(1);
        }
        if (pid==0) // child process
        {
            //将共享内存连接到当前进程的地址空间
            shm1 = shmat(shmid, (void*)0, 0);
            if(shm1== (void*)-1)
            {
                //fprintf(stderr, "shmat failed in begin merge0\n");
                exit(EXIT_FAILURE);
            }
            printf("Memory attached at %X\n", (int)shm1);
            
            shared1 = (struct shared_use_st*)shm1;
            
            printf("child process1: [%d]\n", pid) ;
            index1 = shared1->index;
            
            shared1->index++ ;
            mergeSort(l, m, shmid);
            merge(l, m, r, shmid, shared1);
            while (shared1->index!=index1) {
                sleep(1) ;
            }
            //把共享内存从当前进程中分离
            if(shmdt(shm1) == -1)
            {
                fprintf(stderr, "shmdt failed\n");
                exit(EXIT_FAILURE);
            }
            //删除共享内存
            if(shmctl(shmid, IPC_RMID, 0) == -1)
            {
                //fprintf(stderr, "shmctl(IPC_RMID) failed\n");
                exit(EXIT_FAILURE);
            }
            //exit(EXIT_SUCCESS);
            
        }
        if((pid2=fork())<0)
        {
            perror("fork2") ;
            exit(1);
        }
        if(pid2==0){
            
            //将共享内存连接到当前进程的地址空间
            shm2 = shmat(shmid, (void*)0, 0);
            if(shm2 == (void*)-1)
            {
                //fprintf(stderr, "shmat failed in begin merge1\n");
                exit(EXIT_FAILURE);
            }
            printf("Memory attached at %X\n", (int)shm2);
            
            shared2 = (struct shared_use_st*)shm2;
            
            printf("child process2: [%d]\n", pid2) ;
            index2 = shared2->index;
            
            shared2->index++ ;
            mergeSort(m+1, r, shmid);
            merge(l, m, r, shmid, shared2);
            while (shared2->index!=index2) {
                sleep(1) ;
            }
            //把共享内存从当前进程中分离
            if(shmdt(shm2) == -1)
            {
                fprintf(stderr, "shmdt failed\n");
                exit(EXIT_FAILURE);
            }
            //删除共享内存
            if(shmctl(shmid, IPC_RMID, 0) == -1)
            {
                //fprintf(stderr, "shmctl(IPC_RMID) failed\n");
                exit(EXIT_FAILURE);
            }
            //exit(EXIT_SUCCESS);
            
        }
        else //Parent process
        {
            //printf("parent process: [%d]\n", pid) ;
        }
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

/* Driver program to test above functions */
int main()
{
    int arr[] = {12, 11, 13, 5, 6, 7};
    int arr_size = sizeof(arr)/sizeof(arr[0]);
    
    printf("Given array is \n");
    printArray(arr, arr_size);
    
    /*Implment with shared memory*/
    
    //创建共享内存
    int shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);
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
    
    struct shared_use_st *shared = NULL;
    shared = (struct shared_use_st*)shm;
    
    // init structure
    shared->index = 0;
    for (int i=0; i<ARRAY_SIZE; i++) {
        shared->arr[i] = arr[i] ;
    }
    
    mergeSort(0, arr_size - 1, shmid);
    
    while (shared->index>-1) {
        sleep(1) ;
    }
    
    printf("\nSorted array is \n");
    printArray(shared->arr, arr_size);
    
    
    return 0;
}