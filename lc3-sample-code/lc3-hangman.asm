;
; Author: Sidd Weiker
; Version: 04.24.09
; Program: hangMan.asm
; Registers:
;	R0 = Input/Output
;	R1 = Temp variable
;	R2 = Address of word
;	R3 = Address of guess
;	R4 = "_"
;	R5 = word counter
		.ORIG	x3000
;Start
		LEA	R0, PROMPT1
		PUTS			;Display beginning prompt
		LD	R2, WORD		
		LD	R3, GUESS	;Load address's
		LD	R4, BLANK	;Load "_"
		AND	R5, R5, #0	;Initialized to 0	
;Word Input
INPUT		GETC
		LD	R1, BACKSP
		NOT	R1, R1
		ADD	R1, R1, #1
		ADD	R1, R0, R1
		BRz	INPUT		;Goto [INPUT] backspace read skip it
		LD 	R1, NEWLN
		NOT	R1, R1
		ADD	R1, R1, #1
		ADD 	R1, R0, R1
		BRz 	INPUT_DONE	;Goto [INPUT_DONE] if a new line was entered
		STR	R0, R2, #0	;Store char in word string
		STR	R4, R3, #0	;Store "_" in guess string
		LD	R0, ASCII
		ADD	R0, R0, #-6	
		OUT			;Echo "*"
		ADD	R2, R2, #1	;Increment word
		ADD	R3, R3, #1	;Increment guess
		LD	R0, SPACE
		STR	R0, R3, #0
		ADD	R3, R3, #1	;Put a space in between each char GUI use
		ADD	R5, R5, #1	;Increment word counter
		BRnzp	INPUT		;Goto [INPUT] we're not done
;
; Registers:
;	R0 = Input/Output
;	R1 = Temp variable
;	R2 = Address of word
;	R3 = Address of guess
;	R4 = Length
;	R5 = Address of Used
;
;Prepare word guessing
INPUT_DONE	LEA	R0, PROMPT2	;Display second prompt
		PUTS
		ST	R5, LENGTH
		LD	R5, USED	;Load address of used string
		AND	R0, R0, #0
		STR	R0, R2, #0
		STR	R0, R3, #0
		STR	R0, R5, #0	;Null terminate the strings
TOP		LD	R2, WORD	
		LD	R3, GUESS	;Load starting words
		LD	R5, USED
		LEA	R0, PROMPT3
		PUTS
		LD	R0, USED
		PUTS
		LD	R0, NEWLN
		OUT
		LD	R0, GUESS
		PUTS
		LD	R0, NEWLN
		OUT
;Word guessing
		GETC
		LD	R4, LENGTH
		BRnzp	WORD1		;Goto [WORD1] to get rid of pre changes.
;Is char in the word?
WORD0		ADD	R4, R4, #-1	;Decrement counter
		ADD	R2, R2, #1	;Increment word
		ADD	R3, R3, #2	;Increment guess
WORD1		ADD	R4, R4, #0
		BRz	IN_USED		;Goto [IN_USED] done going through word
		LDR	R1, R2, #0
		NOT	R1, R1
		ADD	R1, R1, #1
		ADD	R1, R0, R1	;Subtract word char from input
		BRnp	WORD0		;Goto [WORD0] if char does not match
		STR	R0, R3, #0
;Is char everywhere in word?
WORD2		ADD	R4, R4, #0
		BRz	CHECK		;Goto [CHECK] Done with input char checking
		ADD	R4, R4, #-1	;Decrement counter
		ADD	R2, R2, #1	;Increment word
		ADD	R3, R3, #2	;Increment guess
		LDR	R1, R2, #0
		NOT	R1, R1
		ADD	R1, R1, #1	;Inverse
		ADD	R1, R0, R1	;Subtract word char from
		Brnp	WORD2		;Goto [WORD2] Not a match, continue
		STR	R0, R3, #0	;Store the char matched into "Guess"
		BRnzp	WORD2		;Goto [WORD2] Is a match, continue
;has char been in "used" already?
IN_USED		LDR	R1, R5, #0
		BRnp	IN_CHAR		;Goto [CHAR] Not end, so check for char
		STR	R0, R5, #0	;Store char into used
		ADD	R5, R5, #1	;Increment used
		AND	R0, R0, #0
		STR	R0, R5, #0	;Null terminate used
		BRnzp	CHECK		;Goto [CHECK] char stored
IN_CHAR		NOT	R1, R1
		ADD	R1, R1, #1
		ADD	R1, R0, R1	;Subtract Used value from char
		BRz	CHECK		;Goto [CHECK] char was already there
		ADD	R5, R5, #1	;Increment address of used
		BRnzp	IN_USED		;Goto [IN_USED] no match found try next.
;Check to see if strings match
CHECK		LD	R2, WORD	
		LD	R3, GUESS
		LD	R4, LENGTH
CHECK_AGAIN	LDR	R0, R2, #0
		LDR	R1, R3, #0
		NOT	R1, R1
		ADD	R1, R1, #1	;Inverse R1
		ADD	R1, R0, R1	;subtract chars from word and guess
		BRnp	TOP		;Goto [TOP] They don't match
		ADD	R4, R4, #0
		Brz	DONE		;Goto [Done] All chars match!
		ADD	R4, R4, #-1	;Decrement length
		ADD	R2, R2, #1	;Increment word
		ADD	R3, R3, #2	;Increment guess
		BRnzp	CHECK_AGAIN	;Goto [CHECK_AGAIN] chars so far have been equal
;End
DONE		LEA	R0, PROMPT2
		PUTS
		LEA	R0, PROMPT3
		PUTS
		LD	R0, USED
		PUTS
		LD	R0, NEWLN
		OUT
		LD	R0, GUESS
		PUTS		
		LEA	R0, PROMPT4
		PUTS		
		HALT
;Data
ASCII	.FILL	x0030
SPACE	.FILL	x0020
BACKSP	.FILL	x0008
TAB	.FILL	x0009
NEWLN	.FILL	x000A
BLANK	.FILL	x005F
LENGTH	.FILL	x0000
PROMPT1	.STRINGZ	"[Enter your word]\n"
PROMPT2	.STRINGZ	"\n[Guess the word!]\n"
PROMPT3	.STRINGZ	"[Letters used] > "
PROMPT4	.STRINGZ	"\n[You got it right!]\n"
WORD	.FILL	x3200
GUESS	.FILL	x3240
USED	.FILL	x3260
;
	.END