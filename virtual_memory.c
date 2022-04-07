/*
 * =====================================================================================
 *
 *       Filename:  virtual_memory.c
 *
 *    Description:  Page replacement algorithms for completing the illusion of
 *    infinite virtual memory by masking the need for reading pages from disk
 *    for every execution. I implement two algorithms First In First Out and
 *    Least Recently Used(LRU) and measure their performance on three benchmark
 *    datasets.
 *    Each file will have 2 values, the first corresponds to the address of the
 *    frame being referenced and the second corresponds to the type of
 *    operation(Read or Write).
 *
 *        Version:  1.0
 *
 *        Created:  04/06/2022 09:05:39 PM
 *
 *       Revision:  none
 *
 *       Compiler:  gcc
 *
 *         Author:
 *
 *         Email:
 *
 *
 *
 * =====================================================================================
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// A linked list node
struct Node {
    int address;
    char op;
    struct Node* next;
};

int SIZE = (int)15e8;
int HashTable[(int)15e8];
int FIFO_IX = 0;

int hashCode(int key) { return key % 10; }

int get(int key) {
    int hashindex = hashCode(key);
    if (hashindex < SIZE) {
        return HashTable[hashindex];
    }
    return 0;
}

void insert(int address, int value) {
    int hashindex = hashCode(address);
    HashTable[hashindex] = value;
}

int delete (int item) {
    int hashindex = hashCode(item);
    int tmp = HashTable[hashindex];
    HashTable[hashindex] = -1;
    return tmp;
}

void usage(char* prog) {
    printf("Example Usage:\n");
    printf("%s  <trace file> <nframes> <fifo|lru>\n", prog);
}

/* Read characters from 'fd' until a newline is encountered. If a newline
  character is not encountered in the first (n - 1) bytes, then the excess
  characters are discarded. The returned string placed in 'buf' is
  null-terminated and includes the newline character if it was read in the
  first (n - 1) bytes. The function return value is the number of bytes
  placed in buffer (which includes the newline character if encountered,
  but excludes the terminating null byte). */
ssize_t readLine(int fd, void* buffer, size_t n) {
    ssize_t numRead;
    size_t totRead;
    char* buf;
    char ch;
    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }
    buf = buffer;
    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);
        if (numRead == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        } else if (numRead == 0) {
            if (totRead == 0) {
                return 0;
            } else {
                break;
            }
        } else {
            if (totRead < n - 1) {
                totRead++;
                *buf++ = ch;
            }
            if (ch == '\n') {
                break;
            }
        }
    }
    *buf = '\0';
    return totRead;
}

// function trims leading and trailing whitespaces
void trim_whitespace(char* str) {
    int i;
    int begin = 0;

    int end = strlen(str) - 1;

    while (isspace((unsigned char)str[begin])) begin++;

    while ((end >= begin) && isspace((unsigned char)str[end])) end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++) str[i - begin] = str[i];

    str[i - begin] = '\0';  // Null terminate string.
}

