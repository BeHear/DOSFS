# DanyaOS v1.2.1

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
- **TUI** (Text User Interface) with menu system

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
| TUI | Text user interface with menus |
| CPUID | CPU identification and feature detection |

## Shell Commands

```
help              - show available commands
clear/cls         - clear screen
echo <msg>        - print message
uname             - system info
mem/free          - memory info
uptime            - timer ticks
ps                - list processes
create <name>     - create process
ipc               - test IPC
ls                - list files
touch <file>      - create file
write <file> <data> - write to file
cat <file>        - read file
rm <file>         - delete file
cp <src> <dst>    - copy file
mv <src> <dst>    - move/rename file
hexdump <file>    - hex dump of file
color <fg> <bg>   - set terminal colors
date              - system uptime
whoami            - current user
pwd               - current directory
calc <a> <op> <b> - calculator (+ - * / %%)
history           - command history
reset             - reset terminal
beep              - PC speaker beep
about             - about DanyaOS
tuitest           - TUI demo
shutdown          - shutdown
reboot            - reboot
cpuinfo           - CPU information (vendor, brand, features)
```

## Building

### Prerequisites

- `gcc` (system compiler with `-ffreestanding` support)
- `nasm` assembler
- `qemu-system-i386` (for testing)

### Install dependencies (Debian/Ubuntu)

```bash
sudo apt install gcc nasm qemu-system-x86
```

### Build

```bash
make
```

### Run

```bash
make run
```

### Debug

```bash
make debug
```

## Project Structure

```
DanyaOS/
├── Makefile
├── linker.ld
├── src/
│   ├── boot/           # Bootloader (MBR, protected mode)
│   ├── kernel/         # Core kernel (GDT, IDT, ISR, kernel_main)
│   ├── drivers/        # Device drivers (VGA, keyboard, timer, serial)
│   ├── memory/         # Memory management (PMM, VMM, heap)
│   ├── process/        # Process management & scheduler, IPC
│   ├── syscall/        # System call interface
│   ├── fs/             # Filesystem (tmpfs)
│   ├── shell/          # Interactive shell
│   ├── tui/            # Text user interface
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
