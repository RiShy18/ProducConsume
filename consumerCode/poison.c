#include <stdio.h>
#include <math.h>
#include <time.h>
 
double U_Random ();
int possion ();
 
 
void main (){
    double u = U_Random ();
    int p = possion ();
    int i = 0;
    while(i!=10){
        int f = rand();
        printf("valor: %d \n", f); 
        i++;
    }
    printf ("randon = %f \n", u );
    printf ("numero de poison = %d \n", p );
}
 
int possion ()/* generates a random number with a Poisson distribution. Lamda is the average number */
{
    int Lambda = 20, k = 0;
    long double p = 1.0;
    long double l = exp (-Lambda);/* it is defined as long double for precision, and exp (-Lambda) is a decimal near 0 */
    printf ("%.15Lfn", l);
    while (p >= l){
        double u = U_Random ();
        p *= u;
        k ++;
    }
    return k-1;
}
 
double U_Random ()/* generates a 0 ~ Random number between 1 */{
    double f;
    f =  rand() % 100 ;
    printf("El valor del random es: %f \n", f/100);
/* Printf ("% fn", f );*/
    return f/100;
}


