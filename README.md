# Mini Operating System Implementation

<div align="center">

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Assembly](https://img.shields.io/badge/Assembly-654FF0?style=for-the-badge&logo=assemblyscript&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

**A minimal operating system built from scratch demonstrating core OS concepts**

[Features](#-features) • [Architecture](#-architecture) • [Getting Started](#-getting-started) • [Demo](#-demo) • [Technical Details](#-technical-details)

</div>

---

## 📖 Overview

A low-level systems programming project that implements fundamental operating system functionality including direct system calls, ELF binary loading, and cooperative multitasking - **all built without using the C standard library**.

This project progressively builds OS complexity across three components:

| Part | Component | Key Concepts |
|------|-----------|--------------|
| **1** | Bare-metal I/O | System calls, file descriptors, raw input/output |
| **2** | Program Loader | ELF parsing, memory mapping, syscall table |
| **3** | Context Switching | Stack management, thread coordination, cooperative scheduling |

## ✨ Features

### 🔧 Part 1: System Call Interface
- **Direct syscall wrappers** for `read`, `write`, `exit` using assembly
- **Custom string I/O** without `printf`, `scanf`, or any standard library
- **Interactive echo program** with command parsing

### 📦 Part 2: ELF Program Loader
- **ELF64 binary parser** - Reads and interprets executable headers
- **Dynamic loading** - Loads relocatable executables at arbitrary memory addresses (0x80000000)
- **Memory management** - `mmap`/`munmap` with 4KB page alignment
- **Custom syscall table** - Microprograms call host functions via vector table
- **Simple shell** - Parse commands, load programs, execute with arguments

**Supported Microprograms:**
```bash
wait   # CPU-intensive loop demonstration
hello  # Basic "Hello World" output
ugrep  # Pattern matching (grep-like functionality)
```

### 🔄 Part 3: Context Switching
- **Cooperative multitasking** between two threads
- **Custom stack allocation** for each execution context
- **Context switch functions** (`yield12`, `yield21`, `uexit`)
- **Assembly-level stack manipulation** for thread coordination

## 🏗️ Architecture

### Memory Layout
```
0x400000            Main program (statically linked)
                    ↓
0x80000000          Loaded microprogram (Part 2)
                    ↓
0x01000000          Process 1 (Part 3)
0x02000000          Process 2 (Part 3)
                    ↓
0x800000000000      Stack region
```

### System Call Flow
```
Microprogram → syscall(n, args) → Vector Table → Host Function → Kernel
```

### Tech Stack
- **Languages:** C (no stdlib), GNU Assembly (AT&T syntax)
- **System:** x86-64 Linux
- **Toolchain:** GCC, GNU Make, GDB, Valgrind
- **Binary Format:** ELF64
- **Key Syscalls:** read(0), write(2), open(2), close(2), lseek(8), mmap(9), munmap(11), exit(60)

## 🚀 Getting Started

### Prerequisites
```bash
# Linux x86-64 system required
gcc --version      # GCC compiler
make --version     # GNU Make
gdb --version      # (Optional) for debugging
valgrind --version # (Optional) for memory testing
```

### Build & Run
```bash
# Clone the repository
git clone https://github.com/HarishNandhaKumar/Mini-Operating-System-Implementation.git
cd mini-os

# Build Part 1 - System Call Interface
make part-1
./part-1

# Build Part 2 - Program Loader
make part-2
./part-2

# Build Part 3 - Context Switching
make part-3
./part-3

# Clean build artifacts
make clean
```

## 🎬 Demo

### Part 1: Echo Program
```bash
$ ./part-1
Hello, type lines of input, or 'quit':
> Hello from bare-metal C!
you typed: Hello from bare-metal C!
> This uses no standard library
you typed: This uses no standard library
> quit
$
```

### Part 2: Loading & Running Programs
```bash
$ ./part-2
> wait
[Program executes billion-iteration loop]
> hello
Hello world!
> ugrep test
ugrep: enter blank line to quit
this is a test line
-- this is a test line
no match here
another test here
-- another test here

> quit
$
```

### Part 3: Context Switching Between Threads
```bash
$ ./part-3
program 1
program 2
program 1
program 2
program 1
program 2
program 1
program 2
done
$
```

## 🔬 Technical Details

### Key Challenges Solved

#### 1. **No Standard Library**
All functionality implemented from scratch:
- String operations (length, comparison, copy)
- I/O buffering and line reading
- Memory operations
- Type conversions

#### 2. **ELF Binary Loading**
```c
// Parse ELF header
struct elf64_ehdr header;
read(fd, &header, sizeof(header));

// Load program headers
for (int i = 0; i < header.e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
        // Allocate memory at specific address
        void *addr = mmap(base + phdr[i].p_vaddr, 
                          ROUND_UP(phdr[i].p_memsz, 4096),
                          PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        // Read segment from file
        lseek(fd, phdr[i].p_offset, SEEK_SET);
        read(fd, addr, phdr[i].p_filesz);
    }
}
```

#### 3. **Stack Setup & Context Switching**
```c
// Setup stack for new thread
char stack1[4096];
void *sp = setup_stack0(stack1 + 4096, entry_point);

// Switch to new stack
void *old_sp;
switch_to(&old_sp, sp);  // Assembly function
```

### File Structure
```
mini-os/
├── part-1.c              # Bare I/O implementation
├── part-2.c              # Program loader & shell
├── part-3.c              # Context switching
├── syscall.S             # System call wrappers (assembly)
├── switch.S              # Stack switching (assembly)
├── vector.S              # Syscall table (assembly)
├── call-vector.S         # Vector call mechanism
├── stack.c               # Stack frame setup
├── elf64.h               # ELF header definitions
├── sysdefs.h             # System call numbers & macros
├── Makefile              # Build configuration
├── wait.c                # Microprogram: CPU loop
├── hello.c               # Microprogram: Hello world
├── ugrep.c               # Microprogram: Pattern matcher
└── README.md             # This file
```

## 🧪 Testing & Debugging

### Memory Safety Check
```bash
# Run with Valgrind to detect memory issues
valgrind --leak-check=full ./part-2
```

### GDB Debugging
```bash
gdb ./part-2
(gdb) b execfile           # Set breakpoint
(gdb) run
> ugrep test
(gdb) add-symbol-file ugrep 0x80001000  # Load microprogram symbols
(gdb) bt                   # View call stack
(gdb) display/i $pc        # Display next instruction
(gdb) stepi                # Step one instruction
```

### Inspect ELF Headers
```bash
# View program headers
readelf -l wait

# View all headers
readelf -a hello

# Disassemble microprogram
objdump -d ugrep
```

## 📚 What I Learned

- ✅ **System Programming**: Direct kernel interaction without abstractions
- ✅ **Binary Formats**: ELF file structure, program headers, entry points
- ✅ **Memory Management**: Virtual memory, page alignment, `mmap` mechanics
- ✅ **Assembly Language**: Register manipulation, stack frames, calling conventions
- ✅ **Context Switching**: Stack pointer manipulation, thread coordination
- ✅ **Debugging**: GDB for assembly-level debugging, Valgrind for memory issues
- ✅ **Build Systems**: Complex Makefiles with custom compilation flags

## 🎓 Academic Context

**Course:** Operating Systems (Graduate Level)  
**Institution:** Northeastern University  
**Semester:** Fall 2024  
**Language:** C without standard library + x86-64 Assembly

### Key Concepts Demonstrated
- Process management and execution
- Virtual memory and address spaces
- System call interface and implementation
- Binary executable format and loading
- Thread scheduling and context switching
- Stack management and calling conventions

## ⚠️ Important Notes

### Assembly Syntax
This project uses **GNU Assembly (AT&T syntax)**, where:
```asm
mov src, dst    # AT&T syntax (used here)
mov dst, src    # Intel syntax (typically taught)
```

## 🔗 Resources

- [GNU Assembly Syntax](https://en.wikibooks.org/wiki/X86_Assembly/GAS_Syntax)
- [Linux System Calls](https://linux.die.net/man/)
- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)
- [x86-64 ABI](https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf)
- [AT&T vs Intel Syntax](https://en.wikipedia.org/wiki/X86_assembly_language#Syntax)


## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

Educational project for portfolio demonstration.

---

<div align="center">

[⬆ Back to Top](#mini-operating-system-implementation)

</div>
