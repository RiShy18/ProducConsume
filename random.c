#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double ran_expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

int main(){
    srand((unsigned)time(NULL));
    printf("%f", ran_expo(5));
    return 0;
}