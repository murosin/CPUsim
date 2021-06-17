/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */

#include "cpuboard.h"
#include <stdio.h>
#include <stdbool.h>

/*=============================================================================
 *   Simulation of a Single Instruction
 *===========================================================================*/
unsigned int num;

Uword get_obj_code(Cpub *cpub) {
    Uword obj;
    obj= cpub->mem[cpub->pc];
    cpub->pc++;
    return obj;
}
int step(Cpub *cpub) {
    Uword obj= get_obj_code(cpub);
    if((obj & 0xf8) == 0x00) {  //NOP
        printf("NOP\n");
    } else if((obj & 0xfc) == 0x0c) {  //HLT
        return RUN_HALT;
    } else if((obj & 0xf8) == 0x10) {  //OUT
        cpub->obuf.buf= cpub->acc;
        cpub->obuf.flag= 1;
    } else if((obj & 0xf8) == 0x18) {  //IN
        cpub->acc= cpub->ibuf->buf;
        cpub->ibuf->flag= 0;
    } else if((obj & 0xf8) == 0x20) {  //RCF
        cpub->cf= 0;
    } else if((obj & 0xf8) == 0x28) {  //SCF
        cpub->cf= 1;
    } else if((obj & 0xf0) == 0x60) {
        LD(obj, cpub);
    } else if((obj & 0xf0) == 0x70) {
        ST(obj, cpub);
    } else if((obj & 0xf0) == 0xB0) {
        ADD(obj, cpub);
    } else if((obj & 0xf0) == 0x90) {
        ADC(obj, cpub);
    } else if((obj & 0xf0) == 0xA0) {
        SUB(obj, cpub);
    } else if((obj & 0xf0) == 0x80) {
        SBC(obj, cpub);
    } else if((obj & 0xf0) == 0xf0) {
        CMP(obj, cpub);
    } else if((obj & 0xf0) == 0xe0) {
        AND(obj, cpub);
    } else if((obj & 0xf0) == 0xd0) {
        OR(obj, cpub);
    } else if((obj & 0xf0) == 0xc0) {
        EOR(obj, cpub);
    } else if((obj & 0xf0) == 0x40 && (obj & 0x04) == 0x00) {
        Ssm(obj, cpub);
    } else if((obj & 0xf0) == 0x40 && (obj & 0x04) == 0x04) {
        Rsm(obj, cpub);
    } else if((obj & 0xf0) == 0x30) {
        Bbc(obj, cpub);
    } else if(obj == 0x0A) {
        JAL(cpub);
    } else if(obj == 0x0B) {
        JR(cpub);
    } else {
        printf("???\n");
    }

    return RUN_STEP;
}

void LD(Uword obj, Cpub *Cpub) {  //(B)->A
    Uword B= 0;
    if((obj & 0x07) == 0x00) {  //B:ACC
        B= Cpub->acc;
    } else if((obj & 0x07) == 0x01) {  //B:IX
        B= Cpub->ix;
    } else if((obj & 0x06) == 0x02) {  //B:d
        B= get_obj_code(Cpub);
    } else if((obj & 0x07) == 0x04) {  //B:[d]
        B= Cpub->mem[get_obj_code(Cpub)];
    } else if((obj & 0x07) == 0x05) {  //B:(d)
        B= Cpub->mem[get_obj_code(Cpub) + 0x100];
    } else if((obj & 0x07) == 0x06) {  //B:[IX+d]
        B= Cpub->mem[Cpub->ix + get_obj_code(Cpub)];
    } else if((obj & 0x07) == 0x07) {  //B:(IX+d)
        B= Cpub->mem[Cpub->ix + get_obj_code(Cpub) + 0x100];
    }

    if((obj & 0x08) == 0x00) {  //A:ACC
        Cpub->acc= B;
    } else {  //A:IX
        Cpub->ix= B;
    }
}

