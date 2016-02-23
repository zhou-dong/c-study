# Shared Memory

## 什么是共享内存

顾名思义，共享内存就是允许两个不相关的进程访问同一个逻辑内存。共享内存是在两个正在运行的进程之间共享和传递数据的一种非常有效的方式。不同进程之间共享的内存通常安排为同一段物理内存。进程可以将同一段共享内存连接到它们自己的地址空间中，所有进程都可以访问共享内存中的地址，就好像它们是由用C语言函数malloc分配的内存一样。而如果某个进程向共享内存写入数据，所做的改动将立即影响到可以访问同一段共享内存的任何其他进程。

*特别提醒*：共享内存并未提供同步机制，也就是说，在第一个进程结束对共享内存的写操作之前，并无自动机制可以阻止第二个进程开始对它进行读取。所以我们通常需要用其他的机制来同步对共享内存的访问，例如信号量。

## 共享内存的函数

与信号量一样，在Linux中也提供了一组函数接口用于使用共享内存，而且使用共享内存的接口还与信号量的非常相似，而且比使用信号量的接口来得简单。它们声明在头文件 sys/shm.h中。

### 1. shmget函数

该函数用来创建共享内存，它的原型为：

```C
int shmget(key_t key, size_t size, int shmflg);  
```

第一个参数，与信号量的semget函数一样，程序需要提供一个参数key（非0整数），它有效地为共享内存段命名，shmget函数成功时返回一个与key相关的共享内存标识符（非负整数），用于后续的共享内存函数。调用失败返回-1.

不相关的进程可以通过该函数的返回值访问同一共享内存，它代表程序可能要使用的某个资源，程序对所有共享内存的访问都是间接的，程序先通过调用shmget函数并提供一个键，再由系统生成一个相应的共享内存标识符（shmget函数的返回值），只有shmget函数才直接使用信号量键，所有其他的信号量函数使用由shmget函数返回的信号量标识符。

第二个参数，size以字节为单位指定需要共享的内存容量.

第三个参数，shmflg是权限标志，它的作用与open函数的mode参数一样，如果要想在key标识的共享内存不存在时，创建它的话，可以与IPC_CREAT做或操作。共享内存的权限标志与文件的读写权限一样，举例来说，0644,它表示允许一个进程创建的共享内存被内存创建者所拥有的进程向共享内存读取和写入数据，同时其他用户创建的进程只能读取共享内存。

### 2. shmat函数

第一次创建完共享内存时，它还不能被任何进程访问，shmat函数的作用就是用来启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间。它的原型如下：

```C
void *shmat(int shm_id, const void *shm_addr, int shmflg);
```

**

第一个参数，shm_id是由shmget函数返回的共享内存标识。

第二个参数，shm_addr指定共享内存连接到当前进程中的地址位置，通常为空，表示让系统来选择共享内存的地址。

第三个参数，shm_flg是一组标志位，通常为0。

调用成功时返回一个指向共享内存第一个字节的指针，如果调用失败返回-1.

### 3.shmdt函数

该函数用于将共享内存从当前进程中分离。注意，将共享内存分离并不是删除它，只是使该共享内存对当前进程不再可用。它的原型如下：

```C
int shmdt(const void *shmaddr);  
```

参数shmaddr是shmat函数返回的地址指针，调用成功时返回0，失败时返回-1.


### 4 shmctl函数

与信号量的semctl函数一样，用来控制共享内存，它的原型如下：

```C
int shmctl(int shm_id, int command, struct shmid_ds *buf);  
```

第一个参数，shm_id是shmget函数返回的共享内存标识符。

第二个参数，command是要采取的操作，它可以取下面的三个值 ：

- IPC_STAT：把shmid_ds结构中的数据设置为共享内存的当前关联值，即用共享内存的当前关联值覆盖shmid_ds的值。
- IPC_SET：如果进程有足够的权限，就把共享内存的当前关联值设置为shmid_ds结构中给出的值
- IPC_RMID：删除共享内存段

第三个参数，buf是一个结构指针，它指向共享内存模式和访问权限的结构。


```C
struct shmid_ds  
{  
    uid_t shm_perm.uid;  
    uid_t shm_perm.gid;  
    mode_t shm_perm.mode;  
};  
```

## 使用共享内存进行进程间通信

