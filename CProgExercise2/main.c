#include<stdio.h>
#include<string.h>

int powerOfNumber(int m, int n)
{
    if(n == 0)
        return 1;
 
    int temp = powerOfNumber(m, n/2);
    
    if(n % 2 == 0)
        return temp * temp;
    else
        return m * temp * temp;
}

int main(){

    for(int i = 0 ; i < 10; i++){
        int ans = powerOfNumber(2,10);
        printf("%d \n", ans);         
    }
    return 0 ; 
}