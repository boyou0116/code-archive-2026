# ELF Sections and Common Inspection Commands

This note summarizes common ELF sections produced from C programs, such as `.text`, `.rodata`, `.data`, and `.bss`, along with useful commands such as `readelf`, `nm`, and `objdump`.

---

## 1. Basic Flow from C to ELF

```text
main.c  --compile-->  main.o  --link-->  main.elf
```

Common commands:

```bash
gcc -c main.c -o main.o
ld -T simple.ld main.o -o main.elf
```

Or link with `gcc`:

```bash
gcc main.o -o main.elf
```

To generate a map file:

```bash
gcc main.o -Wl,-Map=main.map -o main.elf
```

---

## 2. Difference Between `main.o` and `main.elf`

| Item                 | `main.o`                        | `main.elf`                                |
|----------------------|---------------------------------|-------------------------------------------|
| Stage                | After compilation               | After linking                             |
| Complete program?    | No                              | Usually yes                               |
| Executable?          | Usually no                      | Usually yes                               |
| Section addresses    | Final addresses not decided yet | Decided by the linker                     |
| Relocations          | Usually still present           | Usually resolved                          |
| Symbols              | May still be unresolved         | Resolved or handled by the dynamic linker |
| Linker script impact | Usually little direct impact    | Main target of linker script control      |

`main.o` is an object file. It contains machine code, symbols, and relocations, but it does not yet have a complete final layout.

`main.elf` is the output produced by the linker. Its sections and symbols have been arranged into a final layout.

---

## 3. Common ELF Sections

### `.text`

`.text` contains program code, also known as machine instructions.

Example:

```c
int main(void)
{
    return 0;
}
```

The machine code generated from `main()` is usually placed in `.text`.

Common flags:

```text
AX
```

| Flag | Meaning                              |
|------|--------------------------------------|
| `A`  | Alloc: loaded into memory at runtime |
| `X`  | Executable                           |

Useful commands:

```bash
readelf -S main.elf
objdump -d main.elf
objdump -d -j .text main.elf
```

---

### `.rodata`

`.rodata` contains read-only data, such as string literals, read-only constants, and `const` data.

Example:

```c
const char msg[] = "456";
```

This is usually placed in `.rodata`.

Common flags:

```text
A
```

This means the section is loaded into memory at runtime but is not writable.

Useful commands:

```bash
readelf -S main.elf
objdump -s -j .rodata main.elf
nm -n main.elf
```

---

### `.data`

`.data` contains global or static variables that have explicit non-zero initial values and are writable at runtime.

Example:

```c
int global_init = 123;

int main(void)
{
    static int static_init = 456;
    return 0;
}
```

These are usually placed in `.data`.

Common flags:

```text
WA
```

| Flag | Meaning                              |
|------|--------------------------------------|
| `W`  | Writable                             |
| `A`  | Alloc: loaded into memory at runtime |

Useful commands:

```bash
readelf -S main.elf
objdump -s -j .data main.elf
nm -n main.elf
```

In embedded or bare-metal systems, `.data` has an important property:

```text
VMA: RAM
LMA: FLASH
```

This means:

- At runtime, `.data` lives in RAM.
- Its initial values are stored in Flash.
- Startup code copies `.data` from Flash to RAM before entering `main()`.

Common linker script pattern:

```ld
.data :
{
    _sdata = .;
    *(.data*)
    _edata = .;
} > RAM AT > FLASH
```

---

### `.bss`

`.bss` contains global or static variables that are uninitialized or initialized to zero.

Example:

```c
int global_uninit;
int global_zero = 0;

int main(void)
{
    static int static_uninit;
    return 0;
}
```

These are usually placed in `.bss`.

The section type of `.bss` is usually:

```text
NOBITS
```

This means `.bss` does not store actual data in the ELF file. It only records how much memory must be allocated at runtime.

Common flags:

```text
WA
```

Useful commands:

```bash
readelf -S main.elf
nm -n main.elf
```

Note: because `.bss` is `NOBITS`, it usually has no raw file content to dump with `objdump -s`.

In embedded or bare-metal systems, startup code clears `.bss` to zero before entering `main()`.

Common linker script pattern:

```ld
.bss :
{
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
} > RAM
```

---

### `.init_array`

`.init_array` contains pointers to functions that should run during program initialization.

Common uses include:

- C++ global constructors
- libc runtime initialization
- compiler-generated initialization routines

Useful commands:

```bash
readelf -S main.elf
objdump -s -j .init_array main.elf
```

---

### `.fini_array`

`.fini_array` contains pointers to functions that should run during program termination.

Common uses include:

- C++ destructors
- libc cleanup routines

Useful commands:

```bash
readelf -S main.elf
objdump -s -j .fini_array main.elf
```

---

### `.eh_frame` / `.eh_frame_hdr`

These sections are related to exception handling, stack unwinding, and backtraces.

Even C programs may contain them because of default toolchain or runtime settings.

Useful command:

```bash
readelf --debug-dump=frames main.elf
```

When first learning linker scripts, it is enough to know that these sections exist. You do not need to study them deeply at the beginning.

---

### `.symtab`

`.symtab` is the symbol table. It records symbols such as:

- `main`
- `global_init`
- `global_uninit`
- `msg`
- `static_init`
- `static_uninit`

Useful commands:

```bash
readelf -s main.elf
nm -n main.elf
```

---

### `.strtab`

`.strtab` is the symbol string table.

Symbol names used by `.symtab` are usually stored in `.strtab`.

You usually do not inspect it manually, but it is useful to know that it works together with `.symtab`.

---

### `.shstrtab`

`.shstrtab` is the section header string table.

It stores section names such as:

```text
.text
.rodata
.data
.bss
.symtab
.strtab
```

---

### `.interp`

`.interp` appears in Linux dynamically linked executables.

It records the path of the dynamic linker / loader, for example:

```text
/lib64/ld-linux-x86-64.so.2
```

If an ELF file contains `.interp`, it is usually a Linux executable rather than a bare-metal firmware image.

Useful command:

```bash
readelf -p .interp main.elf
```

---

### `.dynamic`

`.dynamic` contains dynamic linking metadata.

The Linux dynamic linker uses it to handle:

- shared libraries
- relocations
- symbol resolution
- init/fini routines

Useful command:

```bash
readelf -d main.elf
```

---

### `.dynsym` / `.dynstr`

`.dynsym` is the dynamic symbol table.

`.dynstr` is the dynamic symbol string table.

They are mainly used for dynamic linking.

Useful command:

```bash
readelf --dyn-syms main.elf
```

---

### `.plt`

`.plt` stands for Procedure Linkage Table.

It commonly appears in dynamically linked executables and is used to call functions from shared libraries.

For example, a call to `printf()` may go through `.plt`.

Useful command:

```bash
objdump -d -j .plt main.elf
```

---

### `.got`

`.got` stands for Global Offset Table.

It is commonly related to dynamic linking and position-independent code / PIE.

Useful commands:

```bash
readelf -S main.elf
objdump -s -j .got main.elf
```

---

### `.rela.dyn` / `.rel.dyn`

These sections contain relocation entries.

On x86-64 Linux, `.rela.dyn` is common.

Useful command:

```bash
readelf -r main.elf
```

---

## 4. Mapping Your C Program to Sections

Example program:

```c
int global_init = 123;
int global_uninit;

const char msg[] = "456";

int main(void)
{
    static int static_init = 456;
    static int static_uninit;

    global_uninit = global_init + static_init;
    return 0;
}
```

Typical mapping:

