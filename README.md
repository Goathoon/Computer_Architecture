# Computer_Architecture
2022-1 조형민 교수님 컴퓨터구조 개론
<hr/>

## 1차과제

**목표 : 해당 과제는 MIPS machine 코드 (binary) 를 disassemble 하여 assembly언어로 해석하는 것을 목표로 하고 있다.**

1. instruction name은 모두 소문자로 할것
2. register 들은 특정 이름이 아닌 숫자로 할것
3. 실제 주소값들은 모두 signed decimal로 할것
4. 구현해야할 instruction들 : 
add, addu , and, div, divu , jalr , jr , mfhi , mflo , mthi ,
mtlo , mult , multu , nor, or, sll , sllv , slt , sltu , sra ,
srav , srl , srlv , sub, subu , syscall , xor , addi , addiu ,
andi , beq , bne , lb , lbu , lh , lhu , lui , lw , ori , sb, slti ,
sltiu , sh , sw , xori , j, jal instruction 만 있다고 가정
5. 만일 해당 instruction에서 벗어난 binary 코드가 발견될 시에, unknown instruction 출력

### SUMMARY

MIPS 아키텍쳐의 instruction에 해당하는 바이너리 파일을 읽어서 assembly어로 출력하는 것이 이 과제의 목표이다.<br/>
따라서, 32비트씩 binary 파일을 읽어와서, int 배열에 저장해놓고 ('bin_arr[8]') 해당 배열에서 이러쿵 저러쿵 하면 쉽게 구현할 수 있다. (물론 메모리 낭비가 있다.)<br />
하지만, opcode 를 해석하기 위해서는 앞 6바이트가 필요하다. 따라서 해당 6바이트를 위해서 약간의 변조가 필요했다.<br/>
```c
(bin_arr[0]<<2)+ (bin_arr[1]>>2)
```
다음과 같은 코드를 이용하여 opcode를 추출해낼 수 있었고, 해당 opcode로 R type인지 I type인지 구분해낸다.<br/>
그 후, 나머지 26비트로 R type 내에서 or I type 내에서 instruction을 분석한다.<br/>
해당 함수로 
```c
R2inst(reg);
I2inst(reg);
```
를 선언하여 해석했다.<br/>
현재 reg는 int형 이므로 4바이트 즉, 32비트이다. <br>
reg에 들어갈 정보는
```c
for(int i = 0; i<8; i++)
        {
            fscanf(bf,"%1d",&bin_arr[i]); //바이너리 파일의 숫자들을 4byte 씩 끊어서 각 숫자들을 배열에 저장 ->그래야 instruction 따질 수 있음 4바이트씩이니까
            reg += bin_arr[i] <<((7-i)*4) ;
        }
```
이렇게 바이너리 파일을 4바이트씩 끊어서 각 숫자들을 배열에 저장한 후, 16로 저장되어 있는 bin_arr를 int 형 변수 reg 로 변환시켰다. <br/>
그 후에, reg에 앞 6비트 정보 = opcode (I type) 혹은 뒤 6비트 정보 = func(R type)를 토대로 무슨 instruction인지 프린트해내고, 각 instruction에 맞는 공통된 instruction 구조로 함수를 정의하여 구현했다.
<hr/>

## 2차과제

**목표 : 해당 과제는 MIPS instruction 들의 일부를 지원하는 instruction simulator를 만드는 과제이다.**

1. instruction들을 읽고 직접 실행시킬 것 (registrt value, memory contents의 변화만 고려한다.)
2. 프로그램 마지막에는 register들의 현재 value값들을 출력 시킬 것
3. 구현해야할 instruction 들: <br/>
  - Arithmetic/logical: add, addu , sub, subu , and, or, slt , sltu <br/>
  - Arithmetic/logical with immediate: addi , addiu , andi , ori , lui , slti , sltiu <br/>
  - Control transfer: beq , bne , j, jal , jr <br/>
  - Shift instructions: sll , srl <br/>
  - Memory access: lw , sw <br/>
  - No need to implement lh , lhu , lb , sh , shu , sb <br/>

4. add, sub, addu, subu, addiu, addi 에서 signed unsigned를 구분 지을 필요는 없다.
5. 다만, slt, slti instruction에서 는 구분 지을 것.

#### 구현해야할 data structure

- Register
모든 레지스터들의 초기 값들은 0x00000000 이다.
레지스터들은 모두 $0 ~ $31이라고 가정한다. (NAME 부여하지 않음)

