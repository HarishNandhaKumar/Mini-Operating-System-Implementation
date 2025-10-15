/*
 * file:        part-3.c
 * description: part 3, CS5600 Project 1, 2025 SP
 */

/* NO OTHER INCLUDE FILES */
#include "elf64.h"
#include "sysdefs.h"

extern void *vector[];
extern void switch_to(void **location_for_old_sp, void *new_value);
extern void *setup_stack0(void *_stack, void *func);

char stack1[4096];
char stack2[4096];

void *sp1, *sp2, *main_sp;

/* ---------- */

/* write these
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

/* copy from Part 2 */

void do_print(char *buf) {
        while(*buf) {
                write(1, (void *)buf, 1);
                buf++;
        }
}

void *exec_file(int fd, uint64_t offset) {

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

    for (i = 0; i < hdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            int len = ROUND_UP(phdrs[i].p_memsz, 4096);
            void *buf = mmap((void *)(phdrs[i].p_vaddr + offset), len,
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
        }
    }
    return (void *)((uint64_t)hdr.e_entry + offset);
}


/* ---------- */

/* write these new functions */

void do_yield12(void) {
        switch_to(&sp1, sp2);
}

void do_yield21(void) {
        switch_to(&sp2, sp1);
}

void do_uexit(void) {
        switch_to(&sp1, main_sp);
        exit(0);
}

/* ---------- */

void main(void)
{
	vector[1] = do_print;

	vector[3] = do_yield12;
	vector[4] = do_yield21;
	vector[5] = do_uexit;

	/* your code here */
	int fd1 = open("process1", 0);
    int fd2 = open("process2", 0);

    void *entry1 = exec_file(fd1, 0x80000000);
    void *entry2 = exec_file(fd2, 0x90000000);

    close(fd1);
    close(fd2);

    // Save the main stack pointer
    asm volatile("movq %%rsp, %0" : "=r"(main_sp));

    sp1 = setup_stack0(stack1 + 4096, entry1);
    sp2 = setup_stack0(stack2 + 4096, entry2);

    if (!sp1 || !sp2) {
        do_print("Invalid stack pointer\n");
        exit(1);
    }

    switch_to(&main_sp, sp1);

	do_print("done\n");
	exit(0);
}
