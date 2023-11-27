#include <stdlib.h>
#include <stdio.h>

int * rand_num_array;

#define LIMIT 100000000

long int n_mult(int n){
    long int c = 0;
    for(int i = 0 ; i <LIMIT; i++){
        if(rand_num_array[i]%n == 0)
        c ++;
    }
    return c;
}
int main(){

    rand_num_array = calloc(LIMIT, sizeof(int));

    for(int i = 0 ; i <LIMIT; i++){
        rand_num_array[i] = random();
    }
    long int n_2 = n_mult(2);
    long int n_3 = n_mult(3);
    long int n_5 = n_mult(5);
    long int n_7 = n_mult(7);
    long int n_11 = n_mult(11);
    printf("%ld %ld %ld %ld %ld \n", n_2, n_3, n_5, n_7, n_11);
    exit(0);

}
