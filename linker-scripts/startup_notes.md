# CH32V Startup File Notes

本文整理 `startup_ch32v30x_D8C.S` 與 `Link.ld` 之間的關係，重點放在 startup 檔的語法、section/symbol 概念、vector table，以及 `handle_reset` 的開機流程。

## 整體開機流程

```text
Reset
  -> _start
  -> handle_reset
  -> 設定 gp
  -> 設定 sp
  -> 複製 .data: Flash -> RAM
  -> 清空 .bss
  -> 設定 CPU / interrupt CSR
  -> 設定 mtvec = _vector_base | mode
  -> SystemInit()
  -> main()
```

`_start` 是 linker script 指定的 entry symbol：

```ld
ENTRY(_start)
```

但 `_start` 不是 section，它是 label / symbol，也就是某個位址的名字。

## Section 與 Symbol

Assembly 裡：

```asm
.section    .init, "ax", @progbits
.global     _start
.align      1

_start:
    j           handle_reset
```

這段的關係是：

```text
.init section
  _start label
    j handle_reset
```

`.section .init` 是建立或切換到 `.init` input section。

`_start:` 是 label，代表目前位置。

`.global _start` 讓 `_start` 對外可見，linker script 的 `ENTRY(_start)` 才能找到它。

`.align 1` 在 RISC-V GNU assembler 裡通常表示對齊到 `2^1 = 2 bytes`。因為這個 startup 目標支援 RVC compressed instruction，指令入口至少 2-byte 對齊即可。

## `"ax", @progbits`

```asm
.section .init, "ax", @progbits
```

`"ax"` 是 section flags：

```text
a = alloc       執行時會佔有記憶體位址
x = executable 這段內容可執行
```

`@progbits` 表示這個 section 在 object/ELF 檔中有實際 bytes 內容。

例如 `.text`、`.init`、`.data` 通常是 `PROGBITS`。

`.bss` 通常是 `NOBITS`，因為它只在 RAM 裡保留空間，不需要在檔案中存一堆 0。

## `.global`、`.weak` 與 local symbol

`_start` 使用：

```asm
.global _start
```

原因是 `_start` 要給 linker script / debugger / 外部工具看見。

`handle_reset` 使用：

```asm
.weak handle_reset
```

原因是 startup 提供一個預設 reset handler，但允許其他 object file 提供 strong `handle_reset` 來覆蓋它。

`_vector_base` 沒有 `.global` 或 `.weak`，因為它目前只在同一份 assembly 裡使用：

```asm
la          t0, _vector_base
```

如果外部 C 檔或 linker script 也需要引用 `_vector_base`，才需要加：

```asm
.global _vector_base
```

## `_vector_base`

`_vector_base` 是 vector table 的起點 symbol：

```asm
.section    .vector, "ax", @progbits
.align      1

_vector_base:
    .option     norvc
    .word       _start
    .word       0
    .word       NMI_Handler
    .word       HardFault_Handler
    ...
```

它不是 section。真正的 section 是 `.vector`。

可以把 `_vector_base` 想成 C 裡的 function pointer table：

```c
typedef void (*Handler)(void);

const Handler vector_table[] = {
    _start,
    0,
    NMI_Handler,
    HardFault_Handler,
};
```

每一個：

```asm
.word HandlerName
```

都會放入一個 32-bit handler address。

## `.word` 與 `.weak`

`.word` 會產生資料並佔空間：

```asm
.word NMI_Handler
```

意思是把 `NMI_Handler` 最終解析後的地址放到目前位置。

`.weak` 不產生機器碼，也不佔執行時記憶體：

```asm
.weak NMI_Handler
```

它只是告訴 linker：這個 symbol 是 weak definition，可以被其他 strong definition 覆蓋。

所以：

```asm
.word       NMI_Handler
...
.weak       NMI_Handler

NMI_Handler:
1:
    j       1b
```

意思是：

