#ifndef Util_h
#define Util_h

#include"json/json.h"


class Pack;




namespace util {


	//使用SDL的文件接口，所以需要调用SDL_free
	unsigned char* SDL_LoadFileToMem(const char* path,size_t* size);
	void SDL_FreeMem(unsigned char* mem);
	
	//从内存读json
	Json::Value BuildJsonFromMem(const char* mem, size_t memSize);
	Json::Value BuildJsonFromFile(const char* filePath);
	bool SaveJsonToFile(const Json::Value& json, const char* filePath);

	//返回pack中某文件夹的模型类型与文件名

	//std::string GetModelFilePathInPack(Pack* pack,const char* foldPathInPack, );
	bool IsStringEndsWith(const std::string& str,const char* end);
	bool IsStringStartWith(const std::string& str,const char* end);

	std::string StringToUpper(const std::string& str);

	
	std::string& ReplaceString(std::string& org,const char* strToBeReplace,const char* newstr);
	std::string GetFileNameFromPath(const std::string& path);
	std::string RemoveExtension(const std::string& path);
	void AppendSeparator(std::string& path);
	bool PathEqual(const std::string& path1, const std::string& path2);


	//JSON中按语言读取字符串，有fullback，无法读取返回"",需要判断json是否为空串
	std::string GetStringFromMultiLangJsonNode(const Json::Value& json);

	
	inline float NanoTicksToSeconds(uint64_t ticks);

}


//工具宏
#define UTIL_IS_CSTR_EMPTY(cstr) (!cstr||0==SDL_strcmp(cstr,""))
#define UTIL_SETLOW32VALUE(dest,value32) *(uint32_t*)(&dest)=static_cast<uint32_t>(value32)
#define UTIL_SETHIGH32VALUE(dest,value32) ((uint32_t*)(&dest))[1]=static_cast<uint32_t>(value32)
#define UTIL_GETLOW32VALUE(dest) (*(uint32_t*)(&dest))
#define UTIL_GETHIGH32VALUE(dest) (((uint32_t*)(&dest))[1])

//保证字符串以分隔符结尾
#define UTIL_FOLDER_PATH_WITH_SEPARATOR(cstrDir) \
((cstrDir[SDL_strlen(cstrDir)-2]=='/'||cstrDir[SDL_strlen(cstrDir)-2]=='\\')?(cstrDir):((std::string(cstrDir)+"/").c_str()))





//内联实现
inline float util::NanoTicksToSeconds(uint64_t ticks)
{
	return static_cast<float>(ticks) / 1'000'000'000.0f;
}



#endif