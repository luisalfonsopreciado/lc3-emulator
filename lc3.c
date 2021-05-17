#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
/* unix specific */
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define FLAG_COUNT 3

/* The LC-3 has 65,536 available memory locations */
uint16_t memory[UINT16_MAX];

/* The LC-3 has 8 general purpose registers, 1 Program Counter, 1 Condition Flag */
enum registers
{
    R_R0 = 0, /* 8 GENERAL PURPOSE REGISTERS*/
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,   /* PROGRAM COUNTER (PC) */
    R_COND, /* CONDITION FLAG */
    R_PSR,
    R_SSP,   /* SUPERVISOR STACK POINTER */
    R_USP,   /* USER STACK POINTER */
    R_COUNT, /* NUMBER OF TOTAL REGISTERS */
};

uint16_t reg[R_COUNT];

/* The LC-3 has 16 different instructions */
enum instruction_set
{
    OP_BR = 0,
    OP_ADD,      /* OPCODE ADD*/
    OP_LD,       /* LOAD */
    OP_ST,       /* STORE */
    OP_JSR,      /* JUMP SUB-ROUTINE */
    OP_AND,      /* AND */
    OP_LDR,      /* LOAD (PC-RELATIVE) */
    OP_STR,      /* STORE (PC-RELATIVE) */
    OP_RTI,      /* RETURN */
    OP_NOT,      /* NOT */
    OP_LDI,      /* LOAD INDIRECT */
    OP_STI,      /* STORE INDIRECT */
    OP_JMP,      /* JUMP */
    OP_RESERVED, /* RESERVED INSTRUCTION*/
    OP_LEA,      /* LOAD EFFECTIVE ADDRESS*/
    OP_TRAP,     /* TRAP */
};

/* Conditional Flags */
enum FLAGS
{
    FL_P = 1 << 0, /* Positive */
    FL_Z = 1 << 1, /* Zero */
    FL_N = 1 << 2, /* Negative */
};

/* TRAP Codes */
enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

uint16_t flags[FLAG_COUNT];

/* Memory Mapped registers */
enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02, /* keyboard data */
    MR_DSR = 0xFE04,  /* display status */
    MR_DDR = 0xFE06   /* display data*/
};

