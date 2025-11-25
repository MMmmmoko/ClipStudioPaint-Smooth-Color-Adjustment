#pragma once
#include<vector>
#include<string>

#define BYTEWILDCARD 0XCC

//https://blog.csdn.net/qiming_zhang/article/details/7309909#5.2
struct PESectiuonHead
{
	char name[8];
	uint32_t virtualAddress;
	uint32_t virtualSize;
	uint32_t sizeOfRawData;
	uint32_t pointerToRawData;
};



class DllInject
{
	friend class AddressGenerator;
public:

	enum EXETYPE
	{
		CSP,
		UDMPRO,
		UDMEX
	};

	bool SetUpPE(uint8_t* exeFileMem, size_t exeFileSize, EXETYPE exeType);

	size_t RVA2FOA(size_t RVA);
	size_t FOA2RVA(size_t FOA);



	bool Patch();

	void SetRandomAddrDisable();
	bool InjectDll();






	std::vector<std::string> GetDllList();


private:


	uint32_t PEHeadPos;
	uint32_t NT_Part1StartPos;
	uint32_t NT_Part23StartPos;
	uint64_t imageBase;
	uint32_t fileAlignment;//文件对齐
	uint32_t sectionAlignment;//内存对齐






	std::vector<PESectiuonHead> sectionheads;
	std::vector<uint8_t>newExeFileMem;




	uint8_t* _exeFileMem=nullptr;
	size_t _exeFileSize=0;
	EXETYPE _exeType = CSP;


};