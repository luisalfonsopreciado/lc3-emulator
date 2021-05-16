;
; Author: Luis Alfonso Preciado
; Version: 00.00.01
; Program: MP2.asm
; Description: 
;     An Assembly program that computes integer division and remainder. 
;     Y=A/B Z=A%B
;     The source operand A is stored in location 0x3100 in memory. 
;     The source operand B is stored in location0x3101. 
;     The result Y should be stored in location 0x3102. 
;     The result Z should be stored in location 0x3103.
;     The program starts at location 0x3000. 
;     The Program Assumes A>0 B>0 
; 
; Registers:
;	R0 = Input/Output
;	R1 = Temp variable
;	R2 = Address of word
;	R3 = Address of guess
;	R4 = "_"
;	R5 = word counte

.ORIG x3000

LD R0, LOCATIONA    ; Load A into x3100
LD R1, A
STR R1, R0, #0
LD R0, LOCATIONB    ; Load B into x3101
LD R1, B
STR R1, R0, #0

LDI R0, LOCATIONA ; Load integer A into R0, Where A > 0, R0 Will contain the mod at the end of the program execution
LDI R1, LOCATIONB ; Load integer B into R1, where B > 0

AND R2, R2, #0 ; Clear R2 which stores division result

; Turn B To Negative, Used in Division loop
NOT R1, R1
ADD R1, R1, #1

; Load A into R3
AND R3, R3, #0
ADD R3, R3, R0

; Check if A <= B, If B > 0 Go to STORERESULTS, else continue to loop
ADD R3, R3,R1
BRnz STORERESULTS

; While A > B
LOOP
ADD R2, R2,#1 ; Increase division Register
ADD R0, R0, R1 ; A -= B

; Check if A <= B
AND R3, R3, #0
ADD R3, R3, R0
ADD R3, R3, R1
BRzp LOOP

; Store results
STORERESULTS STI R2, DIV ; Store result in location x3102
STI R0, MOD         ; Store mod in location x3103

DIV .FILL x3102
MOD .FILL x3103 

LOCATIONA .FILL x3100
LOCATIONB .FILL x3101

A .FILL #8  ; Declare A
B .FILL #3  ; Declare B
HALT
.END