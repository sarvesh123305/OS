#include<stdio.h>
#include<string.h>
#include "linkedList.h"

int main(){
    Number N1,N2;
    initNumber(&N1);
    takeInput(&N1); // for number 1
    takeInput(&N2); // for number 2
    addTwoLinkedLists(N1,N2);
}