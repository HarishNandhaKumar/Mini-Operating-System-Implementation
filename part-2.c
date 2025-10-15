/*
 * file:        part-2.c
 * description: Part 2, CS5600 Project 1, 2025 SP
 */

/* NO OTHER INCLUDE FILES */
#include "elf64.h"
#include "sysdefs.h"

#define MAX_ARGS 10
#define BASE_ADDR 0x80000000

char *global_argv[MAX_ARGS];
int global_argc = 0;

extern void *vector[];

/* ---------- */

/* write these functions
*/

int read(int fd, void *ptr, int len) {
        return syscall(__NR_read, fd, ptr, len);
}

int write(int fd, void *ptr, int len) {
        return syscall(__NR_write, fd, ptr, len);
}

void exit(int err) {
        syscall(__NR_exit, err);
}

int open(char *path, int flags) {
        return syscall(__NR_open, path, flags);
}

int close(int fd) {
        return syscall(__NR_close, fd);
}

int lseek(int fd, int offset, int flag) {
        return syscall(__NR_lseek, fd, offset, flag);
}

void *mmap(void *addr, int len, int prot, int flags, int fd, int offset) {
        return (void *)syscall(__NR_mmap, addr, len, prot, flags, fd, offset);
}

int munmap(void *addr, int len) {
        return syscall(__NR_munmap, addr, len);
}

/* ---------- */

/* the three 'system call' functions - readline, print, getarg
 * hints:
 *  - read() or write() one byte at a time. It's OK to be slow.
 *  - stdin is file desc. 0, stdout is file descriptor 1
 *  - use global variables for getarg
 */

void do_readline(char *buf, int len) {
       int i = 0;
       char c;

       while(i < len - 1) {
                if(read(0, &c, 1) <= 0) {
                        break;
                }

                if(c == '\n') {
                    break;
                }

                buf[i++] = c;
       }

       buf[i] = '\0';
}

void do_print(char *buf) {
        while(*buf) {
            write(1, (void *)buf, 1);
            buf++;
        }
}

char *do_getarg(int i) {
        if (i < 0 || i >= global_argc) {
              return NULL;
    }
    return global_argv[i];
}


/* ---------- */

/* the guts of part 2
 *   read the ELF header
 *   for each section, if b_type == PT_LOAD:
 *     create mmap region
 *     read from file into region
 *   function call to hdr.e_entry
 *   munmap each mmap'ed region so we don't crash the 2nd time
 */

/* your code here */

void exec_file(int fd) {

    struct elf64_ehdr hdr;
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        do_print("Failed to read ELF header\n");
        exit(1);
    }

    int i, n = hdr.e_phnum;
    struct elf64_phdr phdrs[n];
    if (lseek(fd, hdr.e_phoff, SEEK_SET) == -1) {
        do_print("Failed to seek to program headers\n");
        exit(1);
    }
    if (read(fd, phdrs, sizeof(phdrs)) != sizeof(phdrs)) {
        do_print("Failed to read program headers\n");
        exit(1);
    }

    void *allocations[n];
    int lengths[n];

    for (i = 0; i < hdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            int len = ROUND_UP(phdrs[i].p_memsz, 4096);
            void *buf = mmap((void *)(phdrs[i].p_vaddr + BASE_ADDR), len,
                             PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (buf == MAP_FAILED) {
                do_print("mmap failed\n");
                exit(1);
            }

            if (lseek(fd, phdrs[i].p_offset, SEEK_SET) == -1) {
                do_print("Failed to seek to segment\n");
                exit(1);
            }
            if (read(fd, buf, phdrs[i].p_filesz) != phdrs[i].p_filesz) {
                do_print("Failed to read segment\n");
                exit(1);
            }

            allocations[i] = buf;
            lengths[i] = len;
        }
    }

    void (*f)();
    f = (void (*)())(hdr.e_entry + BASE_ADDR);
    f();

    for (i = 0; i < hdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            munmap(allocations[i], lengths[i]);
        }
    }

    close(fd);
}


/* ---------- */

/* simple function to split a line:
 *   char buffer[200];
 *   <read line into 'buffer'>
 *   char *argv[10];
 *   int argc = split(argv, 10, buffer);
 *   ... pointers to words are in argv[0], ... argv[argc-1]
 */
int split(char **argv, int max_argc, char *line)
{
	int i = 0;
	char *p = line;

	while (i < max_argc) {
		while (*p != 0 && (*p == ' ' || *p == '\t' || *p == '\n'))
			*p++ = 0;
		if (*p == 0)
			return i;
		argv[i++] = p;
		while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\n')
			p++;
	}
	return i;
}

/* ---------- */

void main(void)
{
	vector[0] = do_readline;
    vector[1] = do_print;
    vector[2] = do_getarg;

    char buffer[200];
    char *argv[MAX_ARGS];

    while (1) {
        do_print("> ");
        do_readline(buffer, sizeof(buffer));

        global_argc = split(argv, MAX_ARGS, buffer);
        for (int i = 0; i < global_argc; i++) {
            global_argv[i] = argv[i];
        }

        if (global_argc == 0) {
            continue;
        }

        if (global_argc == 1 && global_argv[0][0] == 'q' && global_argv[0][1] == 'u'
            && global_argv[0][2] == 'i' && global_argv[0][3] == 't'
            && global_argv[0][4] == '\0') {
                exit(0);
        }

        int fd = open(global_argv[0], 0);
        if (fd < 0) {
            continue;
        }

        exec_file(fd);
    }
}

