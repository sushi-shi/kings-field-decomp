.set noreorder
.set noat

.text
.globl EnterCriticalSection
.type EnterCriticalSection,@function
EnterCriticalSection:
    addiu   $a0, $zero, 1
    syscall 0
    jr      $ra
     nop
.size EnterCriticalSection, .-EnterCriticalSection
