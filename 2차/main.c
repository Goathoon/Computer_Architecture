#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct _Instruction //Instruction 전체를 한꺼번에 다루기 위한 구조체 선언
{
    char op[7] ; //opernad에 해당하는 문자 eg.addi,slti
    int rs;
    int rt;
    int rd;
    int immValue;
    int address;
    int shamt;
    int offset;
}Instruction;

unsigned int registers[33][2]; //전역변수로 초기화 세로 32번쨰 index는 PC레지스터라 가정.
unsigned int inst_memory[16384][2]; //16384 는 instruction memory의 range를 4로 나눈 값임. 어차피 주소는 4바이트씩 읽으므로, 4로나눈 만큼의 배열공간이 설정되면 된다. 
                                    //1열은 "주소값/4" 를 의미하고, 2열은 각 주소(같은 행의 주소)값에 들어갈 실제 value를 의미한다. 이거는 모두 0xFFFFFFFF으로 초기화시켜주어야함.
unsigned int data_memory[65536][2]; //inst_memory와 달리 1열에는 0x10000000이 +된 값으로 초기화. 여기서는 4로나누면 안됨. 연속적으로 ,data가 주소에 저장되므로.

//----------------bin 에서 inst알아내기 위한 함수-----------//
void R2inst(unsigned int reg,Instruction *inst);
void IJ2inst(unsigned int reg,Instruction *inst);// I,J- TYPE 혼용.
void R2rstd(unsigned int reg,Instruction *inst);
void R2rstd_sa(unsigned int reg,Instruction *inst);
void R2rstd_shift(unsigned int reg,Instruction *inst);
void R2rs(unsigned int reg,Instruction *inst);
void R2rd(unsigned int reg,Instruction *inst);
void R2rst(unsigned int reg,Instruction *inst);
void I2tsi(unsigned int reg,Instruction *inst);
void I2ti(unsigned int reg,Instruction *inst);
void I2branch_2(unsigned int reg,Instruction *inst);
void I2branch(unsigned int reg,Instruction *inst);
void J2address(unsigned int reg,Instruction *inst);
void I2branch_or(unsigned int reg, Instruction *inst);
//--------------------------------------------------------//

void Rinst2reg_Ari(Instruction * inst, unsigned int d[][2]);
void Rinst2reg_Sh(Instruction *inst,unsigned int d[][2]);
void Rinst2reg_jr (Instruction* inst,unsigned int d[][2]);
void Iinst2reg_branch (Instruction *inst,unsigned int d[][2]);
void Iinst2reg_lui (Instruction* inst,unsigned int d[][2]);
void Jinst2reg(Instruction* inst,unsigned int d[][2]);
void Iinst2reg_Mem(Instruction *inst,unsigned int d[][2], unsigned int arr[][2]);
void I2branch_or(unsigned int reg, Instruction *inst);
//-----------------reg에 저장하는 함수 -------------------//
void PrintRegisters (unsigned int d[][2]); //프린트하는 함수
static int data=0;


