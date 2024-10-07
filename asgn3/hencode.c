#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define CHAR_COUNT 256
#define BUFFER_SIZE 100
#define NULL_CHAR '\0'
#define INT_SIZE 32

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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    
    int histogram[CHAR_COUNT] = {0}; 
    int inputFile = open(argv[1], O_RDONLY);
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

    while (read(inputFile, char_buff, 1) > 0) {
        histogram[(int)*char_buff]++;
    }

    // Initialize the linked list
    struct Node* listHead = NULL;
    int i = 0;
    unsigned char num_of_char = 0;
    while (i < CHAR_COUNT) {
        if (histogram[i] > 0) {
            insertNode(&listHead, (unsigned char)i, histogram[i]);
            num_of_char++;
        }
        i++;
    }
    /* For empty*/
    if (!listHead) {
        if (close(inputFile) < 0) {
            perror("close");
            exit(EXIT_FAILURE);
        }

        if (outputFile != STDOUT_FILENO) {
            if (close(outputFile) < 0) {
                perror("close");
                exit(EXIT_FAILURE);
            }
        }
        return 0;
    }


    // struct Node* t;
    // t = listHead;
    // while (t) {
    //     printf("0x%02x\n", t->letter);
    //     t = t->next;
    // }

    listHead = buildHuffmanTree(listHead);


    // Create an array to store Huffman codes for each character
    char* huffmanCodes[CHAR_COUNT] = {NULL};

    // Traverse the Huffman tree to build Huffman codes
    char* currentCode;
    currentCode = (char *)calloc(sizeof(char), CHAR_COUNT);
    DFS(listHead, huffmanCodes, currentCode, 0);

    // Now, you have huffmanCodes[] containing Huffman codes for each character.
    // You can access them using the character's ASCII value as an index.

    num_of_char--;
    unsigned char *pnum_of_char;
    pnum_of_char = &num_of_char;
    if (write(outputFile, pnum_of_char, sizeof(unsigned char)) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    unsigned char *p_char;
    int tfreq_char;
    int *freq_char;


    i = 0;
    unsigned char unsigned_i;
    while (i < CHAR_COUNT) {
        if (histogram[i] > 0) {
            unsigned_i = (unsigned char)i;
            p_char = &unsigned_i;
            if (write(outputFile, p_char, sizeof(char)) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }

            tfreq_char = htonl(histogram[i]);
            freq_char = &tfreq_char;

            if (write(outputFile, freq_char, sizeof(int)) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        i++;
    }
     

    /* Reading the string at the beginnning of the file again */
    int test;
    test = lseek(inputFile, 0, SEEK_SET);
    if (test < 0) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }
    unsigned char *code_string;
    code_string = (unsigned char*)malloc(BUFFER_SIZE);
    if (!code_string) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t size = BUFFER_SIZE;
    int len_code_str = 0;
    const char *huffcode;
    int idx_code_string = 0;
    while (read(inputFile, char_buff, 1) > 0) {
        huffcode = huffmanCodes[(int)*char_buff];
        if (huffcode != NULL) {
            // printf("code: %c \nidx: %02x\n", *huffcode, (int)*char_buff);
            while (*huffcode != NULL_CHAR) {
                if (idx_code_string > BUFFER_SIZE) {
                    size += BUFFER_SIZE;
                    code_string = (unsigned char*)realloc(code_string, size);
                    if (!code_string) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                }
                code_string[idx_code_string] = *huffcode;
                idx_code_string++;
                huffcode++;
                len_code_str++;
            }
        }
    }
    // printf("codestring: %s\n", code_string);
 

    i = 0;
    unsigned int int_code;
    int_code = 0;
    int bit_num = 0;
    unsigned int *pint_code;
    unsigned int tint_code;
    while (i < len_code_str) {
        // printf("bitnum%i", bit_num);
        if (bit_num >= INT_SIZE) {
            tint_code = htonl(int_code);
            pint_code = &tint_code;
            if (write(outputFile, pint_code, sizeof(int)) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            bit_num = 0;
            int_code = 0;
        }
        else {
            if (code_string[i] == '1') {
                int_code = int_code | (1 << (INT_SIZE-1-bit_num));
            }
            bit_num++;
            // printf("\n%02x", int_code);
            i++;
        }
    }
    // printf("\nmainthingy: %02x", int_code);

    if (int_code != 0 || bit_num != 0) {
        int size_code = (bit_num + (INT_SIZE/4) - 1) / (INT_SIZE/4);
        tint_code = htonl(int_code);
        pint_code = &tint_code;
        // printf("\nbitnum%i", bit_num);
        // printf("\nintcode: %02x", int_code);
        // printf("\npintcode: %02x", *pint_code);
        if (write(outputFile, pint_code, size_code) < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    
    if (close(inputFile) < 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    if (outputFile != STDOUT_FILENO) {
        close(outputFile);
    }

    /* Cleanup: Free memory for huffmanCodes */
    i = 0;
    while (i < CHAR_COUNT) {
        if (huffmanCodes[i] != NULL) {
            free(huffmanCodes[i]);
        }
        i++;
    }


    return 0;

}