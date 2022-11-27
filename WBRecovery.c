#include <stdio.h>

int main() {
    int x = hitesh(31);
    printf(x);
    return 0;
}

int hitesh(int number) {
    printf("hello world");
    for (int i = 2; i < number; i++) {
        if (number % i == 0){
            return 1;
        }
    }
    printf("this is a test");
    
    return 0;
}