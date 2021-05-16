;
;   This Code was written by Joseph Ryan
;   https://gist.github.com/P1n3appl3
;
        .ORIG x3000
        LD R1, board
        LD R2, size
        JSR ffill
loop    JSR fclear
        LD R0, board
        LD R1, width
        LD R2, height
        JSR fdraw
        LD R0, board
        LD R1, next
        LD R2, width
        LD R3, height
        JSR fcomp
        LD R0, board
        LD R1, next
        LD R2, size
        JSR fnext
        GETC
        BRnzp loop
        HALT
width   .FILL #30
height  .FILL #15
size    .FILL #450   ; width * height
board   .FILL xA000
next    .FILL xC000

; Divides R0 by R1, placing the quotient in R2 and remainder in R3
; Clobbers R2-R6
fdiv    LEA R3, mask
        AND R6, R6, #0
        ADD R6, R6, #1
loop1   STR R6, R3, #0
        ADD R3, R3, #1
        ADD R6, R6, R6
        BRnp loop1
        AND R2, R2, #0
        AND R3, R3, #0
        LEA R4, mask
loop0   ADD R3, R3, R3
        LDR R5, R4, #15
        AND R6, R5, R0
        BRz noadd
        ADD R3, R3, #1
noadd   NOT R6, R1
        ADD R6, R6, #1
        ADD R6, R6, R3
        BRn nosub
        NOT R6, R1
        ADD R6, R6, #1
        ADD R3, R3, R6
        ADD R2, R2, R5
nosub   ADD R4, R4, #-1
        ADD R5, R5, #-1
        BRp loop0
        RET
mask    .BLKW #16

; Multiplies R0 by R1, placing the result in R2
; Clobbers R2-R5
fmult   AND R5, R1, #-1
        AND R3, R3, #0
        ADD R3, R3 #1
        AND R2, R2, #0
loop7   AND R4, R0, R3
        BRz skip
        ADD R2, R2, R5
skip    ADD R5, R5, R5
        ADD R3, R3, R3
        BRnp loop7
        RET

; Prints a bunch of newlines, effectively clearing the screen
; Clobbers R0-R2
fclear  LD R1, clrh
        LEA R0, endl
        AND R2, R7, #-1
loop2   PUTS
        ADD R1, R1, #-1
        BRp loop2
        AND R7, R2, #-1
        RET
clrh    .FILL #10
endl    .FILL #10
        .FILL #0

; Draws board starting at R0 with dimensions R1xR2
; Clobbers R0-R6
fdraw   AND R6, R7, #-1
        AND R4, R0, #-1
loop4   AND R5, R1, #-1
loop5   LEA R0, space
        LDR R3, R4, #0
        BRz zero
        LEA R0, o
zero    PUTS
        ADD R4, R4, #1
        ADD R5, R5, #-1
        BRp loop5
        LEA R0, endl
        PUTS
        ADD R2, R2, #-1
        BRp loop4
        AND R7, R6, #-1
        RET
space   .STRINGZ " "
o       .STRINGZ "O"

; Fills Mem[R1..R1+R2] with pseudorandom binary numbers
; Clobbers R0-R6
ffill   AND R6, R7, #-1
        LEA R0, msg
        PUTS
        GETC
        AND R4, R4, #0
        ADD R4, R4, #8
        AND R3, R0, #-1
shl     ADD R3, R3, R3
        ADD R4, R4, #-1
        BRp shl
        ADD R0, R0, R3
loop6   AND R3, R3, #0
        ADD R0, R0, R0
        LD R5, left1
        AND R4, R0, R5
        BRz false1
        NOT R3, R3
false1  LD R5, left2
        AND R4, R0, R5
        BRz false2
        NOT R3, R3
false2  ADD R3, R3, #1
        ADD R0, R0, R3
        AND R4, R0, #1
        STR R4, R1, #0
        ADD R1, R1, #1
        ADD R2, R2, #-1
        BRp loop6
        AND R7, R6, #-1
        RET
msg     .STRINGZ "Give me a 1 character seed: "
left1   .FILL x8000
left2   .FILL x4000

; Fills the board at R0 according to the neighbor sums of the board at R1 with size R2
; Clobbers R0-R5
fnext   AND R6, R6, #0
        LDR R3, R1, #0
        LDR R4, R0, #0
        BRz dead
        ADD R3, R3, #-2
        BRn die
        ADD R3, R3, #-1
        BRp die
        BRnzp live
dead    ADD R3, R3, #-3
        BRnp die
live    ADD R6, R6, #1
die     STR R6, R0, #0
        ADD R0, R0, #1
        ADD R1, R1, #1
        ADD R2, R2, #-1
        BRp fnext
        RET

; Fills Mem[R1..R1+R2*R3] with the neighbor sums of the board at R0 with dimensions R2xR3
; Clobbers R0-R6
fcomp   ST R0, b
        ST R1, n
        ST R2, w
        ST R3, h
        ST R7, raddr
        AND R0, R0, #0
        ST R0, i
        ST R0, t
        ADD R1, R3, #0
loop3   ADD R0, R0, R1
        ADD R0, R0, #-1
        JSR fdiv
        ADD R0, R3, #0
        LD R1, w
        JSR fmult
        ST R2, u
        ADD R0, R0, #2
        LD R1, h
        JSR fdiv
        ADD R0, R3, #0
        LD R1, w
        JSR fmult
        ST R2, d
        AND R0, R0, #0
        ST R0, j
loop8   LD R0, j
        LD R1, w
        ADD R0, R0, R1
        ADD R0, R0, #-1
        JSR fdiv
        ST R3, l
        ADD R0, R0, #2
        JSR fdiv
        LD R0, b
        LD R1, u
        LD R2, d
        LD R4, t
        AND R5, R5, #0
        ADD R6, R0, R3
        ADD R7, R6, R1
        LDR R7, R7, #0
        ADD R5, R5, R7
        ADD R7, R6, R2
        LDR R7, R7, #0
        ADD R5, R5, R7
        ADD R7, R6, R4
        LDR R7, R7, #0
        ADD R5, R5, R7
        LD R3, l
        ADD R6, R0, R3
        ADD R7, R6, R1
        LDR R7, R7, #0
        ADD R5, R5, R7
        ADD R7, R6, R2
        LDR R7, R7, #0
        ADD R5, R5, R7
        ADD R7, R6, R4
        LDR R7, R7, #0
        ADD R5, R5, R7
        LD R3, j
        ADD R6, R0, R3
        ADD R7, R6, R1
        LDR R7, R7, #0
        ADD R5, R5, R7
        ADD R7, R6, R2
        LDR R7, R7, #0
        ADD R5, R5, R7
        LD R0, n
        ADD R0, R0, R3
        ADD R0, R0, R4
        STR R5, R0, #0
        ADD R3, R3, #1
        ST R3, j
        LD R2, w
        NOT R3, R3
        ADD R3, R3, #1
        ADD R3, R3, R2
        BRp loop8
        ADD R4, R4, R2
        ST R4, t
        LD R0, i
        ADD R0, R0, #1
        ST R0, i
        LD R1, h
        NOT R2, R0
        ADD R2, R2, #1
        ADD R2, R2, R1
        BRp loop3
        LD R7, raddr
        RET
b       .FILL #0
n       .FILL #0
w       .FILL #0
h       .FILL #0
raddr   .FILL #0
i       .FILL #0
j       .FILL #0
u       .FILL #0
t       .FILL #0
d       .FILL #0
l       .FILL #0
        .END