int main(int argc, char* argv[]) {
    int BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    char operation;
    char* str_address;
    int fd, numRead, address;
    int nReads = 0, nWrites = 0;
    int pageFrames, totalItems = 0;
    int first_index = 0;

    if (argc < 4 || strcmp(argv[1], "--help") == 0) {
        usage(argv[0]);
        exit(0);
    }
    pageFrames = (int)strtol(argv[2], NULL, 10);
    struct Node CurrentFrame[pageFrames];
    for (int i = 0; i < SIZE; i++) {
        HashTable[i] = -1;
    }
    for (int i = 0; i < pageFrames; i++) {
        struct Node tmp = {.address = -1, .op = 0};
        CurrentFrame[i] = tmp;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        printf("Failed to open %s\n", argv[1]);
        exit(1);
    }
    // algorithm
    if ((strcmp(argv[3], "fifo")) == 0) {
        while ((numRead = readLine(fd, buf, BUF_SIZE)) > 0) {
            operation = buf[strlen(buf) - 2];
            trim_whitespace(buf);
            buf[strlen(buf) - 1] = '\0';
            str_address = strtok(buf, " ");
            address = (int)strtol(str_address, NULL, 16);
            /* printf("%x ---> %c\n", address, operation); */
            totalItems++;
            /* insert(address, operation); */
            struct Node tmp = {
                .address = address, .op = operation, .next = NULL};
            for (int i = 0; i < pageFrames; i++) {
                if (CurrentFrame[i].address == address) {
                    break;
                }
                if (CurrentFrame[i].address == -1) {
                    CurrentFrame[i] = tmp;
                    if (operation == 'R') {
                        nReads++;
                    } else if (operation == 'W') {
                        nWrites++;
                    }
                    break;
                }
                if ((CurrentFrame[first_index].address > -1) &&
                    first_index + 1 < pageFrames) {
                    first_index++;
                }
            }
            if (first_index + 1 > pageFrames) {
                first_index = pageFrames - 1;
            }
            int exists = 0;
            for (int i = 0; i < pageFrames; i++) {
                if (CurrentFrame[i].address == address) {
                    exists = 1;
                }
            }
            if (!exists) {
                if (first_index == pageFrames) {
                    first_index = 0;
                }
                CurrentFrame[first_index] = tmp;
                if (operation == 'R') {
                    nReads++;
                } else if (operation == 'W') {
                    nWrites++;
                }
            }
            for (int i = 0; i < pageFrames; i++) {
                printf("%x\t", CurrentFrame[i].address);
            }
            printf("\n");
        }
    } else if ((strcmp(argv[3], "lru")) == 0) {
        while ((numRead = readLine(fd, buf, BUF_SIZE)) > 0) {
            operation = buf[strlen(buf) - 2];
            trim_whitespace(buf);
            buf[strlen(buf) - 1] = '\0';
            str_address = strtok(buf, " ");
            address = (int)strtol(str_address, NULL, 16);
            /* printf("%x ---> %c\n", address, operation); */
            totalItems++;
            struct Node tmp = {
                .address = address, .op = operation, .next = NULL};
            if (get(address) > -1) {
                insert(address, get(address) + 1);
            } else {
                insert(address, 1);
            }
            // get LRU
            int min = get(CurrentFrame[0].address);
            int cix = 0;
            for (int i = 0; i < pageFrames; i++) {
                if (CurrentFrame[i].address == address) {
                    break;
                }
                if (CurrentFrame[i].address == -1) {
                    CurrentFrame[i] = tmp;
                    if (operation == 'R') {
                        nReads++;
                    } else if (operation == 'W') {
                        nWrites++;
                    }
                    break;
                }
                if (get(CurrentFrame[i].address) < min && min != -1) {
                    cix = i;
                    min = get(CurrentFrame[i].address);
                }
            }
            int exists = 0;
            for (int i = 0; i < pageFrames; i++) {
                if (CurrentFrame[i].address == address) {
                    exists = 1;
                }
            }
            if (!exists) {
                CurrentFrame[cix] = tmp;
                if (operation == 'R') {
                    nReads++;
                } else if (operation == 'W') {
                    nWrites++;
                }
            }
            for (int i = 0; i < pageFrames; i++) {
                printf("%x\t", CurrentFrame[i].address);
            }
            printf("\n");
        }
    } else {
        usage(argv[0]);
        exit(1);
    }
    if (numRead == -1) {
        printf("Error Reading file: %s\n", argv[1]);
    }
    if (close(fd) == -1) {
        printf("Error closing file\n");
        exit(1);
    }
    // Count number of reads and writes
    printf("Contents of page frames\n");
    for (int i = 0; i < pageFrames; i++) {
        printf("%x\t", CurrentFrame[i].address);
    }
    printf("\n");
    // get page frame content
    printf("Number of Reads: %d\n", nReads);
    printf("Number of Writes: %d\n", nWrites);
    return 0;
}