int main(int argc, char**argv) //argv 인수에 1번째는 읽어드릴 inst의 갯수, 
                               //2번째는 instruction binary file, 3번째는 data(필요에 의하면)
{
    FILE *fp, *fp2;
    fp = fopen(argv[2],"rb"); //argv[1] 에는 불러드릴 변수의 수.
    int N; //불러드릴 instruction의 수
    N = atoi(argv[1]);
    fp2 = fopen(argv[3],"rb");
    if(fp == NULL)
    {
        printf("파일을 찾을 수 없습니다.");
        return 1;
    }
    
    //registers 배열은 2차원배열로, 첫번째 열에는 순차적으로 0~31의 레지스터의 넘버링이 부여되고,
    //두번째 열에는 그 레지스터에 들어있는 value값이 저장된다.
    //후에 결과값을 print할 떄에는 0을 8개의 값으로 도출하게끔, 적절히 변형하여 출력할 예정
    for(int i = 0; i<32; i++)
    {
        registers[i][0] = i; //32개 레지스터 넘버링 부여
    }

    //inst_memory 초기화.
    for (int i = 0; i < 16384; i++)
    {
        inst_memory[i][1] = UINT_MAX; //모두 0xFFFFFFFF으로 초기화시켜주어야함.
    }

    for (int i = 0; i < 65536; i++)
    {
        data_memory[i][0] += ((1<<28) + i); //data_memory 가상의 주소값에 맞게끔 1열에다 더해줌 그리고 추가적으로 주소값 변동 i 더해준다.
        data_memory[i][1] = 255; //모두 0xFF으로 초기화시켜주어야함.
        
    }

    //구조체 포인터 생성. 함수 실행마다, 이 구조체 포인터를 이용하여 값을 저장할 것.
    Instruction *Inst = malloc(sizeof(Instruction));
    
    int bin_arr[8];
    int data_arr[8];
    unsigned int ch1,ch2;
    unsigned int reg1= 0;
    unsigned int reg2= 0;
    int k1 = 0;
    int k2 = 0;
    int i = 0;
    int cur1 = 0;//inst binary 파일을 inst_memory에 저장시키기 위한 변수.
    int cur2 = 0; //data binary 파일을 data_memory에 저장시키기 위한 변수.
    while ((ch1 = fgetc(fp)) != EOF) // 1바이트씩 읽기로하자. 먼저 binary파일을 쭉읽고, instruction memory 에 다저장한 다음에, instruction을 수행할 것임. 지금 이 반복문은 '저장'단계이다.
    {
        bin_arr[k1] = ch1 >> 4;
        bin_arr[++k1] = ch1 % (1 << 4);
        k1++;
        if (k1 == 8)//1바이트읽을때마다 k두번 증가시켰으므로, 총 4바이트 읽은거임.
        {
            for (int j = 0; j < 8; j++)
            {
                reg1 += bin_arr[j] << ((7 - j) * 4);
            }
            inst_memory[cur1][1] = reg1;
            cur1++;
            k1 = 0;
            reg1 = 0;
        }
    }
    if(fp2!=NULL)
    {
        while ((ch2 = fgetc(fp2)) != EOF)
        {
            data_arr[k2] = ch2 >> 4;
            data_arr[++k2] = ch2 % (1 << 4);
            k2++;
            if (k2 == 2) // 1바이트읽을때마다 k두번 증가시켰으므로, 총 4바이트 읽은거임.
            {

                reg2 += (data_arr[0] * 16 );
                reg2 += data_arr[1];

                data_memory[cur2][1] = reg2;
            }
            cur2++;
            k2 = 0;
            reg2 = 0;
        }
    }
    int cnt = 0;
    unsigned int divided_current_address = 0; //PC랑 아예 같은 역할.
    unsigned int data_pc_address = 0 ; //data memory 접근 용이하기위한 변수
    registers[32][1] = 0;
    while (1) // Instruction 수행의 단계
    {

        if(cnt==N)
        {
            if(inst_memory[divided_current_address-1][1] ==UINT_MAX && divided_current_address >=1)
            {
                printf("unknown instruction \n");
                PrintRegisters(registers);    
                break;
            }
            else
            {
                PrintRegisters(registers);
                break;
            }
        }
        else
        {
            if(divided_current_address >=1)
            {

                if (inst_memory[divided_current_address - 1][1] == UINT_MAX)
                {
                    printf("unknown instruction \n");
                    PrintRegisters(registers);
                    break;
                }
            }
        }

        if(inst_memory[divided_current_address][1]/(1<<26)==0) //R-type
        {
            registers[32][1] +=4;
            R2inst(inst_memory[divided_current_address ][1] , Inst);
        }

        else
        {
            registers[32][1] +=4;
            if(inst_memory[divided_current_address ][1]/(1<<26) ==3) //jal일경우
            {
                registers[31][1] = (divided_current_address +1)*4; //$ra에 저장 (PC+4)./m
            }
            IJ2inst(inst_memory[divided_current_address][1],Inst);
        }
        cnt++;
        divided_current_address = registers[32][1]/4;
    }

}


