#pragma once




#include<vector>
#include"json/json.h"



//struct PointerMap
//{
//	void* baseAddr;
//	std::vector<int> offset;
//
//	<template T>
//		T& GetValue() {};
//};



//基地址跨版本定位:
//代码段寻找对基地址的引用
//对引用进行特征码定位
// 
//可以依次对对每个偏移值也进行特征码定位吗？


class AddressTable
{

	

	
public:
	static bool LoadFromFile();

	static void* GetAddress(const char* addrName);
	std::vector<int> GetOffsetList(const char* offsetName);
	std::string_view GetCSPVersion() { return cspVersion; };
	

private:

	Json::Value addressMap;
	std::string cspVersion;

};