```text
vector table 裡放 NMI_Handler 的最終地址。
如果使用者沒有定義 NMI_Handler，就用 startup 的預設版本。
```

## Default Handler Infinite Loop

startup 裡有很多 handler label 疊在一起：

```asm
NMI_Handler:
HardFault_Handler:
Ecall_M_Mode_Handler:
...
DMA2_Channel11_IRQHandler:
1:
    j           1b
```

label 本身不佔空間。因為中間沒有任何指令，所以這些 label 的地址都相同：

```text
NMI_Handler = HardFault_Handler = ... = DMA2_Channel11_IRQHandler = 1
```

`1:` 是 local numeric label。

`1b` 表示 backward 找最近的 `1:`。

所以：

```asm
1:
    j 1b
```

等價於：

```c
while (1) {
}
```

## `.option norvc`

```asm
.option norvc
...
.option rvc
```

`norvc` 表示暫時不要產生 RVC compressed instruction。

這份 vector table 主要是 `.word`，本身不會被壓成 16-bit 指令。不過在 vector 區域關掉 RVC 是防守性寫法，可以避免未來有人把 vector entry 改成指令形式，例如 `j Handler`，結果被壓成 16-bit，破壞固定 entry 大小。

更嚴謹的寫法可以用：

```asm
.option push
.option norvc
...
.option pop
```

這樣會恢復原本 option 狀態，而不是假設原本一定開著 RVC。

## `handle_reset`

`handle_reset` 是 reset 後真正初始化 C 執行環境的地方：

```asm
.section    .text.handle_reset, "ax", @progbits
.weak       handle_reset
.align      1

handle_reset:
```

它放在 `.text.handle_reset` input section。linker script 裡：

```ld
.text :
{
    *(.text)
    *(.text.*)
} >FLASH AT>FLASH
```

會把它收進 output `.text`，最後放到 Flash。

## 設定 `gp`

```asm
.option     push
.option     norelax
la          gp, __global_pointer$
.option     pop
```

`gp` 是 RISC-V global pointer，暫存器 x3。

它主要用於快速存取 small data，例如 `.sdata`、`.sbss`。

`__global_pointer$` 是 linker script 定義的慣用 RISC-V symbol：

```ld
PROVIDE(__global_pointer$ = . + 0x800);
```

`$` 是 symbol 名字的一部分，不是運算子。

`+ 0x800` 的依據是 RISC-V load/store 的 immediate offset 是 12-bit signed：

```text
-2048 bytes
-0x800 ~ +0x7ff
```

把 `gp` 設在 small data 區附近偏中間的位置，可以讓前後約 4KB 內的資料用 gp-relative addressing。

`norelax` 是因為這行正在初始化 `gp` 本身，不能讓 linker relaxation 把它最佳化成依賴 `gp` 已經有效的形式。

`push/pop` 比單純：

```asm
.option norelax
...
.option relax
```

更安全，因為它會恢復原本狀態，而不是假設原本一定是 relax。

## 設定 `sp`

```asm
la          sp, _eusrstack
```

`sp` 是 stack pointer。

linker script 中：

```ld
.stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size :
{
    PROVIDE(_heap_end = .);
    . = ALIGN(4);
    PROVIDE(_susrstack = .);
    . = . + __stack_size;
    PROVIDE(_eusrstack = .);
} >RAM
```

`_susrstack` 大致是 start user stack。

`_eusrstack` 大致是 end user stack。

RISC-V stack 通常往低位址成長，所以初始 `sp` 放在 stack 高位址端：

```text
sp = _eusrstack
```

以 RAM `0x20000000`、大小 `128K`、stack size `0x800` 為例：

```text
_susrstack = 0x2001F800
_eusrstack = 0x20020000
```

## 複製 `.data`

```asm
la          a0, _data_lma
la          a1, _data_vma
la          a2, _edata
bgeu        a1, a2, 2f

1:
    lw      t0, (a0)
    sw      t0, (a1)
    addi    a0, a0, 4
    addi    a1, a1, 4
    bltu    a1, a2, 1b
```

