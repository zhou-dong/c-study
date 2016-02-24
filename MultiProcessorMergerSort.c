#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


 
 #define BUFFER_SIZE 2048
 char arr[] = {12,11,13,5,6,7};
 int arr_size = sizeof(arr)/sizeof(arr[0]);

void merge(char arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
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
void mergeSort(char arr[], int l, int r)
{
    if (l < r)
    {
        int m = l+(r-l)/2; //Same as (l+r)/2, but avoids overflow for large l and h
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);
    }
}
 
 
/* UITLITY FUNCTIONS */
/* Function to print an array */
void printArray(char A[], int size)
{
    int i;
    for (i=0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}
 int main() {
     pid_t pid;
     int shmid;
     char *shm_addr;
     char flag[]="Parent";
     char buff[BUFFER_SIZE];
     char  a[20];
 // 创建当前进程的私有共享内存
     if ((shmid=shmget(IPC_PRIVATE,BUFFER_SIZE,0666))<0) {
         perror("shmget");
         exit(1);
     }
     if((pid=fork())<0) {
         perror("fork");
         exit(1);
     }else if (pid>0) {
 // 自动分配共享内存映射地址，为可读可写，映射地址返回给shm_addr
         if ((shm_addr=shmat(shmid,0,0))==(void*)-1) {
             perror("Parent:shmat");
             exit(1);
         }
 // 比较shm_addr,flag的长度为strlen(flag)的字符
 // 当其内容相同时，返回0
 // 否则返回（str1[n]-str2[n]）
         while (strncmp(shm_addr,flag,strlen(flag))) {
             printf("Parent: Waiting for data...\n");
             sleep(10);
         }
 
         strcpy(buff,shm_addr+strlen(flag));
         printf("Parent: Sorted Array:");
         printArray(buff,arr_size);
 // 删除父进程的共享内存映射地址
         if (shmdt(shm_addr)<0) {
             perror("Parent:shmdt");
             exit(1);
         }
 
     }else{
         sleep(1);
 // 自动分配共享内存映射地址，为可读可写，映射地址返回给shm_addr
         if ((shm_addr=shmat(shmid,0,0))==(void*)-1) {
             perror("Child:shmat");
             exit(1);
         }
 // shm_addr为flag+stdin
         sleep(1);
         printf("Child:given array is \n");
         printArray(arr, arr_size);
         mergeSort(arr, 0, arr_size - 1);
         printf("Child:finish sorting! \n");
         strncpy(shm_addr+strlen(flag),arr,strlen(arr));
         strncpy(shm_addr,flag,strlen(flag));
         printf("Child:data has been sent to shared memory! \n");
 // 删除子进程的共享内存映射地址
         if (shmdt(shm_addr)<0) {
             perror("Child:shmdt");
             exit(1);
         }
 // 保证子进程在删除共享内存前，父进程能读到共享内存的内容
         waitpid(pid,NULL,0);
 // 删除共享内存
         if (shmctl(shmid,IPC_RMID,NULL)==-1) {
             perror("shmct:IPC_RMID");
             exit(1);
         }
 
       
     }
 
     exit(0);
 }
