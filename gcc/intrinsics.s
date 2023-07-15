.syntax unified
.p2align 2

.global DisableInterrupts
.type   DisableInterrupts,%function
DisableInterrupts:
    CPSID   I
    BX      LR

.global EnableInterrupts
.type   EnableInterrupts,%function
EnableInterrupts:
    CPSIE   I
    BX      LR

# *********** StartCritical ************************
# make a copy of previous I bit, disable interrupts
.global StartCritical
.type   StartCritical,%function
StartCritical:
    MRS     R0, PRIMASK @ save old status
    CPSID   I           @ mask all (except faults)
    BX      LR

# *********** EndCritical ************
# restore I bit to its previous value
.global EndCritical
.type   EndCritical,%function
EndCritical:
    MSR     PRIMASK, R0 @ restore
    BX      LR
    

# *********** memcpy ************
# Copy n bytes to dest from src
.global memcpy
.type memcpy,%function
memcpy:
    CMP R2, #4
    BLT memcpy_bytes
    LDR R3, [R1], #4
    STR R3, [R0], #4
    SUBS R2, #4
    BNE memcpy
memcpy_bytes:
    CMP R2, #0
    BEQ memcpy_done
    LDRB R3, [R1], #1
    STRB R3, [R0], #1
    SUBS R2, #1
    BEQ memcpy_bytes
memcpy_done: 
    BX LR