三個 symbol：

```text
_data_lma = .data 初始值在 Flash 的來源地址
_data_vma = .data 執行時在 RAM 的目的地址
_edata    = .data 在 RAM 的結束地址
```

`.data` 是有初始值、但執行時可改的變數，例如：

```c
int global_init = 123;
static int x = 456;
```

初始值存在 Flash，但執行時變數要在 RAM，所以 startup 要複製：

```text
Flash [_data_lma, ...] -> RAM [_data_vma, _edata)
```

等價 C：

```c
uint32_t *src = (uint32_t *)&_data_lma;
uint32_t *dst = (uint32_t *)&_data_vma;
uint32_t *end = (uint32_t *)&_edata;

while (dst < end) {
    *dst++ = *src++;
}
```

## VMA 與 LMA

`.data` 在 linker script 中通常寫成：

```ld
.data :
{
    ...
} >RAM AT>FLASH
```

意思是：

```text
VMA = RAM    執行時位址
LMA = FLASH  載入/燒錄位址
```

在 linker script 裡，`.` 代表目前 output section 的 VMA location counter。

所以在：

```ld
.data : { ... } >RAM AT>FLASH
```

`.data` 裡的 `.` 是 RAM 位址，不是 Flash 位址。

如果要取得 `.data` 的 Flash LMA，較清楚的寫法是：

```ld
PROVIDE(_data_lma = LOADADDR(.data));
```

而 `_data_vma` 可以放在 `.data` 開頭：

```ld
.data :
{
    . = ALIGN(4);
    PROVIDE(_data_vma = .);
    ...
    PROVIDE(_edata = .);
} >RAM AT>FLASH

PROVIDE(_data_lma = LOADADDR(.data));
```

原本 script 用 `.dalign` 和 `.dlalign`：

```ld
.dalign :
{
    . = ALIGN(4);
    PROVIDE(_data_vma = .);
} >RAM AT>FLASH

.dlalign :
{
    . = ALIGN(4);
    PROVIDE(_data_lma = .);
} >FLASH AT>FLASH
```

這是 vendor script 的繞法：透過切到 RAM/FLASH 兩個 region，分別記錄 `.data` 的 VMA 和 LMA。

它們不是硬體需要的特殊 section，只是用來對齊並產生 symbol 的輔助 section。

## 清空 `.bss`

```asm
2:
    la      a0, _sbss
    la      a1, _ebss
    bgeu    a0, a1, 2f

1:
    sw      zero, (a0)
    addi    a0, a0, 4
    bltu    a0, a1, 1b
```

`.bss` 是未初始化或初始為 0 的 global/static 變數，例如：

```c
int global_uninit;
static int counter;
```

C 語言保證它們進入 `main()` 前是 0，所以 startup 要清零：

```text
RAM [_sbss, _ebss) = 0
```

`zero` 是 RISC-V x0 暫存器，永遠是 0。

等價 C：

```c
uint32_t *p = (uint32_t *)&_sbss;
uint32_t *end = (uint32_t *)&_ebss;

while (p < end) {
    *p++ = 0;
}
```

## 設定 CH32V CSR

```asm
li          t0, 0x1f
csrw        0xbc0, t0
```

根據註解，這是設定 pipeline 與 instruction prediction。`0xbc0` 是 CH32V/QingKe 特定 CSR。

```asm
li          t0, 0x0b
csrw        0x804, t0
```

根據註解，這是啟用 interrupt nesting 與 hardware stack。`0x804` 也是 CH32V 特定 CSR。

```asm
li          t0, 0x6088
csrw        mstatus, t0
```

`mstatus` 是 RISC-V machine status CSR。這裡設定 interrupt、floating point、privilege mode 相關狀態。具體 bit 要對照 CH32V/QingKe 手冊。

## 設定 `mtvec`