| C object        | Description                                          | Typical section | `nm` type |
|-----------------|------------------------------------------------------|-----------------|-----------|
| `main`          | Function code                                        | `.text`         | `T`       |
| `global_init`   | Global variable with an initial value                | `.data`         | `D`       |
| `global_uninit` | Uninitialized global variable                        | `.bss`          | `B`       |
| `msg`           | `const char[]`                                       | `.rodata`       | `R`       |
| `static_init`   | Function-local static variable with an initial value | `.data`         | `d`       |
| `static_uninit` | Function-local static uninitialized variable         | `.bss`          | `b`       |

Case difference in `nm` symbol types:

| Case      | Meaning                  |
|-----------|--------------------------|
| Uppercase | Global / external symbol |
| Lowercase | Local / internal symbol  |

Therefore, function-local `static` variables usually appear as lowercase symbols:

```text
d static_init.0
b static_uninit.1
```

---

## 5. Common Commands

### View Section Headers

```bash
readelf -S main.elf
```

Important columns:

| Column    | Meaning                          |
|-----------|----------------------------------|
| `Name`    | Section name                     |
| `Type`    | Section type                     |
| `Address` | Runtime address                  |
| `Offset`  | Offset inside the ELF file       |
| `Size`    | Section size                     |
| `Flags`   | Section permissions / attributes |
| `Align`   | Alignment requirement            |

---

### View Symbols

```bash
nm -n main.elf
```

Show only your symbols:

```bash
nm -n main.elf | grep -E 'main|global|static|msg'
```

Show symbol sizes:

```bash
nm -S -n main.elf
```

Sort by symbol size:

```bash
nm -S --size-sort main.elf
```

Show only undefined symbols:

```bash
nm -u main.o
```

---

### View Section Summary

```bash
objdump -h main.elf
```

This is more compact than `readelf -S` and is useful for quickly checking section sizes and addresses.

---

### Disassemble `.text`

```bash
objdump -d main.elf
```

Only disassemble `.text`:

```bash
objdump -d -j .text main.elf
```

---

### Dump Raw Section Contents

Dump `.rodata`:

```bash
objdump -s -j .rodata main.elf
```

Dump `.data`:

```bash
objdump -s -j .data main.elf
```

`.bss` usually cannot be dumped as raw content because it is `NOBITS`.

---

### View Relocations

```bash
readelf -r main.o
readelf -r main.elf
```

`main.o` usually still contains relocation entries.

`main.elf` usually has most relocations resolved by the linker. If it is a dynamic executable, it may still contain dynamic relocations.

---

### View ELF Header

```bash
readelf -h main.elf
```

This shows:

- ELF class
- machine architecture
- entry point
- section header offset
- program header offset

---

### View Program Headers

```bash
readelf -l main.elf
```

`readelf -S` shows sections.

`readelf -l` shows segments / program headers.

The Linux loader mainly uses program headers, not section headers, when loading a program.

---

### View Dynamic Linking Information

```bash
readelf -d main.elf
```

If the ELF file is dynamically linked, this shows dynamic section entries.

---

### View the Map File

Generate a map file during linking:

```bash
gcc main.o -Wl,-Map=main.map -o main.elf
```

Or:

```bash
ld -T simple.ld main.o -Map=main.map -o main.elf
```

View it:

```bash
less main.map
```

A map file shows:

- output section layout
- how input sections are collected
- final symbol addresses
- memory region usage

---

## 6. Common `nm` Symbol Types

| Type | Related section | Meaning |
|---|---|---|
| `T` | `.text` | Global function / code |
| `t` | `.text` | Local function / code |
| `R` | `.rodata` | Global read-only data |
| `r` | `.rodata` | Local read-only data |
| `D` | `.data` | Global initialized data |
| `d` | `.data` | Local initialized data |
| `B` | `.bss` | Global uninitialized / zero data |
| `b` | `.bss` | Local uninitialized / zero data |
| `U` | none | Undefined symbol |
| `A` | absolute | Absolute symbol |

---

## 7. Common `readelf -S` Flags