void Rinst2reg_Ari (Instruction *inst,unsigned int d[][2])//기본연산 R-type 의 Instruction을 레지스터에 저장하기 위한 함수
{
    if(!strcmp(inst->op, "add"))
    {
        d[inst->rd][1] = d[inst->rs][1] + d[inst->rt][1];
    }
    
    else if(!strcmp(inst->op, "addu"))
    {
        d[inst->rd][1] = d[inst->rs][1] + d[inst->rt][1];
    }

    else if(!strcmp(inst->op,"sub"))
    {
        d[inst->rd][1] = d[inst->rs][1] - d[inst->rt][1];
    }

    else if(!strcmp(inst->op,"subu"))
    {
        d[inst->rd][1] = d[inst->rs][1] - d[inst->rt][1];
    }

    else if(!strcmp(inst->op,"add"))
    {
        d[inst->rd][1] = d[inst->rs][1] & d[inst->rt][1];
    }

    else if(!strcmp(inst->op,"or"))
    {
        d[inst->rd][1] = d[inst->rs][1] | d[inst->rt][1];
    }

    else if(!strcmp(inst->op,"slt"))
    {
        if((int)d[inst->rs][1] < (int)d[inst->rt][1])
            d[inst->rd][1] = 1;
        else
            d[inst->rd][1] = 0;
    }

    else if(!strcmp(inst->op,"sltu"))
    {
        if((unsigned int)d[inst->rs][1] < (unsigned int)d[inst->rt][1])
            d[inst->rd][1] = 1;
        else
            d[inst->rd][1] = 0;
    }
}

void Rinst2reg_Sh(Instruction* inst,unsigned int d[][2]) //R-type의 shift연산을 위한 함수
{
    if(!strcmp(inst->op,"sll"))
    {
        d[inst->rd][1] = d[inst->rt][1] << inst->shamt;
    }

    else if(!strcmp(inst->op,"srl"))
    {
        d[inst->rd][1] = d[inst->rt][1] >> inst->shamt;
    }
}

void Rinst2reg_jr (Instruction* inst,unsigned int d[][2])
{
    if(!strcmp(inst->op,"jr"))
    {
        d[32][1] = d[(inst->rs)][1]; //PC값 수정
    }
}


void Iinst2reg_Ari (Instruction *inst,unsigned int d[][2])
{
    if(!strcmp(inst->op,"addi"))
    {
        d[inst->rt][1] = d[inst->rs][1] + (short)inst->immValue;
        if(d[inst->rt][1]/(1<<15)==1) //sign extension
        {
            for(int i = 0; i<16; i++)
            {
                d[inst->rt][1] += 1<<(16+i);
            }

        }
    }

    else if(!strcmp(inst->op,"addiu"))
    {
        d[inst->rt][1] = d[inst->rs][1] + (short)inst->immValue;
    }
    else if(!strcmp(inst->op,"andi"))
    {
        d[inst->rt][1] = d[inst->rs][1] & inst->immValue;
    }
}

void Iinst2reg_branch (Instruction *inst,unsigned int d[][2])
{
    if(!strcmp(inst->op, "ori"))
    {
        d[inst->rt][1] = d[inst->rs][1] | inst->offset;
    }
    else if(!strcmp(inst->op,"slti"))
    {
        if(d[inst->rs][1] < (inst->offset))
        {
            d[inst->rt][1] = 1;
        }
        else   
            d[inst->rt][1] = 0;
    }
    else if(!strcmp(inst->op,"sltiu"))
    {
        if((unsigned int)d[inst->rs][1] < (unsigned int)(inst->offset))
        {
            d[inst->rt][1] = 1;
        }
        else
            d[inst->rt][1] = 0;
    }

    else if(!strcmp(inst->op,"beq"))
    {
        if(d[inst->rs][1] == d[inst->rt][1])
        {
            d[32][1] = 4*(inst->offset) + (d[32][1]);
        }
    }

    else if(!strcmp(inst->op,"bne"))
    {
        if (d[inst->rs][1] != d[inst->rt][1])
        {
            d[32][1] = 4*(inst->offset) +(d[32][1]);
        }
    }

}

void Iinst2reg_lui (Instruction* inst,unsigned int d[][2])
{
    if(!strcmp(inst->op,"lui"))
    {
        d[inst->rt][1] = inst->immValue<<16;      
    }
}

