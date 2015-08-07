//类：就是包含了函数的structure
#include <iostream>
using namespace std;

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
    cout << a << endl  ;

}
