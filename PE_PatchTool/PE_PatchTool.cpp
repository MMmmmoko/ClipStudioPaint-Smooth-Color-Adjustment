// ImportAdder.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

//CSP的正版检测目前看来像是一段宏插进代码的，而不是写在函数内的。
//其有如下特征



//00000001403AE6B9 | E8 52941B03 | call <clipstudiopaint.checkData> |
// 
// 
// 
// CODE PART 2
//00000001403AE6BE | 84C0 | test al, al |
//00000001403AE6C0 | 74 1B | je clipstudiopaint.1403AE6DD |
// 
// 
// 
// 
//00000001403AE6C2 | E8 E9BE1203 | call <clipstudiopaint.valueof(1450209B8)> |
// 
// 
// 
//  CODE PART 3
//00000001403AE6C7 | 48 : 85C0 | test rax, rax |
//00000001403AE6CA | 74 11 | je clipstudiopaint.1403AE6DD |
// 
// 
// 
// 
//00000001403AE6CC | E8 DFBE1203 | call <clipstudiopaint.valueof(1450209B8)> |
// 
// 
//00000001403AE6D1 | 48 : 8BC8 | mov rcx, rax |
// 
//00000001403AE6D4 | E8 F7F61203 | call <clipstudiopaint.重要正版检测函数？> |
//00000001403AE6D9 | 85C0 | test eax, eax |
// 
// 
// 
// 检查eax=1之后的逻辑
// 
//////**********************00000001403AE6DB | 75 62 | jne clipstudiopaint.1403AE73F |

//这里尝试以匹配、修改的方法将这些代码移除



uint8_t* Posstart = NULL;
bool MatchCallValueOf1450209b8(uint8_t* pos)
{

    if (*pos!= 0xe8)return false;

    const uint32_t PosOfCall = 0x1434da5B0&0xFFFFFFFF;
    uint32_t addrOfCurrentPos = ((uintptr_t)(pos - Posstart) - 0x400 + 0x140001000) & 0xFFFFFFFF;


    uint32_t offset = PosOfCall - addrOfCurrentPos-5;
    if (*(uint32_t*)(pos + 1) != offset)
        return false;
    return true;
}


bool MatchCallCheckData(uint8_t* pos)
{

    if (*pos!= 0xe8)return false;

    const uint32_t PosOfCall = 0x143567B10&0xFFFFFFFF;
    uint32_t addrOfCurrentPos = ((uintptr_t)(pos - Posstart) - 0x400 + 0x140001000) & 0xFFFFFFFF;


    uint32_t offset = PosOfCall - addrOfCurrentPos - 5;
    if (*(uint32_t*)(pos + 1) != offset)
        return false;
    return true;
}

bool MatchCallCheckLicense(uint8_t* pos)
{

    if (*pos!= 0xe8)return false;

    const uint32_t PosOfCall = 0x1434DDDD0&0xFFFFFFFF;
    uint32_t addrOfCurrentPos = ((uintptr_t)(pos - Posstart) - 0x400 + 0x140001000) & 0xFFFFFFFF;


    uint32_t offset = PosOfCall - addrOfCurrentPos - 5;
    if (*(uint32_t*)(pos + 1) != offset)
        return false;
    return true;
}


bool MatchCodePart1(uint8_t* pos)
{
    // CODE PART 1
    //00000001403AE6AE | 48 : 8D15 D327A304 | lea rdx, qword ptr ds : [144DE0E88] |
    //00000001403AE6B5 | 48 : 8D4D F0 | lea rcx, qword ptr ss : [rbp - 10] |
    // 

   //第一行不进行匹配
    if(*pos!=0x48)return false;
    if(*(pos+7)!=0x48)return false;

    

    return true;
}