void Jinst2reg(Instruction* inst,unsigned int d[][2])
{
    if(!strcmp(inst->op, "j"))
    {
        d[32][1] = inst->address;
    }

    else if(!strcmp(inst->op, "jal"))
    {
        d[32][1] = inst->address;//문제가생길것 미리 처리 필요 main에서 돌아올 주소를 저장해야하므로.
    }
}

void Iinst2reg_Mem(Instruction *inst,unsigned int d[][2], unsigned int arr[][2])
{
    if(!strcmp(inst->op, "lw"))
    {
        int num1=0;
        for(int i = 0; i<65536; i++)
        {
            if(inst->offset + d[inst->rs][1] == arr[i][0])
            {
                num1 = i; //찾는 주소값이 있있는 가로 index 탐색.
                break;
            }
        }
        d[inst->rt][1] = 0;
        for(int j = 0; j<4; j++)
        {
            d[inst->rt][1] += arr[num1++][1] <<4*(6-2*j);

        }

    }
    else if(!strcmp(inst->op, "sw"))
    {
        int num2=0;
        for (int i = 0; i < 65536; i++)
        {
            if (inst->offset + d[inst->rs][1] == arr[i][0])
            {
                num2 = i; //찾는 주소값이 있있는 가로 index 탐색.
                break;
            }
        }
        arr[num2++][1] = d[inst->rt][1]/(1<<24);
        arr[num2++][1] = (d[inst->rt][1]%(1<<24))/(1<<16);
        arr[num2++][1] = (d[inst->rt][1]%(1<<16))/(1<<8);
        arr[num2++][1] = (d[inst->rt][1]%(1<<8));
    }
}


void PrintRegisters(unsigned int d[][2])
{
    for(int i = 0 ; i<32; i++) 
    {
        printf("$%d: 0x%08x\n", d[i][0],d[i][1]);
    }
    printf("PC: 0x%08x\n",d[32][1]);

}

/////////////////////////////////////////////////////////////////////
///////////////////////이전에 했던 함수 그대로 이용//////////////////
////////////////////////////////////////////////////////////////////
//모든 과정은 결국 Inst구조체에 op ,rd rs rt offset 등등 맞는 value 값들을 one to one으로 저장하게함
void R2inst(unsigned int reg, Instruction *inst) //R-type 은 아니지만 R- type으로 여겨지는 jr과 jalr이 존재
{
    switch(reg % (1<<6)){ //func 비트에 따라서 instruction 결정
        case 32: //add
            strcpy(inst->op,"add");
            R2rstd(reg,inst);//instruction 구조체 값 지정해주는 함수임. 이하동문
            Rinst2reg_Ari(inst, registers);
            break;
        case 33:
            strcpy(inst->op,"addu"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst, registers);
            break;
        case 36:
            strcpy(inst->op,"and"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst, registers);
            break;
        case 26:
            strcpy(inst->op,"div");
            R2rstd(reg,inst);
            break;
        case 27:
            strcpy(inst->op,"divu");
            R2rstd(reg,inst);
            break;
        case 9:
            strcpy(inst->op,"jalr");
            R2rs(reg,inst);
            break;
        case 8:
            strcpy(inst->op,"jr");
            R2rs(reg,inst);
            Rinst2reg_jr(inst,registers);
            break;
        case 16:
            strcpy(inst->op,"mfhi");
            R2rs(reg,inst);
            break;
        case 18:
            strcpy(inst->op,"mflo");
            R2rd(reg,inst);
            break;
        case 17:
            strcpy(inst->op,"mthi");
            R2rs(reg,inst);
            break;
        case 19:
            strcpy(inst->op,"mtlo");
            R2rs(reg,inst);
            break;
        case 24:
            strcpy(inst->op,"mult");
            R2rst(reg,inst);
            break;
        case 25:
            strcpy(inst->op,"multu");
            R2rst(reg,inst);
            break;
        case 39:
            strcpy(inst->op,"nor");
            R2rstd(reg,inst);
            break;
        case 37:
            strcpy(inst->op,"or"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst, registers);
            break;
        case 0:
            strcpy(inst->op,"sll");
            R2rstd_sa(reg,inst);
            Rinst2reg_Sh(inst, registers);
            break;
        case 4:
            strcpy(inst->op,"sllv");
            R2rstd_shift(reg,inst);
            break;
        case 42:
            strcpy(inst->op,"slt"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst, registers);
            break;
        case 43:
            strcpy(inst->op,"sltu"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst, registers);
            break;
        case 3:
            strcpy(inst->op,"sra");
            R2rstd_sa(reg,inst);
            break;
        case 7:
            strcpy(inst->op,"srav");
            R2rstd_shift(reg,inst);
            break;
        case 2:
            strcpy(inst->op,"srl");
            R2rstd_sa(reg,inst);
            Rinst2reg_Sh(inst, registers);
            break;
        case 6:
            strcpy(inst->op,"srlv");
            R2rstd_shift(reg,inst);
            break;
        case 34:
            strcpy(inst->op,"sub"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst,registers);
            break;
        case 35:
            strcpy(inst->op,"subu"); 
            R2rstd(reg,inst);
            Rinst2reg_Ari(inst,registers);
            break;
        case 12:
            printf("syscall");
            break;
        case 38:
            strcpy(inst->op,"xor");
            R2rstd(reg,inst);
            break;
        default :
            //printf("unknown instruction\n");
            break;
    }
}

