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

**SUMMARY**

MIPS 아키텍쳐의 instruction에 해당하는 바이너리 파일을 읽어서 assembly어로 출력하는 것이 이 과제의 목표이다.<br/>
따라서, 32비트씩 binary 파일을 읽어와서, int 배열에 저장해놓고 ('bin_arr[8]') 해당 배열에서 이러쿵 저러쿵 하면 쉽게 구현할 수 있다.<br />
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
reg에 앞 6비트 정보를 토대로 무슨 instruction인지 프린트해내고, 각 instruction에 맞는 공통된 instruction 구조로 함수를 정의하여 구현했다.

