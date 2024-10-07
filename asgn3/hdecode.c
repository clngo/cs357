#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define CHAR_COUNT 256
#define BUFFER_SIZE 100
#define INT_BITS 32
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
    // printf("Inserted: letter=%c, freq=%d\n", letter, freq);

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


int main(int argc, char *argv[]) {
    int inputFile;

    if (argc < 2 || (strcmp(argv[1], "-") == 0)) {
        inputFile = STDIN_FILENO;
    }
    else {
        inputFile = open(argv[1], O_RDONLY);
        if (inputFile < 0) {
        fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
        exit(EXIT_FAILURE);
        }
    }

    

    int outputFile;
    if ((outputFile = open(argv[2], O_WRONLY | O_CREAT 
    | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
        outputFile = STDOUT_FILENO;
    }
    
    if (inputFile < 0) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    unsigned char char_buff[2];
    char_buff[1] = NULL_CHAR;
    unsigned char num_of_char;
    unsigned char *pnum_of_char;
    pnum_of_char = &num_of_char;
    if (read(inputFile, pnum_of_char, sizeof(unsigned char)) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }    
    
    int tfreq_char;
    int *freq_char;
    int total_freq = 0;

    int histogram[CHAR_COUNT] = {0}; 

    int i = 0;
    // printf("num_of_char: %i", *pnum_of_char);
    while (i < (*pnum_of_char+1)) {
        if (read(inputFile, char_buff, sizeof(char)) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        tfreq_char = 0;
        freq_char = &tfreq_char;

        if (read(inputFile, freq_char, sizeof(int)) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        *freq_char = (int)htonl(*freq_char);
        // printf("\nchar: %02x, freq: %i", *char_buff, *freq_char);
        histogram[(int)*char_buff] = (int)*freq_char;
        total_freq += (int)*freq_char;
        i++;
    }

    // Initialize the linked list
    struct Node* listHead = NULL;
    i = 0;
    while (i < CHAR_COUNT) {
        if (histogram[i] > 0) {
            insertNode(&listHead, (char)i, histogram[i]);
        }
        i++;
    }

    // struct Node* t;
    // t = listHead;
    // while (t) {
    //     printf("0x%02x\n", t->letter);
    //     t = t->next;
    // }

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



    unsigned int int_code;
    unsigned int *pint_code;

    pint_code = (unsigned int*)malloc(sizeof(unsigned int));
    if (!pint_code) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }


    /* Create a string of the codes from reading */

    unsigned char *code_string;
    code_string = (unsigned char*)malloc(BUFFER_SIZE);
    if (!code_string) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int bit_num; 
    unsigned char temp_code[INT_BITS];
    size_t code_string_size = BUFFER_SIZE;
    int idx_code_string = 0;
    int freq_counter = 0;
    if (*pnum_of_char == 0) {
        while (freq_counter < total_freq) {
            if (write(outputFile, char_buff, sizeof(char))< 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            freq_counter++;
        }
    }
    while (read(inputFile, pint_code, sizeof(int)) > 0) {
        int_code = htonl(*pint_code);
        // printf("\nint_code: %02x",int_code);

        i = 0;
        while (i < INT_BITS) {
            bit_num = int_code & (1 << (INT_BITS-1-i));
            // printf("\nbit_num: %i", bit_num);
            if (bit_num == 0) {
                temp_code[i] = '0';
            }
            else {
                temp_code[i] = '1';
            }
            i++;          
        }
        temp_code[i] = NULL_CHAR;
        // printf("\ntempcode: %s", temp_code);

        i = 0;
        while (temp_code[i] != NULL_CHAR) {
            if (idx_code_string > BUFFER_SIZE) {
                code_string_size += BUFFER_SIZE; // Double the size
                code_string = (unsigned char *)realloc(code_string, 
                code_string_size);
                if (!code_string) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }
            code_string[idx_code_string] = temp_code[i];
            i++;
            idx_code_string++;
            calloc(INT_BITS, sizeof(int));
        }
    }
    // printf("\ncodestring: %s", code_string);
    
    i = 0;
    struct Node* temp_pointer = listHead;
    freq_counter = 0;
    while (code_string[i] != NULL_CHAR && (freq_counter < total_freq)) {
        if (!listHead) {
            break;
        }
        if (code_string[(int)i] == '0') {
            temp_pointer = temp_pointer->left;
            // printf("0");
            if ((!temp_pointer->left) && (!temp_pointer->right)) {
                /* At the leaf node, write out the character */
                if (write(outputFile, &temp_pointer->letter, sizeof(char))< 0) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                // printf("writing: %c\n", temp_pointer->letter);
                freq_counter++;
                temp_pointer = listHead;
            }
        }
        else if (code_string[(int)i] == '1') {
            temp_pointer = temp_pointer->right;
            // printf("1");
            if ((!temp_pointer->left) && (!temp_pointer->right)) {
                /* At the leaf node, write out the character */
                if (write(outputFile, &temp_pointer->letter, sizeof(char))< 0) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                // printf("writing: %c\n", temp_pointer->letter);
                freq_counter++;
                temp_pointer = listHead;
            }
        }
        else if ((!temp_pointer->left) && (!temp_pointer->right)) {
            if (write(outputFile, &temp_pointer->letter, sizeof(char))< 0) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                // printf("writing: %c\n", temp_pointer->letter);
                freq_counter++;
                temp_pointer = listHead;
        }
        i++;

    }
    
    if (close(inputFile) < 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    if (outputFile != STDOUT_FILENO) {
        close(outputFile);
    }
    return 0;

}