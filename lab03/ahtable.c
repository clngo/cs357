#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct node* insert(struct node *list, struct node *new);
struct node* makehuffman(struct node *root);
struct node* createnode(int f, unsigned int b);
void gethuffcodes(struct node *root, 
char** codelist, char* strcode, int len); 
void printchars(char *s);

typedef struct node {
	unsigned int freq;
	unsigned char byte;
	struct node *next;
	struct node *left;
	struct node *right;
} node;


int main(int argc, char* argv[]) {
	int freq_list[256];
	int c;
	unsigned int i;
	struct node *head;	
	struct node *root;
	char *code;
    char *codes[256] = {NULL};
	
	FILE *inputFile = fopen(argv[1], "r");

	if (inputFile == NULL) {
    	perror("fopen");
    	exit(1);
	}


	code = (char *) calloc((sizeof(char)), 256);

	head = NULL;
	root = NULL;	
	/* initialize array to 0 */
	for (i = 0; i < 256; i++) {
		freq_list[i] = 0;
	}

	/* make freq list */
	while ((c = fgetc(inputFile)) != EOF) {
		freq_list[c] += 1;	
	}


	for (i = 0; i < 256; i++) {
		/* print out freq_list */
		if (freq_list[i] != 0){
			// printf("%x: %d\n", i, freq_list[i]); 
			head = insert(head, createnode(freq_list[i], i));
		}
	}
	root = head;
	root = makehuffman(head);


	gethuffcodes(root, codes, code, 0);	  
	
	for (i = 0; i < 256; i++) {
		if (freq_list[i] != 0){
			printf("0x%02x: ", i);
			printchars(codes[i]);
			printf("\n");	
		}
	}

	// // Cleanup: Free memory for huffmanCodes
    i = 0;
    while (i < 256) {
        if (codes[i] != NULL) {
            free(codes[i]);
        }
        i++;
    }

	return 0;
}

void printchars(char *s){
	int i;
	i = 0;
	while(s[i]){
		printf("%c", s[i]);
		i++;
	}
}

void gethuffcodes(struct node *head, 
	char **codelist, char *code, int len) {

	int c;

	if (!head) {
		return;
	}

	if((head->left == NULL) && (head->right == NULL)){
		c = head->byte;
        codelist[c] = (char *)malloc(len + 1);
        if(!codelist[c]) {
            perror("malloc huffcodes");
            exit(1);
        }
        strcpy(codelist[(int) c], code);
        return;
    }

	if(head->left != NULL){
		code[len] = '0';
		code[len + 1] = '\0';
		gethuffcodes(head->left, codelist, code, len + 1);
		code[len] = '\0';
	}

	if(head->right != NULL){
		code[len] = '1';
		code[len + 1] = '\0';
		gethuffcodes(head->right, codelist, code, len + 1);
		code[len] = '\0';
	}

}

struct node* createnode (int f, unsigned int b) {
	struct node *new;
	new = (struct node*)malloc(sizeof(struct node));

	if(!new) {
		perror("malloc createnode");
		exit(1);
	}
	new -> freq = f;
	new -> byte = b;
	new -> next = NULL;
	new -> left = NULL;
	new -> right = NULL;
	
	return new;
}

struct node* insert (struct node *list, struct node *new) {
	struct node *t;

	/* insert new at beginning of list */
	if (!list || list->freq >= new->freq) {
		new -> next = list;
		list = new;
	}
	else { /* insert new in middle/end of list (ordered) */
		for (t = list; 
		(t -> next && t->next->freq < new->freq); 
		t = t -> next) {
		/* this just makes t the node before where we want to insert */
		;
		}

		/* TIEBREAKER if freq are the same and NOT at the end*/
		if(t->next != NULL){
			if (t->next->freq == new->freq) {
				while (t->next != NULL && 
				t->next->freq == new->freq) {
					if(t->next->byte < new->byte) {
						t = t->next;
					}
				}
			}
		}

		new -> next = t -> next;
		t -> next = new;
	}
	
	return list;
}

struct node* makehuffman (struct node *root) {
	/* loop until you only have one thing in the list */
	struct node *n1;
	struct node *n2;
	struct node *new;
	struct node *t;

	if (!root || !(root->next)) {
		return root;
	}

	while (root->next != NULL) {
		n1 = root;
		n2 = root->next;
		root = n2->next;

		new = (struct node*)malloc(sizeof(struct node));
		if (!new) {
			perror("malloc huff");
			exit(1);
		}


		new->byte = n1->byte;
		new->freq = (n1->freq) + (n2->freq);
		new->left = n1;
		new->right = n2;
		new->next = NULL;
	
		if (!root || root->freq >= new->freq) {
        	        new -> next = root;
                	root = new;
        	}
        else { /* insert new in middle/end of list (ordered) */
            for (t = root; 
			(t -> next && t->next->freq < new->freq); 
			t = t -> next) {
                 /* this just makes t the node 
 				* before where we want to insert */;
                	}

                /* TIEBREAKER if freq are the same*/

            new -> next = t -> next;
        	t -> next = new;
        	}
	}

	return root;

/*
	n1 = list;
	n2 = list->next;

        hnode = malloc(sizeof(node));
        hnode -> freq = (n1->freq) + (n2->freq);
        hnode -> byte = n1 -> byte;
        hnode -> left = n1;
        hnode -> right = n2;


	list = list->next->next;

	insert(list, hnode);

        printf("added to huffman tree\n");
        return list; */	
}