bool MatchCodePart2(uint8_t* pos)
{
    // CODE PART 2
    //00000001403AE6BE | 84C0 | test al, al |
    //00000001403AE6C0 | 74 1B | je clipstudiopaint.1403AE6DD |



    
    uint8_t bytes[3] = { 0x84,0xC0,0x74/*,0x1B*/};
    if (memcmp(pos, bytes, 3) != 0)
        return false;

    return true;
}
bool MatchCodePart3(uint8_t* pos)
{
    //  CODE PART 3
    //00000001403AE6C7 | 48 : 85C0 | test rax, rax |
    //00000001403AE6CA | 74 11 | je clipstudiopaint.1403AE6DD |


    
    uint8_t bytes[4] = { 0x48,0x85,0xC0,0x74/*,0x11*/};
    if (memcmp(pos, bytes, 4) != 0)
        return false;

    return true;
}



bool MatchContent(uint8_t* pos)
{

    //if (!MatchCodePart1(pos))return false;
    //pos += (0xb9-0xAE);
    if (!MatchCallCheckData(pos))return false;
    pos += 5;
    if (!MatchCodePart2(pos))return false;
    pos += 4;
    if (!MatchCallValueOf1450209b8(pos))return false;
    pos += 5;
    if (!MatchCodePart3(pos))return false;
    pos += 5;
    if (!MatchCallValueOf1450209b8(pos))return false;
    pos += 5;



    uint8_t bytes[3] = { 0x48,0x8b,0xC8};
    if (memcmp(pos, bytes, 3) != 0)
        return false;
    pos += 3;
    if (!MatchCallCheckLicense(pos))return false;
    pos += 5;
    uint8_t bytes2[2] = { 0x85,0xc0};
    if (memcmp(pos, bytes2, 2) != 0)
        return false;

    return true;
}

void PatchCode(uint8_t* pos)
{
    //旧实现 发现部分尾部逻辑有变化（比如复制图层到剪贴板），所以应该跳转的不是最后跳转指令的目标，
    // 而是跳转到正版验证函数后，这样可以顺着正版验证为真的逻辑继续运行
    //uint32_t targetPos =(0x6db - 0x6b9)+2+ *(pos + (0x6db - 0x6b9 + 1));
    //*pos = 0xe9;//jmp
    //*(uint32_t*)(pos + 1) = targetPos - 5;
    ////*(pos + 1+4) = 0x90;
    ////*(pos + 1+5) = 0x90;

    *pos = 0xeb;//jmp
    *(pos+1) = 0x19;//jmp的偏移
    //填充0x19字节
    for (int i = 0; i < 0X19; i++)
    {
        uint8_t* curPos = pos + 2 + i;
        *curPos = 0X90;
    }
    //在test eax,eax之前，mov eax，1
    uint8_t* curPos = pos + 0x1B;
    uint8_t op_mov_eax_1[5] = {0xb8,0x01,0x00,0x00,0x00};
    memcpy(curPos, op_mov_eax_1,sizeof(op_mov_eax_1));

    return;
}


int main(int argc, char* argv[])
{
    std::string filePath = "G:\\CSPMOD\\x64\\Debug\\CLIPStudioPaint.exe";
    std::string fileOutPath = "G:\\CSPMOD\\x64\\Debug\\CLIPStudioPaint_PATCH.exe";


    // 1. 读取文件内容到内存
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        std::cerr << "无法打开输入文件" << std::endl;
        return 0;
    }

    // 获取文件大小
    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // 读取文件内容
    std::vector<char> fileData(fileSize);
    inFile.read(fileData.data(), fileSize);
    inFile.close();


    Posstart = (uint8_t*)fileData.data();

    //用工具查找到代码段位置为0x400 总尺寸3EE9000

    for (int i = 400; i < 0x3EE9000; i++)
    {
        uint8_t* curFilePos = (uint8_t*) & fileData[i];

        if (MatchContent(curFilePos))
        {
            printf("succeess Find Point at %p\n", curFilePos);
            PatchCode(curFilePos);
        }


    }


    //保存文件
    std::ofstream outFile(fileOutPath, std::ios::binary | std::ios::trunc);
    if (!outFile) {
        std::cerr << "无法保存文件" << std::endl;
        return 0;
    }
    outFile.write(fileData.data(), fileSize);
    outFile.close();
    std::cout << "文件修改成功" << std::endl;


	system("pause");

	return 0;
}

