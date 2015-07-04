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

    system("pause");
    exit(0); 
}