```asm
la          t0, _vector_base
ori         t0, t0, 3
csrw        mtvec, t0
```

`la` 是 pseudo instruction，load address：

```text
t0 = address of _vector_base
```

`ori t0, t0, 3`：

```text
t0 = t0 | 3
```

也就是把低兩個 bit 設成 `0b11`。

`mtvec` 是 machine trap-vector base-address register。

在 RV32 中，`mtvec` 是 32-bit CSR。標準格式大致是：

```text
mtvec[31:2] = BASE
mtvec[1:0]  = MODE
```

CH32V 這裡用 `| 3`，是 vendor 特定 vector table mode。

```asm
csrw mtvec, t0
```

把 vector table base 與 mode 寫進 CPU。

## `SystemInit`

```asm
jal         SystemInit
```

這等價於呼叫：

```c
SystemInit();
```

如果 `SystemInit` 用 C 寫，一般只要是非 `static` 函式，且有一起參與 link，startup 就能連到：

```c
void SystemInit(void)
{
}
```

如果是 C++，要避免 name mangling：

```cpp
extern "C" void SystemInit(void)
{
}
```

Assembly 端通常不需要 `.extern SystemInit`。assembler 會留下 unresolved relocation，由 linker 在其他 object file 中解析。

## 為什麼不是 `jal main`

這份 startup 最後寫：

```asm
la          t0, main
csrw        mepc, t0
mret
```

而不是：

```asm
jal         main
```

`jal main` 是普通函式呼叫：

```text
ra = return address
pc = main
```

`mepc = main; mret` 則是用 machine return 的方式進入 `main`：

```text
mepc = main address
mret -> jump to mepc and apply mstatus return semantics
```

這通常是 vendor startup 想讓 `main` 在前面設定好的 `mstatus` 狀態下開始執行，包含 privilege / interrupt state transition。

如果改用 `jal main`，可能也能跑，但不會觸發 `mret` 的狀態恢復語意，而且一般還需要在 `main` return 後補無限迴圈：

```asm
jal main
1:
    j 1b
```

## C `static` 函式與 linker 可見性

如果 C 裡寫：

```c
static void SystemInit(void)
{
}
```

這個 symbol 只有該 `.c` 檔內可見，object file 裡會是 local symbol，linker 不能拿它來滿足 startup 裡的：

```asm
jal SystemInit
```

非 `static`：

```c
void SystemInit(void)
{
}
```

通常會產生 global symbol，linker 可以解析。

可以用 `nm` 觀察：

```text
T SystemInit  global text symbol
t SystemInit  local text symbol
U SystemInit  undefined reference
```

## `ALIGN(4)`

linker script 裡：

```ld
. = ALIGN(4);
```

這裡的 `4` 是 4 bytes。

它會把目前 VMA location counter `.` 往上推到下一個 4-byte 對齊地址。

```text
0x1000 -> 0x1000
0x1001 -> 0x1004
0x1002 -> 0x1004
0x1003 -> 0x1004
```

注意 linker script 裡 `ALIGN(4)` 是 bytes；assembly 裡 `.align 1` 在 RISC-V GNU assembler 常是 `2^1 = 2 bytes`。

## 總結

`startup_ch32v30x_D8C.S` 做的是：

```text
1. 定義 entry symbol _start
2. 建立 vector table
3. 提供 weak default interrupt handlers
4. 初始化 gp/sp
5. 複製 .data
6. 清空 .bss
7. 設定 CH32V/RISC-V CSR
8. 設定 mtvec
9. 呼叫 SystemInit
10. 用 mret 進入 main
```

`Link.ld` 則負責提供 startup 需要的 symbol：

```text
_data_lma
_data_vma
_edata
_sbss
_ebss
__global_pointer$
_susrstack
_eusrstack
```

startup 和 linker script 是一組契約：startup 用這些 symbol 做初始化，linker script 必須正確定義這些 symbol 的位置。