void ST(Uword obj, Cpub *Cpub) {  //(A)->B
    Uword A= 0;
    if((obj & 0x08) == 0x00) {  //A:ACC
        A= Cpub->acc;
    } else {  //A:IX
        A= Cpub->ix;
    }

    if((obj & 0x07) == 0x04) {  //B:[d]
        Cpub->mem[get_obj_code(Cpub)]= A;
    } else if((obj & 0x07) == 0x05) {  //B:(d)
        Cpub->mem[get_obj_code(Cpub) + 0x100]= A;
    } else if((obj & 0x07) == 0x06) {  //B:[IX+d]
        Cpub->mem[Cpub->ix + get_obj_code(Cpub)]= A;
    } else if((obj & 0x07) == 0x07) {  //B:(IX+d)
        Cpub->mem[Cpub->ix + get_obj_code(Cpub) + 0x100]= A;
    }
}

void Bbc(Uword obj, Cpub *Cpub) {  //B'->PC or NOP
    Uword bc= obj & 0x0f;
    bool c= false;
    switch(bc) {
        case 0x00:  //BA
            c= true;
            break;
        case 0x08:  //BVF
            if(Cpub->vf == 1) c= true;
            break;
        case 0x01:  //BNZ
            if(Cpub->zf == 0) c= true;
            break;
        case 0x09:  //BZ
            if(Cpub->zf == 1) c= true;
            break;
        case 0x02:  //BZP
            if(Cpub->nf == 0) c= true;
            break;
        case 0x0A:  //BN
            if(Cpub->nf == 1) c= true;
        case 0x03:  //BP
            if((Cpub->nf | Cpub->zf) == 0) c= true;
            break;
        case 0x0B:  //BZN
            if((Cpub->nf | Cpub->zf) == 1) c= true;
            break;
        case 0x04:  //BNI
            if(Cpub->ibuf->flag == 0) c= true;
            break;
        case 0x0c:  //BNO
            if(Cpub->obuf.flag == 1) c= true;
            break;
        case 0x05:  //BNC
            if(Cpub->cf == 0) c= true;
            break;
        case 0x0d:  //BC
            if(Cpub->cf == 1) c= true;
            break;
        case 0x06:  //BGE
            if((Cpub->vf ^ Cpub->nf) == 0) c= true;
            break;
        case 0x0e:  //BLT
            if((Cpub->vf ^ Cpub->nf) == 1) c= true;
            break;
        case 0x07:  //BGT
            if(((Cpub->vf ^ Cpub->nf) | Cpub->zf) == 0) c= true;
            break;
        case 0x0f:  //BLE
            if(((Cpub->vf ^ Cpub->nf) | Cpub->zf) == 1) c= true;
            break;
        default:
            break;
    }
    Uword toaddr= get_obj_code(Cpub);
    if(c) Cpub->pc= toaddr;
}

void JAL(Cpub *Cpub) {  //PC+2->ACC, B'->PC
    Uword toaddr= get_obj_code(Cpub);
    Cpub->acc= Cpub->pc;
    Cpub->pc= toaddr;
}

void JR(Cpub *Cpub) {  //ACC->PC
    Cpub->pc= Cpub->acc;
}
/* numの値を元にNFとZFを管理 */
void Manage_NF_ZF(Cpub *cpub, int num) {
    if(num == 256 || num == 0) {
        cpub->zf= 1;
    } else {
        cpub->zf= 0;
    }
    if(num < 0 || (num & 0x80) != 0) {
        cpub->nf= 1;
    } else {
        cpub->nf= 0;
    }
}

/* VFを管理 */
void Manage_VF(Cpub *cpub, int num) {
    if(num < -128 || num > 127 && num < 256) {  //符号付きのオーバーフロー管理
        cpub->vf= 1;
    } else {
        cpub->vf= 0;
    }
}

