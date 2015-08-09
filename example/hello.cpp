//类：就是包含了函数的structure
#include <iostream>

class Rectangle {

    public: int w ; 
           int h ;

    void init(int w_, int h_){
        w = w_;
        h = h_ ;
    }

    int Area(){
        return w * h ;
    }

    int Perimeter(){
        return 2 * (w + h) ;
    }

} ;

int main(){
    
    Rectangle r ;
    r.init(10,20);
    int a = r.Area() ;

    printf("%d\n", 7);

    return 0 ;
}