/* TODO: Load LC3 OS */
void load_lc3_os()
{
        uint16_t trap_vector_table_entries[] = {0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025};
        uint16_t trap_vector_table_values[] = {0x0400, 0x0430, 0x0450, 0x04A0, 0x04E0, 0xFD70};
    //     lc3os = {
    //     // Implementation of GETC
    //     0x0400: 0x3E07,
    //     0x0401: 0xA004,
    //     0x0402: 0x07FE,
    //     0x0403: 0xA003,
    //     0x0404: 0x2E03,
    //     0x0405: 0xC1C0,
    //     0x0406: 0xFE00,
    //     0x0407: 0xFE02,
    //     // Implementation of OUT
    //     0x0430: 0x3E0A,
    //     0x0431: 0x3208,
    //     0x0432: 0xA205,
    //     0x0433: 0x07FE,
    //     0x0434: 0xB004,
    //     0x0435: 0x2204,
    //     0x0436: 0x2E04,
    //     0x0437: 0xC1C0,
    //     0x0438: 0xFE04,
    //     0x0439: 0xFE06,
    //     // Implementation of PUTS
    //     0x0450: 0x3E16,
    //     0x0451: 0x3012,
    //     0x0452: 0x3212,
    //     0x0453: 0x3412,
    //     0x0454: 0x6200,
    //     0x0455: 0x0405,
    //     0x0456: 0xA409,
    //     0x0457: 0x07FE,
    //     0x0458: 0xB208,
    //     0x0459: 0x1021,
    //     0x045A: 0x0FF9,
    //     0x045B: 0x2008,
    //     0x045C: 0x2208,
    //     0x045D: 0x2408,
    //     0x045E: 0x2E08,
    //     0x045F: 0xC1C0,
    //     0x0460: 0xFE04,
    //     0x0461: 0xFE06,
    //     0x0462: 0xF3FD,
    //     0x0463: 0xF3FE,
    //     // Implementation of IN
    //     0x04A0: 0x3E06,     // ST R7, SaveR7
    //     0x04A1: 0xE006,     // LEA R0, Message
    //     0x04A2: 0xF022,     // PUTS
    //     0x04A3: 0xF020,     // GETC
    //     0x04A4: 0xF021,     // OUT
    //     0x04A5: 0x2E01,     // LD R7, SaveR7
    //     0x04A6: 0xC1C0,     // RET
    //     0x04A7: 0x3001,     // SaveR7 (.BLKW #1)
    //     /* the "Input a character> " message goes here */
    //     // Implementation of PUTSP
    //     0x04E0: 0x3E27,
    //     0x04E1: 0x3022,
    //     0x04E2: 0x3222,
    //     0x04E3: 0x3422,
    //     0x04E4: 0x3622,
    //     0x04E5: 0x1220,
    //     0x04E6: 0x6040,
    //     0x04E7: 0x0406,
    //     0x04E8: 0x480D,
    //     0x04E9: 0x2418,
    //     0x04EA: 0x5002,
    //     0x04EB: 0x0402,
    //     0x04EC: 0x1261,
    //     0x04ED: 0x0FF8,
    //     0x04EE: 0x2014,
    //     0x04EF: 0x4806,
    //     0x04F0: 0x2013,
    //     0x04F1: 0x2213,
    //     0x04F2: 0x2413,
    //     0x04F3: 0x2613,
    //     0x04F4: 0x2E13,
    //     0x04F5: 0xC1C0,
    //     0x04F6: 0x3E06,
    //     0x04F7: 0xA607,
    //     0x04F8: 0x0801,
    //     0x04F9: 0x0FFC,
    //     0x04FA: 0xB003,
    //     0x04FB: 0x2E01,
    //     0x04FC: 0xC1C0,
    //     0x04FE: 0xFE06,
    //     0x04FF: 0xFE04,
    //     0x0500: 0xF3FD,
    //     0x0501: 0xF3FE,
    //     0x0502: 0xFF00,
    //     // Implementation of HALT
    //     0xFD00: 0x3E3E,
    //     0xFD01: 0x303C,
    //     0xFD02: 0x2007,
    //     0xFD03: 0xF021,
    //     0xFD04: 0xE006,
    //     0xFD05: 0xF022,
    //     0xFD06: 0xF025,
    //     0xFD07: 0x2036,
    //     0xFD08: 0x2E36,
    //     0xFD09: 0xC1C0,
    //     0xFD70: 0x3E0E,
    //     0xFD71: 0x320C,
    //     0xFD72: 0x300A,
    //     0xFD73: 0xE00C,
    //     0xFD74: 0xF022,
    //     0xFD75: 0xA22F,
    //     0xFD76: 0x202F,
    //     0xFD77: 0x5040,
    //     0xFD78: 0xB02C,
    //     0xFD79: 0x2003,
    //     0xFD7A: 0x2203,
    //     0xFD7B: 0x2E03,
    //     0xFD7C: 0xC1C0,
    //     /* the "halting the processor" message goes here */
    //     0xFDA5: 0xFFFE,
    //     0xFDA6: 0x7FFF,
    //     // Display status register
    //     0xFE04: 0x8000,
    //     // Machine control register
    //     0xFFFE: 0xFFFF,
    // };
}

