#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_COUNT 256
#define NULL_CHAR '\0'

struct Node {
    unsigned char letter;
    unsigned int freq;
    struct Node* next;
    struct Node* left;
    struct Node* right;
};

void insertNode(struct Node** head, unsigned char letter, int freq) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));

    if (newNode == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    newNode->letter = letter;
    newNode->freq = freq;
    newNode->next = NULL;
    newNode->left = NULL;
    newNode->right = NULL;

    // Debugging statement to check the insertion
    printf("Inserted: letter=%c, freq=%d\n", letter, freq);

    struct Node** current = head;
    struct Node* previous = NULL;

    while (*current != NULL && (freq > (*current)->freq || 
    (freq == (*current)->freq && letter > (*current)->letter))) {
        previous = *current;
        current = &((*current)->next);
    }
    // Debugging statement to check the position in the list
    // printf("Inserting at position: %p\n", (void*)current);

    if (previous == NULL) {
        newNode->next = *head;
        *head = newNode;
    } else {
        newNode->next = *current;
        previous->next = newNode;
    }

    if (*current == NULL) {
        newNode->next = NULL;
    }
}

// Function to construct the Huffman tree
struct Node* buildHuffmanTree(struct Node* listHead) {
    if (listHead == NULL || listHead->next == NULL) {
        return listHead; // No further processing is needed
    }
    while (listHead->next != NULL) {
        // Remove the first two nodes
        struct Node* first = listHead;
        struct Node* second = listHead->next;
        listHead = second->next;

        // Create a new node with the sum of frequencies
        struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
        if (newNode == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }


        newNode->letter = NULL_CHAR; // Not necessary for Huffman

        // Calculate the sum of frequencies
        int newFreq = (first->freq) + (second->freq);

        newNode->freq = newFreq;
        newNode->next = NULL;
        newNode->left = first;
        newNode->right = second;

        // Reinsert the new node into the list in order
        struct Node* current = listHead;
        struct Node* previous = NULL;

        while (current != NULL && newFreq > current->freq) {
            previous = current;
            current = current->next;
        }

        if (previous == NULL) {
            newNode->next = listHead;
            listHead = newNode;
        } else {
            newNode->next = current;
            previous->next = newNode;
        }
    }
    return listHead; // Return the root of the Huffman tree
}

// void printCharacterFrequencies(int histogram[]) {
//     for (int i = 0; i < CHAR_COUNT; i++) {
//         if (histogram[i] > 0) {
//             printf("0x%02x ('%c'): %d\n", i, (char)i, histogram[i]);
//         }
//     }
// }


void DFS(struct Node* node, char* huffmanCodes[], 
    char* currentCode, int level) {
    if (!node) {
        return;
    }
    if (!node->left && !node->right) {
        // Leaf node reached, store the Huffman code in huffmanCodess
        int c = node->letter;
        huffmanCodes[c] = (char*)malloc(level + 1);
        if (huffmanCodes[c] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(huffmanCodes[c], currentCode);
        return; // Exit the DFS for this leaf node
    }

    if (node->left) {
        currentCode[level] = '0';
        currentCode[level + 1] = NULL_CHAR;
        DFS(node->left, huffmanCodes, currentCode, level + 1);
        currentCode[level] = NULL_CHAR; 
    }

    if (node->right) {
        currentCode[level] = '1';
        currentCode[level + 1] = NULL_CHAR;
        DFS(node->right, huffmanCodes, currentCode, level + 1);
        currentCode[level] = NULL_CHAR; 
    }
}


// void DFS(struct Node* node) {
//     if (node == NULL) {
//         return;
//     }

//     if (!node->left && !node->right) {
//         printf("%c", node->letter); 
//     }

//     DFS(node->left); // Traverse the left subtree
//     DFS(node->right); // Traverse the right subtree
// }


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    
    int histogram[CHAR_COUNT] = {0}; 
    FILE *inputFile = fopen(argv[1], "r");

    if (inputFile == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int c;
    while ((c = fgetc(inputFile)) != EOF) {
        if (c >= 0 && c < CHAR_COUNT) {
            histogram[c]++;
        }
    }
    fclose(inputFile);


    // Initialize the linked list
    struct Node* listHead = NULL;
    int i = 0;
    while (i < CHAR_COUNT) {
        if (histogram[i] > 0) {
            insertNode(&listHead, (char)i, histogram[i]);
        }
        i++;
    }

    struct Node* t;
    t = listHead;
    while (t) {
        printf("0x%02x\n", t->letter);
        t = t->next;
    }

    // listHead = buildHuffmanTree(listHead);
    listHead = buildHuffmanTree(listHead);


    // Create an array to store Huffman codes for each character
    char* huffmanCodes[CHAR_COUNT] = {NULL};
    // for (i = 0; i < CHAR_COUNT; i++) {
    //     huffmanCodes[i] = NULL;
    // }

    // Traverse the Huffman tree to build Huffman codes
    char* currentCode;
    currentCode = (char *)calloc(sizeof(char), CHAR_COUNT);
    DFS(listHead, huffmanCodes, currentCode, 0);

    // Now, you have huffmanCodes[] containing Huffman codes for each character.
    // You can access them using the character's ASCII value as an index.

    i = 0;
    while (i < CHAR_COUNT) {
        if (huffmanCodes[i] != NULL) {
            printf("0x%02x: %s\n", i, huffmanCodes[i]);
        }
        i++;
    }

    // // Cleanup: Free memory for huffmanCodes
    i = 0;
    while (i < CHAR_COUNT) {
        if (huffmanCodes[i] != NULL) {
            free(huffmanCodes[i]);
        }
        i++;
    }


    return 0;

}