下面就以两个不相关的进程来说明进程间如何通过共享内存来进行通信。其中一个文件shmread.c创建共享内存，并读取其中的信息，另一个文件shmwrite.c向共享内存中写入数据。为了方便操作和数据结构的统一，为这两个文件定义了相同的数据结构，定义在文件shmdata.c中。结构shared_use_st中的written作为一个可读或可写的标志，非0：表示可读，0表示可写，text则是内存中的文件。

shmdata.h的代码：

```C
#ifndef _SHMDATA_H_HEADER  
#define _SHMDATA_H_HEADER  

#define TEXT_SZ 2048  

struct shared_use_st  
{  
    int written;//作为一个标志，非0：表示可读，0表示可写  
    char text[TEXT_SZ];//记录写入和读取的文本  
};  

#endif
```

shmread.c的代码：

```C
#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <sys/shm.h>  
#include "shmdata.h"  

int main()  
{  
    int running = 1;//程序是否继续运行的标志  
    void *shm = NULL;//分配的共享内存的原始首地址  
    struct shared_use_st *shared;//指向shm  
    int shmid;//共享内存标识符  
    //创建共享内存  
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);  
    if(shmid == -1)  
    {  
        fprintf(stderr, "shmget failed\n");  
        exit(EXIT_FAILURE);  
    }  
    //将共享内存连接到当前进程的地址空间  
    shm = shmat(shmid, 0, 0);  
    if(shm == (void*)-1)  
    {  
        fprintf(stderr, "shmat failed\n");  
        exit(EXIT_FAILURE);  
    }  
    printf("\nMemory attached at %X\n", (int)shm);  
    //设置共享内存  
    shared = (struct shared_use_st*)shm;  
    shared->written = 0;  
    while(running)//读取共享内存中的数据  
    {  
        //没有进程向共享内存定数据有数据可读取  
        if(shared->written != 0)  
        {  
            printf("You wrote: %s", shared->text);  
            sleep(rand() % 3);  
            //读取完数据，设置written使共享内存段可写  
            shared->written = 0;  
            //输入了end，退出循环（程序）  
            if(strncmp(shared->text, "end", 3) == 0)  
                running = 0;  
        }  
        else//有其他进程在写数据，不能读取数据  
            sleep(1);  
    }  
    //把共享内存从当前进程中分离  
    if(shmdt(shm) == -1)  
    {  
        fprintf(stderr, "shmdt failed\n");  
        exit(EXIT_FAILURE);  
    }  
    //删除共享内存  
    if(shmctl(shmid, IPC_RMID, 0) == -1)  
    {  
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");  
        exit(EXIT_FAILURE);  
    }  
    exit(EXIT_SUCCESS);  
}  
```

shmwrite.c的代码

```C
#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <sys/shm.h>  
#include "shmdata.h"  

int main()  
{  
    int running = 1;  
    void *shm = NULL;  
    struct shared_use_st *shared = NULL;  
    char buffer[BUFSIZ + 1];//用于保存输入的文本  
    int shmid;  
    //创建共享内存  
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);  
    if(shmid == -1)  
    {  
        fprintf(stderr, "shmget failed\n");  
        exit(EXIT_FAILURE);  
    }  
    //将共享内存连接到当前进程的地址空间  
    shm = shmat(shmid, (void*)0, 0);  
    if(shm == (void*)-1)  
    {  
        fprintf(stderr, "shmat failed\n");  
        exit(EXIT_FAILURE);  
    }  
    printf("Memory attached at %X\n", (int)shm);  
    //设置共享内存  
    shared = (struct shared_use_st*)shm;  
    while(running)//向共享内存中写数据  
    {  
        //数据还没有被读取，则等待数据被读取,不能向共享内存中写入文本  
        while(shared->written == 1)  
        {  
            sleep(1);  
            printf("Waiting...\n");  
        }  
        //向共享内存中写入数据  
        printf("Enter some text: ");  
        fgets(buffer, BUFSIZ, stdin);  
        strncpy(shared->text, buffer, TEXT_SZ);  
        //写完数据，设置written使共享内存段可读  
        shared->written = 1;  
        //输入了end，退出循环（程序）  
        if(strncmp(buffer, "end", 3) == 0)  
            running = 0;  
    }  
    //把共享内存从当前进程中分离  
    if(shmdt(shm) == -1)  
    {  
        fprintf(stderr, "shmdt failed\n");  
        exit(EXIT_FAILURE);  
    }  
    sleep(2);  
    exit(EXIT_SUCCESS);  
}  
```

---


引用： http://blog.csdn.net/ljianhui/article/details/10253345