| Flag | Meaning |
|---|---|
| `W` | Writable |
| `A` | Alloc |
| `X` | Executable |
| `M` | Merge |
| `S` | Strings |
| `I` | Info |
| `L` | Link order |
| `G` | Group |
| `T` | TLS |
| `C` | Compressed |
| `E` | Exclude |

Most common combinations:

| Flags | Common section | Meaning |
|---|---|---|
| `AX` | `.text` | Loaded into memory and executable |
| `A` | `.rodata` | Loaded into memory and not writable |
| `WA` | `.data`, `.bss` | Loaded into memory and writable |

---

## 8. Minimal Linker Script Example

This version is useful for learning section layout:

```ld
ENTRY(main)

SECTIONS
{
    . = 0x10000;

    .text :
    {
        _stext = .;
        *(.text*)
        _etext = .;
    }

    . = ALIGN(4);

    .rodata :
    {
        _srodata = .;
        *(.rodata*)
        _erodata = .;
    }

    . = ALIGN(4);

    .data :
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    }

    . = ALIGN(4);

    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    }
}
```

Usage:

```bash
gcc -c main.c -o main.o
ld -T simple.ld main.o -Map=main.map -o main.elf
readelf -S main.elf
nm -n main.elf
```

---

## 9. Embedded Linker Script Concept Example

```ld
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS
{
    .text :
    {
        _stext = .;
        *(.isr_vector)
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}
```

Key syntax:

| Syntax | Meaning |
|---|---|
| `MEMORY` | Defines memory regions |
| `> FLASH` | Places the section VMA in FLASH |
| `> RAM` | Places the section VMA in RAM |
| `AT > FLASH` | Places the section LMA in FLASH |
| `LOADADDR(.data)` | Gets the load address of `.data` |
| `ALIGN(4)` | Aligns to a 4-byte boundary |
| `KEEP()` | Prevents a section from being removed by garbage collection |

---

## 10. VMA vs LMA

| Name | Meaning |
|---|---|
| VMA | Virtual Memory Address: runtime address |
| LMA | Load Memory Address: load-time address |

In MCU firmware, `.data` is the classic example:

```text
.data VMA = RAM
.data LMA = FLASH
```

Reason:

```c
int global_init = 123;
```

`global_init` must be writable at runtime, so it must live in RAM.

However, the initial value `123` must be stored persistently, so it is stored in Flash first.

Startup code usually does:

```text
copy .data from FLASH to RAM
clear .bss to zero
```

---

## 11. Suggested Inspection Workflow

### Step 1: Inspect the Object File

```bash
gcc -c main.c -o main.o
readelf -S main.o
nm main.o
readelf -r main.o
```

Key points:

- `main.o` already contains `.text`, `.rodata`, `.data`, and `.bss`.
- Addresses are usually not final yet.
- Relocations may still exist.

---

### Step 2: Link into an ELF File

```bash
ld -T simple.ld main.o -Map=main.map -o main.elf
```

---

### Step 3: Inspect Section Layout

```bash
readelf -S main.elf
objdump -h main.elf
```

---

### Step 4: Inspect Symbols

```bash
nm -n main.elf
nm -S -n main.elf
```

---

### Step 5: Inspect Section Contents

```bash
objdump -d -j .text main.elf
objdump -s -j .rodata main.elf
objdump -s -j .data main.elf
```

---

### Step 6: Inspect the Map File

```bash
less main.map
```

---

## 12. One-Sentence Summary

The four most important sections to understand first are:

| Section | Contains |
|---|---|
| `.text` | Program code |
| `.rodata` | Read-only data |
| `.data` | Initialized writable global/static variables |
| `.bss` | Uninitialized or zero-initialized global/static variables |

The three most important tools are:

| Command | Purpose |
|---|---|
| `readelf -S` | Inspect sections |
| `nm -n` | Inspect symbols |
| `objdump -h/-d/-s` | Inspect section summary, disassembly, and raw section contents |