void IJ2inst(unsigned int reg, Instruction *inst)
{
    switch(reg/(1<<26)){
        case 8:
            strcpy(inst->op,"addi");
            I2tsi(reg,inst);
            Iinst2reg_Ari(inst,registers);
            break;
        case 9:
            strcpy(inst->op,"addiu");
            I2tsi(reg,inst);
            Iinst2reg_Ari(inst,registers);
            break;
        case 12:
            strcpy(inst->op,"andi");
            I2tsi(reg,inst);
            Iinst2reg_Ari(inst,registers);
            break;
        case 4:
            strcpy(inst->op,"beq");
            I2branch_2(reg,inst);
            Iinst2reg_branch(inst,registers);
            break;
        case 5:
            strcpy(inst->op,"bne");
            I2branch_2(reg,inst);
            Iinst2reg_branch(inst,registers);
            break;  
        case 32:
            strcpy(inst->op,"lb");
            I2branch(reg,inst);
            break;
        case 36:
            strcpy(inst->op,"lbu");
            I2branch(reg,inst);
            break;
        case 33:
            strcpy(inst->op,"lh");
            I2branch(reg,inst);
            break;
        case 37:
            strcpy(inst->op,"lhu");
            I2branch(reg,inst);
            break;
        case 15:
            strcpy(inst->op,"lui");
            I2ti(reg,inst);
            Iinst2reg_lui (inst,registers);
            break;
        case 35:
            strcpy(inst->op,"lw");
            I2branch(reg,inst);
            Iinst2reg_Mem(inst, registers,data_memory);
            break;
        case 13:
            strcpy(inst->op,"ori");
            I2branch_or(reg,inst);
            Iinst2reg_branch(inst,registers);
            break;
        case 40:
            strcpy(inst->op,"sb");
            I2branch(reg,inst);
            break;
        case 10:
            strcpy(inst->op,"slti");
            I2branch_2(reg,inst);
            Iinst2reg_branch(inst,registers);
            break;
        case 11:
            strcpy(inst->op,"sltiu");
            I2branch_2(reg,inst);
            Iinst2reg_branch(inst,registers);
            break;
        case 41:
            strcpy(inst->op,"sh");
            I2branch(reg,inst);
            break;
        case 43:
            strcpy(inst->op,"sw");
            I2branch(reg,inst);
            Iinst2reg_Mem(inst, registers,data_memory);
            break;
        case 14:
            strcpy(inst->op,"xori");
            I2branch_2(reg,inst);
            break;
        // 여기서부터 j
        case 2:
            strcpy(inst->op,"j");
            J2address(reg,inst);
            Jinst2reg(inst, registers);
            break;
        case 3:
            strcpy(inst->op,"jal");
            J2address(reg, inst);
            Jinst2reg(inst, registers);
            break;
        default:
            //printf("unknown instruction\n");
            break;
    }

}       


