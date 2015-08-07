#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Hello world!\n");  

    int i ;
    for (i=0; i<10; i++)
        printf("%d\n", i) ;

    if (i==10)
        printf("%s", "I equal 10\n");
    else
        printf("%d\n", i);

    printf("Finish C\n") ;

    printf("%d\n", 100) ;

    system("pause");

    short intSize = sizeof(int);
    printf("Size of int: ");
    printf("%d\n", intSize) ;

    short shortSize = sizeof(short int);
    printf("Size of short int: ");
    printf("%d\n", shortSize);

    short longSize = sizeof(long int);
    printf("Size of long int:" );
    printf("%d\n", longSize);

    short floatSize = sizeof(float);
    printf("Size of fload: ");
    printf("%d\n", floatSize);

    short doubleSize = sizeof(double);
    printf("Size of double: ");
    printf("%d\n", doubleSize);

    short longDoubleSize = sizeof(long double);
    printf("Size of long double: ") ;
    printf("%d\n", longDoubleSize) ;

    short charSize = sizeof(char);
    printf("Size of char: ") ;
    printf("%d\n", charSize) ;

    int address = 100 ;
    int *a = &address ;
    printf("%p\n", &address) ;
    printf("%p\n", a);
    printf("%d\n", *&address);
    printf("%p\n", &a);
    //exit(0) ;
    return 0 ;
}
