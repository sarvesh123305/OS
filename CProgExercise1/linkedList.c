#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "linkedList.h"

#define MAX 2048

void initList(List *L1) {
    *L1 = NULL;
    return ;
}
void initNumber(Number *N1)
{
    initList(&(*N1).head);
    initList(&(*N1).tail);
    return; 
}

void insertFront(Number* head,int data){ //Appends number to first of list
    Node* newnode = (Node*)malloc(sizeof(Node));
    if(!newnode)
        return ;

    newnode -> data = data;
    newnode -> next = NULL;
    newnode -> prev = NULL;


    if(!((*head).head)){
        (*head).head = newnode;    //If list is null newnode is the head
        (*head).tail = newnode;    //If list is null newnode is the head
        return;
    }

    newnode -> next = (*head).head;        //otherwise join the list to next of newnode and make newnode as head
    (*head).head -> prev = newnode;
    (*head).head = newnode;
    return ;
}


void append(Number* head,int data){        //Appends number to last of list
    Node* newnode = (Node*)malloc(sizeof(Node));
    
    if(!newnode)
        return ;

    newnode -> data = data;
    newnode -> next = NULL;
    newnode -> prev = NULL;

    if(!((*head).head)){
        (*head).head = newnode;        //If list is null newnode is the head
        (*head).tail = newnode;        //If list is null newnode is the head
        return ;
    }

    Node* temp = (*head).tail;         //otherwise go to end of list and append the number to last
    newnode -> prev = temp;
    temp -> next = newnode ;
    (*head).tail = newnode;
    return ;
}


void display(List head){

    printf("The addition is : " );
    Node* temp = head; 
    while(temp){
        printf("%d ", temp -> data);
        if(temp -> next)
            printf(" -> ");
        temp = temp -> next;
    }

    printf("\n");
    return ;
}

void removePreceedingZeros(Number *L){
    if(!((*L).head))
    return ;

    Node* temp = (*L).head;

    //Checking for preeceding zeros and removing it Eg : 0012 -> 12
    //This would make our subtraction task easy

    while(temp -> next){
        if(temp -> data == 0)
        {
            Node* deleteNode = temp;
            temp = temp -> next;
            free(deleteNode);
        }
        else{
            break;  // Break the loop when there are no preceding zeros
        }
    }
    (*L).head = temp;

    return ;
}


void freeList(List* numberList){
    while(numberList){
        List* freeNode = numberList;
        *(numberList) = (*numberList)-> next;           //Freeing up the space 
        free(freeNode);
    }
}

void takeInput(Number *N1){
    initNumber(N1);
    char str[MAX] = "";

        printf("Enter a number : "); 
        fgets(str,MAX,stdin);

        int len = strlen(str);
        if(len == 0)
            printf("Invalid Input ");
        
        else{
            str[len-1] = '\0';
            for(int i = 0 ; i < len-1; i++){
                int digit = str[i] - '0';
                append(N1,digit);

            }
            //removePreeceding zeros
            removePreceedingZeros(N1);
        }
}

Number addTwoLinkedLists(Number head1,Number head2){
    Number L3;
    initNumber(&L3);

    int carry = 0 ;
    int sum = 0 ;
    
    Node* num1 = head1.tail;
    Node* num2 = head2.tail;

    while(num1 || num2 || carry ){
        int val1 = 0 , val2 = 0 ;
            if(num1)
                val1 = num1 -> data;
            if(num2)
                val2 = num2 -> data;
            
            sum = val1 + val2 + carry;
            
            insertFront(&L3,sum%10);
            carry = sum / 10 ;

            if(num1)
                num1 = num1 -> prev;
            if(num2)
                num2 = num2 -> prev;
    } 
    
    removePreceedingZeros(&L3);
    display(L3.head);
    return L3;
}
