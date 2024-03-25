
typedef struct Node{
    short int data;
    struct Node *next;
    struct Node *prev;
}Node;

typedef Node* List;

typedef struct Number{
    List head;
    List tail;
}Number;


void initList(List*);
void initNumber(Number *N1);
void freeList(List* numberList);
void removePreceedingZeros(Number *L);
void insertFront(Number* head,int data);//Appends number to first of list
void append(Number* head,int data);        //Appends number to last of list
void takeInput(Number *L1);
Number addTwoLinkedLists(Number head1,Number head2);

