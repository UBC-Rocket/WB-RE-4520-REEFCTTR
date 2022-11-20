#include <stdio.h>

int sabrina(int num);

int main() {
    printf("Hello world!");


     int x=sabrina(31);
    printf("\n%d\n", x);

    printf("i have added a feature");
    
    return 0;
}

int sabrina(int num){
    for(int i=2; i<num; i++){
        if(num % i == 0){
            return 0;
        }
        else{
            return 1;
        }
    }
}