- Instruction Memory
주소값 범위는 0x00000000 ~ 0x00010000 (4KB)라고 가정한다.
모든 data 들은 0xFF라고 가정한다. (word 같은 경우 0xFFFFFFFF)

- Data Memory
실제 CPU와 달리, Data Memory는 Instruction Memory와 분리되어 있다고 가정한다.
주소값 범위는 0x10000000 ~ 0x100100000 (4KB) 라고 가정한다.

#### 실행 방법
1. 2 ~ 3 개의 Command Line을 인자로 입력받는다.
2. 첫번째 인자에는 실행시킬 instruction들의 수
3. 두번째 인자에는 instruction 확장자인 binary 파일 이름
4. optional 인자로, data memory의 값들을 저장시킨 data 파일 (시작 주소는 0x10000000 임에 유의)

### SUMMARY

Instruction 전체를 한꺼번에 다루기 위해서, Instruction 구조체를 다음과 같이 선언했다.
```c
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
```
그리고, register 에 value들을 저장할 배열, 그리고 instruction memory, data memory를 구현해야만 했다.
```c
unsigned int registers[33][2]; //전역변수로 초기화 세로 32번쨰 index는 PC레지스터라 가정.
unsigned int inst_memory[16384][2]; //16384 는 instruction memory의 range를 4로 나눈 값임. 어차피 주소는 4바이트씩 읽으므로, 4로나눈 만큼의 배열공간이 설정되면 된다. 
                                    //1열은 "주소값/4" 를 의미하고, 2열은 각 주소(같은 행의 주소)값에 들어갈 실제 value를 의미한다. 이거는 모두 0xFFFFFFFF으로 초기화시켜주어야함.
unsigned int data_memory[65536][2]; //inst_memory와 달리 1열에는 0x10000000이 +된 값으로 초기화. 여기서는 4로나누면 안됨. 연속적으로 ,data가 주소에 저장되므로.
```
다음은 초기화 과정이다.
```c
    //registers 배열은 2차원배열로, 첫번째 열에는 순차적으로 0~31의 레지스터의 넘버링이 부여되고, (32번째는 PC register = 다음 fetch 될 instruction 주소 가리킴)
    //두번째 열에는 그 레지스터에 들어있는 value값이 저장된다.
    //후에 결과값을 print할 때에는 0을 8개의 값으로 도출하게끔, 적절히 변형하여 출력할 예정
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
        data_memory[i][0] += ((1<<28) + i); //data_memory 가상의 주소값(0x10000000이 초기값임에 유의)에 맞게끔 1열에다 더해줌 그리고 추가적으로 주소값 변동 i 더해준다.
        data_memory[i][1] = 255; //모두 0xFF으로 초기화시켜주어야함.
        
    }
```
다음은 각각 Instruction Memory와 Data Memory에 인자로 받은 binary .inst 파일과 .data 파일에서 값을 저장하는 과정이다.<br/>
물론 여기에서 instruction 그리고 data 들을 int형 배열 bin_arr, data_arr 에 저장해서 메모리가 많이 낭비된다. 하지만 당시에는 구현에 어려움이 있어서, 메모리 낭비를 감안했다. <br/>
그러므로 bin_arr 에서 각 원소들이 실질적으로 4바이트 공간을 차지하지만, 구현 의도는 각 원소들을 4비트를 담는다고 가정하자.
```c
// Instruction Memory 저장 과정
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
    
    // Data Memory 저장 과정
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
```

이후에는, N개의 입력받은 instruction을 수행한 경우에, 약간의 예외 처리와 함께 register 값들을 출력하는 과정을 거친다. <br/>
N개의 instruction을 수행하는 과정에서는 PC register와 동일한 역할을 하는 divided_current_address 변수를 적절히 조절하여 과정을 진행한다. <br/>
기본적인 알고리즘 순서는, divided_current_address index에 존재하는 inst_memory (기억하자, inst_memory의 index는 기본적으로 주소값을 4로 나눈값임을) value값을 통해 Instruction type을 fetch 한 후, 해당 instruction을 수행한다. <br/>
이 때 만약 jal과 같은 경우에 $ra 레지스터 (이 프로젝트에서는 $31로 지정) 에 다음 명령어의 주소를 저장한다. <br/>
그렇지 않은 경우에는 1차과제와 마찬가지로, 해당 instruction에 맞게끔 각각 opcode, shamt, immediate value 등과 같은 값들을 Instruction 구조체를 이용하여 저장시킨다. <br/>
저장된 구조체를 이용하여 register value 혹은 memory value들을 바꾸는 알고리즘을 적용시켰다.