/* CFを管理 */
void Manage_CF(Cpub *cpub, int num) {
    if(num > 255) {  //符号なしのキャリー管理
        cpub->cf= 1;
    } else {
        cpub->cf= 0;
    }
}

/*
   ケースに応じて適切にA,Bに値を格納する
 */
void Store_A_B(int obj, int *A, int *B, Cpub *cpub) {
    if((obj & 0x08) == 0) {
        *A= cpub->acc;
    } else {
        *A= cpub->ix;
    }

    if((obj & 0x07) == 0) {
        *B= cpub->acc;
    } else if((obj & 0x07) == 1) {
        *B= cpub->ix;
    } else if((obj & 0x07) == 2 || (obj & 0x07) == 3) {
        Uword second_obj= get_obj_code(cpub);
        *B= second_obj;
    } else if((obj & 0x07) == 4) {
        Uword second_obj= get_obj_code(cpub);
        *B= cpub->mem[second_obj];
    } else if((obj & 0x07) == 5) {
        Uword second_obj= get_obj_code(cpub);
        *B= cpub->mem[second_obj + 0x100];
    } else if((obj & 0x07) == 6) {
        Uword second_obj= get_obj_code(cpub);
        *B= cpub->mem[cpub->ix + second_obj];
    } else {
        Uword second_obj= get_obj_code(cpub);
        *B= cpub->mem[cpub->ix + second_obj + 0x100];
    }
}

/* numの値を適切な格納先にUword型にcastして格納 */
void storage(int obj, Cpub *cpub, int num) {
    if((obj & 0x8) == 0) {
        cpub->acc= (Uword)num;
    } else {
        cpub->ix= (Uword)num;
    }
}

int A, B;

void ADD(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A + B;
    Manage_NF_ZF(cpub, num);
    Manage_VF(cpub, num);
    storage(obj, cpub, num);
}

void ADC(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A + B + cpub->cf;
    Manage_NF_ZF(cpub, num);
    Manage_VF(cpub, num);
    Manage_CF(cpub, num);
    storage(obj, cpub, num);
    if(num > 255) {
        cpub->cf= 1;
    }
}

void SUB(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A - B;
    Manage_NF_ZF(cpub, num);
    Manage_VF(cpub, num);
    storage(obj, cpub, num);
}

void SBC(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A - B - cpub->cf;
    Manage_NF_ZF(cpub, num);
    Manage_VF(cpub, num);
    Manage_CF(cpub, num);
    storage(obj, cpub, num);
}

void CMP(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A - B;
    Manage_NF_ZF(cpub, num);
}

void AND(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A & B;
    Manage_NF_ZF(cpub, num);
    storage(obj, cpub, num);
    cpub->vf= 0;
}

void OR(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A | B;
    Manage_NF_ZF(cpub, num);
    storage(obj, cpub, num);
    cpub->vf= 0;
}

void EOR(int obj, Cpub *cpub) {
    Store_A_B(obj, &A, &B, cpub);
    num= A ^ B;
    Manage_NF_ZF(cpub, num);
    storage(obj, cpub, num);
    cpub->vf= 0;
}

void Ssm(int obj, Cpub *cpub) {
    int sm= obj & 0x03;
    if(sm == 0) {
        SRA(obj, cpub);
    } else if(sm == 1) {
        SLA(obj, cpub);
    } else if(sm == 2) {
        SRL(obj, cpub);
    } else {
        SLL(obj, cpub);
    }
}