void R2rstd(unsigned int reg, Instruction *inst)
{
    int all_r; //레지스터 위치에 해당하는 비트들을 정수로 표현 (shamt, funct, opcode 부분은 제외하고 생각하였다.)
    int rd;
    int rs;
    int rt;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rd = all_r % (1<<5);
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) / (1<<5);

    inst->rd = rd;
    inst->rs = rs;
    inst->rt = rt;

}

void R2rst(unsigned int reg,Instruction *inst)
{
    int all_r;
    int rs;
    int rt;

    all_r = (reg%(1<<26)) >> 16;
    rt = all_r % (1<<5);
    rs = (all_r>>5) / (1<<5);
    
    inst->rt = rt;
    inst->rs = rs;
}

void R2rs(unsigned int reg , Instruction *inst)
{
    int all_r;
    int rs;
    int rd;

    all_r = (reg%(1<<26)) >> 16;
    rs = (all_r >> 5);

    inst->rs = rs;
}

void R2rd(unsigned int reg, Instruction *inst)
{
    int all_r;
    int rs;
    int rd;

    all_r = (reg%(1<<26)) >> 16;
    rd = all_r % (1 << 5);

    inst->rd = rd;
}

void R2rstd_sa(unsigned int reg, Instruction *inst)
{
    int all_r; //레지스터 위치에 해당하는 비트들을 정수로 표현 (shamt, funct, opcode 부분은 제외하고 생각하였다.)
    int rd;
    int sa;
    int rt;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rd = all_r % (1<<5);
    rt = (all_r>>5) % (1<<5);
    sa = (reg % (1<<11)) / (1<<6); //shamt (shift amount)

    inst -> rd= rd;
    inst -> rt= rt;
    inst -> shamt = sa;
}

void R2rstd_shift(unsigned int reg, Instruction *inst)
{
    int all_r; //레지스터 위치에 해당하는 비트들을 정수로 표현 (shamt, funct, opcode 부분은 제외하고 생각하였다.)
    int rd;
    int rs;
    int rt;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rd = all_r % (1<<5);
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) / (1<<5);

    inst->rd= rd;
    inst->rt= rt;
    inst->rs= rs;
}

void I2tsi(unsigned int reg, Instruction *inst)
{
    int all_r;
    int rs;
    int rt;
    int imm;
    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) / (1<<5);
    imm = (reg%(1<<16));
    inst->rt = rt;
    inst->rs = rs;
    inst->immValue =imm;
}

void I2ti(unsigned int reg, Instruction *inst)
{
    int all_r;
    int rt;
    int imm;

    all_r =((reg % (1<<26))>>16);
    rt = (all_r)%(1<<5);
    imm = reg % (1<<16);

    inst->rt = rt;
    inst->immValue = (unsigned int)imm;
}

void I2branch_2(unsigned int reg, Instruction *inst)
{
    int all_r;
    int rs;
    int rt;
    int offset;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) / (1<<5);
    offset = (reg%(1<<16));
    /* if (offset / (1 << 15) == 1) // sign extension //sample 5 떄문에 고침
    {
        for (int i = 0; i < 16; i++)
        {
            offset += 1 << (16 + i);
        }
    } */

    inst->rt = rt;
    inst->rs = rs;
    inst->offset = (short) offset;
}

void I2branch_or(unsigned int reg, Instruction * inst)
{
    int all_r;
    int rs;
    int rt;
    unsigned int offset;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) / (1<<5);
    offset = (reg%(1<<16));

    inst->rt = rt;
    inst->rs = rs;
    inst->offset = offset;
}

void I2branch(unsigned int reg, Instruction *inst)
{
    int all_r;
    int rs;
    int rt;
    int offset;

    all_r = (reg % (1 << 26)) >> 11; // opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r >> 5) % (1 << 5);
    rs = (all_r >> 5) / (1 << 5);
    offset = (reg % (1 << 16));

    inst->rt= rt;
    inst->rs= rs;
    if(offset/ (1<<15) ==1)
    {
        for(int i = 0; i<16; i++)
        {
            offset += 1<<(16+i);
        }
    }   
    inst->offset = offset;
}

void J2address(unsigned int reg, Instruction *inst)
{
    int address;

    address = reg%(1<<26);
    address *=4;
    address += (reg/(1<<28))<<28; 
    inst->address = address;
}
