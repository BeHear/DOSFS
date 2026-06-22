# DanyaOS v1.1.0

A hobby microkernel operating system written in C and x86 assembly.

## Architecture

- **Microkernel design** - minimal kernel with userspace drivers
- **x86 (i386)** target architecture
- **Preemptive multitasking** scheduler
- **Virtual memory** with paging
- **IPC** (Inter-Process Communication)
- **System calls** via INT 0x80
- **tmpfs** in-memory filesystem
- **Interactive shell** with built-in commands

## Subsystems

| Module | Description |
|--------|-------------|
| GDT | Global Descriptor Table (Ring 0-3) |
| IDT | Interrupt Descriptor Table |
| ISR/IRQ | Interrupt handlers |
| PMM | Physical Memory Manager (bitmap) |
| VMM | Virtual Memory Manager (paging) |
| Heap | Dynamic memory allocator |
| VGA | Text-mode terminal driver |
| Keyboard | PS/2 keyboard driver |
| Timer | PIT timer (100Hz) with preemptive scheduling |
| Scheduler | Round-robin process scheduler |
| IPC | Message-passing IPC |
| Syscalls | INT 0x80 syscall interface |
| tmpfs | In-memory filesystem |
| Shell | Interactive command shell |

## Shell Commands

```
help              - show available commands
clear             - clear screen
echo <msg>        - print message
uname             - system info
mem               - memory info
uptime            - timer ticks
ps                - list processes
create <name>     - create process
ipc               - test IPC
ls                - list files
touch <file>      - create file
write <file> <data> - write to file
cat <file>        - read file
rm <file>         - delete file
color <fg> <bg>   - set terminal colors
reboot            - reboot
```

## Building

### Prerequisites

- `i686-elf-gcc` cross-compiler
- `nasm` assembler
- `qemu-system-i386` (for testing)

### Install cross-compiler (Debian/Ubuntu)

```bash
sudo apt install gcc-i686-linux-gnu nasm qemu-system-x86
```

Or build from source: https://wiki.osdev.org/GCC_Cross-Compiler

### Build

```bash
make
```

### Run

```bash
make run
```

## Project Structure

```
DanyaOS/
├── Makefile
├── linker.ld
├── src/
│   ├── boot/           # Bootloader (MBR, protected mode)
│   ├── kernel/         # Core kernel (GDT, IDT, ISR, kernel_main)
│   ├── drivers/        # Device drivers (VGA, keyboard, timer)
│   ├── memory/         # Memory management (PMM, VMM, heap)
│   ├── process/        # Process management & scheduler, IPC
│   ├── syscall/        # System call interface
│   ├── fs/             # Filesystem (tmpfs)
│   ├── shell/          # Interactive shell
│   ├── libc/           # Minimal C library
│   └── include/        # Common headers (types, I/O)
└── build/              # Build output
```

## Memory Map

```
0x00000000 - 0x000FFFFF  Kernel (1MB)
0x00100000 - 0x003FFFFF  Kernel heap
0x00400000 - 0x00FFFFFF  Kernel heap (expandable)
0xB8000000               VGA text buffer
0xBFFFF000               User stack
0xC0000000               Kernel virtual base
```

## Syscall Table

| # | Name | Arguments |
|---|------|-----------|
| 1 | write | EBX=buf, ECX=len |
| 2 | read | EBX=buf, ECX=max |
| 3 | exit | EBX=code |
| 6 | getpid | - |
| 7 | ipc_send | EBX=to, ECX=data, EDX=len |
| 8 | ipc_recv | EBX=from, ECX=buf, EDX=max |
| 9 | sleep | - |
| 10 | meminfo | returns free/total in EAX/EBX |

## License

MIT License

## Author

DanyaOS Project (2025)
