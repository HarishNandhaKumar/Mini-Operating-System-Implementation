/*
 * file:        part-1.c
 * description: Part 1, CS5600 Project 1, 2025 SP
 */

/* THE ONLY INCLUDE FILE */
#include "sysdefs.h"

/* write these functions */

int read(int fd, void *ptr, int len) {
       return syscall(__NR_read, fd, ptr, len);
}

int write(int fd, void *ptr, int len) {
       return syscall(__NR_write, fd, ptr, len);
}

void exit(int err) {
       syscall(__NR_exit, err);
}

void readline(char *buffer, int max_string_len) {
       int i = 0;
       char c;

       while(i < max_string_len - 1) {
                if(read(0, &c, 1) <= 0) {
                        break;
                }

                if(c == '\n') {
                    break;
                }

                buffer[i++] = c;
       }

       buffer[i] = '\0';
}

void print(const char *str) {
        while(*str) {
            write(1, (void *)str, 1);
            str++;
        }
}


/* ---------- */

/* Factor, factor! Don't put all your code in main()!
*/

/* read one line from stdin (file descriptor 0) into a buffer: */

/* print a string to stdout (file descriptor 1) */

/* ---------- */

void main(void)
{
	/* your code here */

    print("Hello, type lines of input, or 'quit':\n");

	char buffer[201];

	while(1) {
            print("> ");
            readline(buffer, sizeof(buffer));

            if (buffer[0]== 'q' && buffer[1] == 'u' && buffer[2] == 'i' && buffer[3] == 't' && buffer[4] == '\0') {
                exit(0);
            }

            print("you typed: ");
            print(buffer);
            print("\n");
	}
}
