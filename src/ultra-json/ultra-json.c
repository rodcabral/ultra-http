#include "./ultra-json.h"

typedef struct _Node {
    char ch;
    struct _Node *prev;
    struct _Node *next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
} Stack;

Stack* init_stack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    
    stack->head = NULL;
    stack->tail = NULL;

    return stack;
}

void delete_stack(Stack* stack) {
    if(!stack->head) return;

    Node* temp = stack->head;

    while(stack->head) {
        temp = stack->head;
        stack->head  = stack->head->next;
        free(temp);
    }
    
    free(stack);
}

void push(Stack* stack, char ch) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->ch = ch;

    if(!stack->head) {
        stack->head = temp;
        stack->tail = temp;
        return;
    }

    temp->next = NULL;
    temp->prev = stack->tail;

    stack->tail->next = temp;
    stack->tail = temp;
}

char pop(Stack* stack) {
    Node* temp = stack->tail;
    char ch = temp->ch;

    if(temp->prev) {
        stack->tail = temp->prev;
    }

    if(temp == stack->head) {
        free(stack->head);
        stack->head = NULL;
        return ch;
    }

    if(temp) {
        free(temp);
        temp = NULL;
    }

    return ch;
}

UltraJson *ultra_json(char* json) {
    UltraJson* ujson = (UltraJson*)malloc(sizeof(UltraJson));

    if(!json) {
        return NULL;
    }

    ujson->data = json;
    ujson->parsed = malloc(strlen(json));

    Stack* stack = init_stack();

    delete_stack(stack);

    return ujson;
}

void ultra_json_close(UltraJson* ujson) {
    if(!ujson) return;

    ujson->data = NULL;

    if(ujson->parsed) {
        free(ujson->parsed);
    }

    free(ujson);
}
