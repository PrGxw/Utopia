#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
int foo(int **arr){
    return 1;
}
void main(){
    int arr[4][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,16}};
    foo((int**) arr);
    int* arr2 = (int*)malloc(4*sizeof(int));
    int64_t arr2_address = (int)arr2;

}