/* UNIX specific keyboard check */
uint16_t check_key()
{
    fd_set readfds;
    /*FD_ZERO(&fdset) initializes a descriptor set fdset to the null set.*/
    FD_ZERO(&readfds);
    /* FD_SET(fd, &fdset) : Include a particular descriptor fd in fdset (readfds) */
    FD_SET(STDIN_FILENO, &readfds); /* include STDIN_FILENO in the readfds fd set */

    /* maximum interval to wait for the below selection to complete */
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    /* select call returns 1 if the readfds is ready for reading */
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

/* This is Unix specific code for setting up terminal input. */
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

/* Called when the user types in the 'interrupt' character */
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

/* Swaps the bits of an input unsigned 16 bit integer */
uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

/* Read image file and place instructions in the LC-3 VM memory*/
void read_image_file(FILE *f)
{
    /* Origin informs us where in memory to place the image */
    uint16_t orig;
    fread(&orig, sizeof(orig), 1, f);
    orig = swap16(orig);

    /* Set PC equal to origin */
    reg[R_PC] = orig;

    uint16_t max_read = UINT16_MAX - orig;
    /* Declare pointer to current memory location to write */
    uint16_t *p = memory + orig;
    size_t read = fread(p, sizeof(uint16_t), max_read, f);

    /* While there are bytes to write to memory */
    while (read-- > 0)
    {
        /* The LC-3 uses big endian to interpret the instructions, 
            however most modern computers are little endian. 
            Therefore, it is necessary to swap the bits
        */
        *p = swap16(*p);
        ++p;
    }
}

/* Read image file into the LC-3 VM memory. Returns 1 on success, 0 otherwise */
int read_image(char *file)
{
    /* Open the file with read binary permission */
    FILE *f = fopen(file, "rb");

    if (!f)
        return 0;

    read_image_file(f);

    fclose(f);

    return 1;
}

uint16_t sign_extend(uint16_t n, int bit_count)
{
    /* If the most significant bit is set to 1, set the all bits to the left*/
    if ((n >> (bit_count - 1)) & 1)
        n |= (0xFFFF << bit_count);
    return n;
}

uint16_t mem_read(uint16_t addr)
{
    if (addr == MR_KBSR)
    {
        /* check if stdin has a key ready to be read */
        if (check_key())
        {
            /* The ready bit (bit [15]) indicates if the keyboard has received a new character. */
            memory[MR_KBSR] = (1 << 15);
            /* Place the character in the KeyBoard Data Register */
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[addr];
}

/* Memory Access */
void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

/* Print a section of memory */
void print_mem(uint16_t start, uint16_t end)
{
    for (int i = start; i <= end; i++)
    {
        printf("%x : %x\n", i, mem_read(i));
    }
}

/* Update Flags given the register */
void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_Z;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_N;
    }
    else
    {
        reg[R_COND] = FL_P;
    }
}

/* Keep the CPU Running */
int running;

/* Handle Trap System Call */
void trap(uint16_t trap_vect)
{
    switch (trap_vect)
    {
    case TRAP_GETC:
    {
        /* Read a single character from the keyboard. The character is not echoed onto the
console. Its ASCII code is copied into RO. The high eight bits of RO are cleared. 
        */
        reg[R_R0] = (uint16_t)getchar();
    }
    break;
    case TRAP_HALT:
    {
        /* Halt execution and print a message on the console. */
        puts("HALT");
        fflush(stdout);
        running = 0;
    }
    break;
    case TRAP_IN:
    {
        /*
            Print a prompt on the screen and read a single character from the keyboard. The
character is echoed onto the console monitor, and its ASCII code is copied into RO.
The high eight bits of RO are cleared.
        */
        printf("Enter a character: ");
        reg[R_R0] = (uint16_t)getchar();
        putc((char)reg[R_R0], stdout);
    }
    break;
    case TRAP_OUT:
    {
        /*
            Write a character in R0[7:0] to the console display.
        */
        putc((char)reg[R_R0], stdout);
        fflush(stdout);
    }
    break;
    case TRAP_PUTS:
    {
        /*
            Write a string of ASCII characters to the console display. The characters are contained
            in consecutive memory locations, one character per memory location, starting with
            the address specified in RO. Writing terminates with the occurrence of xOOOO in a
            memory location.
        */

        uint16_t *ptr = memory + reg[R_R0];
        while (*ptr)
        {
            putc((char)*ptr, stdout);
            ptr++;
        }
        fflush(stdout);
    }
    break;
    case TRAP_PUTSP:
    {
        /*
            Write a string of ASCII characters to the console. The characters are contained in
            consecutive memory locations, two characters per memory location, starting with the
            address specified in RO. The ASCII code contained in bits [7:0] of a memory location
            is written to the console first. Then the ASCII code contained in bits [15:8] of that
            memory location is written to the console. (A character string consisting of an odd
            number of characters to be written will have xOO in bits [15:8J of the memory
            location containing the last character to be written.) Writing terminates with the
            occurrence of xOOOO in a memory location.
        */

        uint16_t *ptr = memory + reg[R_R0];

        while (*ptr)
        {
            uint16_t start_c = (*ptr) & 0x00FF;
            uint16_t end_c = *ptr >> 8;
            putc((char)start_c, stdout);
            if (!end_c)
                break;
            putc((char)end_c, stdout);
            ptr++;
        }
        fflush(stdout);
    }
    break;

    default:
        break;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        /* Hassle user */
        printf("lc3 <image-file> \n");
        exit(2);
    }

    load_lc3_os();

    /* Load the obj files into memory */
    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    /* Setup */
    signal(SIGINT, handle_interrupt); /* handle the interrupt signal by calling the handle_interrupt function*/
    disable_input_buffering();

    /* By default the starting address for the LC-3 is 0x3000 */
    uint16_t PC_START = 0x3000;
    reg[R_PC] = PC_START;

    /* PSR set initially to x8002*/
    reg[R_PSR] = 0x8002;

    running = 1;

    while (running)
    {
        /* Fetch the instruction from memory and increment PC */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op_code = instr >> 12;

        switch (op_code)
        {
        case OP_BR:
        {
            /*
                The condition codes specified by the state of bits [11:9] are tested. If bit [11] is
                set, N is tested; if bit [11] is clear, N is not tested. If bit [10] is set, Z is tested, etc.
                If any of the condition codes tested is set, the program branches to the location
                specified by adding the sign-extended PCoffset9 field to the incremented PC.
            */
            uint16_t cond_flag = (instr >> 9) & 0x7;
            if (cond_flag & reg[R_COND])
            {
                reg[R_PC] += sign_extend(instr & 0x1FF, 9);
            }
        }
        break;
        case OP_ADD:
        {
            /*
                If bit [5] is 0, the second source operand is obtained from SR2. If bit [5] is 1, the
                second source operand is obtained by sign-extending the imm5 field to 16 bits.
                In both cases, the second source operand is added to the contents of SRI and the
                result stored in DR. The condition codes are set, based on whether the result is
                negative, zero, or positive.
            */
            uint16_t R_DR = (instr >> 9) & 0x7;
            uint16_t R_SR1 = (instr >> 6) & 0x7;
            /* are we in immediate mode? */
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                reg[R_DR] = reg[R_SR1] + sign_extend(instr & 0x01F, 5);
            }
            else
            {
                reg[R_DR] = reg[R_SR1] + reg[instr & 0x7];
            }

            update_flags(R_DR);
        }
        break;
        case OP_LD:
        {
            /* An address is computed by sign-extending bits [8:0] to 16 bits and adding this
            value to the incremented PC. The contents of memory at this address are loaded
            into DR. The condition codes are set, based on whether the value loaded is
            negative, zero, or positive. 
            */
            uint16_t R_DEST = (instr >> 9) & 0x7;
            uint16_t PC_OFFSET = sign_extend(instr & 0x1FF, 9);
            reg[R_DEST] = mem_read(reg[R_PC] + PC_OFFSET);
            update_flags(R_DEST);
        }
        break;
        case OP_ST:
        {
            /*  The contents of the register specified by SR are stored in the memory location
                whose address is computed by sign-extending bits [8:0] to 16 bits and adding this
                value to the incremented PC.
            */
            uint16_t R_SR = (instr >> 9) & 0x7;
            uint16_t PC_OFFSET = sign_extend(0x1FF & instr, 9);
            mem_write(reg[R_PC] + PC_OFFSET, reg[R_SR]);
        }
        break;
        case OP_JSR:
        {
            uint16_t pc_off_flag = (instr >> 11) & 1;
            reg[R_R7] = reg[R_PC];

            if (pc_off_flag)
            {
                /* JSR */
                reg[R_PC] += sign_extend(0x7FF & instr, 11);
            }
            else
            {
                /* JSRR */
                reg[R_PC] = reg[(instr >> 6) & 0x7];
            }
            break;
        }
        break;
        case OP_AND:
        {
            uint16_t R_DR = (instr >> 9) & 0x7;
            uint16_t R_SR1 = (instr >> 6) & 0x7;
            /* are we in immediate mode? */
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                uint16_t imm_5 = sign_extend(instr & 0x1F, 5);
                reg[R_DR] = reg[R_SR1] & imm_5;
            }
            else
            {
                uint16_t R_SR2 = instr & 0x7;
                reg[R_DR] = reg[R_SR1] & reg[R_SR2];
            }

            update_flags(R_DR);
        }

        break;
        case OP_LDR:
        {
            /*An address is computed by sign-extending bits [5:0] to 16 bits and adding this
value to the contents of the register specified by bits [8:6]. The contents of memory
at this address are loaded into DR. The condition codes are set, based on whether
the value loaded is negative, zero, or positive.*/
            uint16_t R_DR = (instr >> 9) & 0x7;
            uint16_t R_BASE = (instr >> 6) & 0x7;
            reg[R_DR] = mem_read(reg[R_BASE] + sign_extend(instr & 0x3F, 6));
            update_flags(R_DR);
        }
        break;
        case OP_STR:
        { /*The contents of the register specified by SR are stored in the memory location
whose address is computed by sign-extending bits [5:0] to 16 bits and adding this
value to the contents of the register specified by bits [8:6].*/
            uint16_t R_SR = (instr >> 9) & 0x7;
            uint16_t R_BASE = (instr >> 6) & 0x7;
            mem_write(reg[R_BASE] + sign_extend(instr & 0x3F, 6), reg[R_SR]);
        }
        break;
        case OP_RTI:
        { /*If the processor is running in Supervisor mode, the top two elements on the
            Supervisor Stack are popped and loaded into PC, PSR. If the processor is running
            in User mode, a privilege mode violation exception occurs.
          */
            // PSR[15] = 0 in Supervisor Mode
            uint16_t sup_mode = (reg[R_PSR] >> 15) & 1;
            if (sup_mode == 0)
            {
                // Pop PC from supervisor stack
                reg[R_PC] = mem_read(reg[R_R6]);
                reg[R_R6]++;
                // Pop PSR from supervisor stack
                reg[R_PSR] = mem_read(reg[R_R6]);
                reg[R_R6]++;
                // Restore User Stack Pointer
                reg[R_R6] = reg[R_USP];
            }
            else
            {
                /* Privilege mode violation occurs if the processor encounters the RTI instruction while running in User Mode, throwing an illegal opcode exception */
                // The processor sets the privilege mode to Supervisor mode (PSR[15] = 0).
                reg[R_PSR] &= ~(1 << 15);
                // R6 is loaded with the Supervisor Stack Pointer (SSP) if it does not already contain the SSP
                reg[R_R6] = reg[R_SSP];
                // The PSR and PC of the interrupted process are pushed onto the Supervisor Stack
                mem_write(reg[R_R6]--, reg[R_PSR]);
                mem_write(reg[R_R6]--, reg[R_PC]);
                // The exception supplies its 8-bit vector. In the case of the Privilege mode violation, that vector is x00

                // The processor expands that vector to x0100, the corresponding 16-bit address in the interrupt vector table.

                /* The PC is loaded with the contents of memory location x0100,
                the address of the first instruction in the corresponding exception service
                routine. */
                reg[R_PC] = 0x0100;
                printf("illegal opcode exception: An RTI instruction was executed while in user mode. The machine has been halted.\n");
                running = 0;
            }
        }

        break;
        case OP_NOT:
        {
            /*The bit-wise complement of the contents of SR is stored in DR. The condition
codes are set, based on whether the binary value produced, taken as a 2's
complement integer, is negative, zero, or positive.*/
            uint16_t R_DR = (instr >> 9) & 0x7;
            uint16_t R_SR = (instr >> 6) & 0x7;
            reg[R_DR] = ~reg[R_SR];
            update_flags(R_DR);
        }
        break;
        case OP_LDI:
        { /* An address is computed by sign-extending bits [8:0] to 16 bits and adding this
value to the incremented PC. What is stored in memory at this address is the
address of the data to be loaded into DR. The condition codes are set, based on
whether the value loaded is negative, zero, or positive. */
            uint16_t R_DR = (instr >> 9) & 0x7;
            reg[R_DR] = mem_read(mem_read(reg[R_PC] + sign_extend(instr & 0x1FF, 9)));
            update_flags(R_DR);
        }
        break;
        case OP_STI:
        {
            /*The contents of the register specified by SR are stored in the memory location
whose address is obtained as follows: Bits [8:0] are sign-extended to 16 bits and
added to the incremented PC. What is in memory at this address is the address of
the location to which the data in SR is stored.*/
            uint16_t R_SR = (instr >> 9) & 0x7;
            mem_write(mem_read(reg[R_PC] + sign_extend(instr & 0x1FF, 9)), reg[R_SR]);
        }
        break;
        case OP_JMP:
        { /* The program unconditionally jumps to the location specified by the contents of
the base register. Bits [8:6] identify the base register.
            */
            reg[R_PC] = reg[(instr >> 6) & 0x7];

            /*The RET instruction is a special case of the JMP instruction. The PC is loaded
with the contents of R7, which contains the linkage back to the instruction
following the subroutine call instruction.*/}
            break;
            case OP_RESERVED:
                break;
            case OP_LEA:
            {
                /* An address is computed by sign-extending bits [8:0] to 16 bits and adding this
value to the incremented PC. This address is loaded into DR.4" The condition
codes are set, based on whether the value loaded is negative, zero, or positive.*/
                uint16_t R_DR = (instr >> 9) & 0x7;
                reg[R_DR] = reg[R_PC] + sign_extend(instr & 0x01FF, 9);
                update_flags(R_DR);
            }
            break;
            case OP_TRAP:
            { /*
                    First R7 is loaded with the incremented PC. (This enables a return to the instruction
                    physically following the TRAP instruction in the original program after the service
                    routine has completed execution.) Then the PC is loaded with the starting address
                    of the system call specified by trapvector8. The starting address is contained in
                    the memory location whose address is obtained by zero-extending trapvector8 to
                    16 bits.
                */
                trap(instr & 0xFF);
            }
            break;
            default:
                // Bad Opcode
                return 0;
            }
    }
    // printf("PC: %x\nR0: %x, R1 : %x, R2: %x, R3: %x\nR4 : %x, R5 : %x, R6 : %x, R7 : %x\n", reg[R_PC], reg[R_R0], reg[R_R1], reg[R_R2], reg[R_R3], reg[R_R4], reg[R_R5], reg[R_R6], reg[R_R7]);
    /* Shutdown */
    restore_input_buffering();
}