void SRA(int obj, Cpub *cpub) {
    int top= 0;
    if((obj & 0x08) == 0) {
        signed int num= (signed int)cpub->acc;
        if((num & 0x80) != 0) {
            top= 1;
        }
        int bottom= num & 0x01;
        cpub->cf= bottom;  //CF管理
        num= num >> 1;
        if(top == 1) {
            num= num | 0x80;
        }
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        signed int num= (signed int)cpub->ix;
        if((num & 0x80) != 0) {
            top= 1;
        }
        int bottom= num & 0x01;
        cpub->cf= bottom;
        num= num >> 1;
        if(top == 1) {
            num= num | 0x80;
        }
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->vf= 0;
}

void SLA(int obj, Cpub *cpub) {
    if((obj & 0x08) == 0) {
        int num= (int)cpub->acc;
        int top;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        cpub->cf= top;
        num= num << 1;
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        int num= (int)cpub->ix;
        int top;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        cpub->cf= top;
        num= num << 1;
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    Manage_VF(cpub, num);
}

void SRL(int obj, Cpub *cpub) {
    if((obj & 0x08) == 0) {
        unsigned int num= (unsigned int)cpub->acc;
        int bottom= num & 0x01;
        cpub->cf= bottom;
        num= num >> 1;
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        unsigned int num= (unsigned int)cpub->ix;
        int bottom= num & 0x01;
        cpub->cf= bottom;
        num= num >> 1;
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->vf= 0;
}

void SLL(int obj, Cpub *cpub) {
    if((obj & 0x08) == 0) {
        int num= (int)cpub->acc;
        int top;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        cpub->cf= top;
        num= num << 1;
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        int num= (int)cpub->ix;
        int top;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        cpub->cf= top;
        num= num << 1;
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->vf= 0;
}

void Rsm(int obj, Cpub *cpub) {
    int sm= obj & 0x03;
    if(sm == 0) {
        RRA(obj, cpub);
    } else if(sm == 1) {
        RLA(obj, cpub);
    } else if(sm == 2) {
        RRL(obj, cpub);
    } else {
        RLL(obj, cpub);
    }
}

void RRA(int obj, Cpub *cpub) {
    int bottom;
    if((obj & 0x08) == 0) {
        unsigned int num= (unsigned int)cpub->acc;
        bottom= num & 0x01;
        num= num >> 1;
        if(cpub->cf == 1) {
            num= num | 0x80;
        }
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        unsigned int num= (unsigned int)cpub->ix;
        bottom= num & 0x01;
        num= num >> 1;
        if(cpub->cf == 1) {
            num= num | 0x80;
        }
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->cf= bottom;
    cpub->vf= 0;
}

void RLA(int obj, Cpub *cpub) {
    int top;
    if((obj & 0x08) == 0) {
        int num= (int)cpub->acc;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        num= num << 1;
        if(cpub->cf == 1) {
            num= num | 0x01;
        }
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        int num= (int)cpub->ix;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        num= num << 1;
        if(cpub->ix == 1) {
            num= num | 0x01;
        }
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->cf= top;
    Manage_VF(cpub, num);
}

void RRL(int obj, Cpub *cpub) {
    if((obj & 0x08) == 0) {
        unsigned int num= (unsigned int)cpub->acc;
        int bottom= num & 0x01;
        cpub->cf= bottom;
        num= num >> 1;
        if(bottom == 1) {
            num= num | 0x80;
        }
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        unsigned int num= (unsigned int)cpub->ix;
        int bottom= num & 0x01;
        cpub->cf= bottom;
        num= num >> 1;
        if(bottom == 1) {
            num= num | 0x80;
        }
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->vf= 0;
}

void RLL(int obj, Cpub *cpub) {
    int top;
    if((obj & 0x08) == 0) {
        int num= (int)cpub->acc;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        cpub->cf= top;
        num= num << 1;
        if(top == 1) {
            num= num | 0x01;
        }
        Manage_NF_ZF(cpub, num);
        cpub->acc= (Uword)num;
    } else {
        int num= (int)cpub->ix;
        if((num & 0x80) != 0) {  //最上位が0でないなら
            top= 1;
        } else {
            top= 0;
        }
        cpub->cf= top;
        num= num << 1;
        if(top == 1) {
            num= num | 0x01;
        }
        Manage_NF_ZF(cpub, num);
        cpub->ix= (Uword)num;
    }
    cpub->vf= 0;
}
