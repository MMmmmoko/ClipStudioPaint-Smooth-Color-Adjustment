#include<SDL3/SDL.h>
#include<fstream>
#include"Util/Util.h"


#pragma comment(lib, "SDL3.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "imm32.lib")


unsigned char* util::SDL_LoadFileToMem(const char* path, size_t* size)
{
    //通过SDL加载文件
    SDL_IOStream* modelFileStream = SDL_IOFromFile(path, "r");
    if (!modelFileStream)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Open File Failed: %s", path);
        *size = 0;
        return NULL;
    }
    uint64_t _size = SDL_GetIOSize(modelFileStream);
    void* buffer = SDL_malloc(_size);
    SDL_ReadIO(modelFileStream, buffer, _size);

    SDL_CloseIO(modelFileStream); // 关闭文件流


    *size = _size;
	return (unsigned char*)buffer;
}

void util::SDL_FreeMem(unsigned char* mem)
{
    SDL_free(mem);
}

Json::Value util::BuildJsonFromMem(const char* mem, size_t memSize)
{
    Json::CharReaderBuilder builder;

    builder["emitUTF8"] = true;

    std::unique_ptr<Json::CharReader>reader(builder.newCharReader());

    Json::Value root;
    std::string errs;
    bool ok = reader->parse(mem, mem+ memSize,&root,&errs);
    if (!ok)
    {
        SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION,"Failed to parse json : %s",errs.c_str());
        return Json::Value();
    }
    return root;
}

Json::Value util::BuildJsonFromFile(const char* filePath)
{
    size_t size;
    auto mem=SDL_LoadFileToMem(filePath,&size);
    if(!mem)
        return Json::Value();
    
    Json::Value result=BuildJsonFromMem((const char*)mem,size);
    SDL_FreeMem(mem);
    return result;
}

bool util::SaveJsonToFile(const Json::Value& json, const char* filePath)
{
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    std::unique_ptr<Json::StreamWriter> writer(
        builder.newStreamWriter());

    std::ofstream ofs;
    ofs.open(filePath);
    if (!ofs)
    {
        SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, "Failed to write file: %s", filePath);
        return false;
    }

    writer->write(json, &ofs);
    ofs.close();
    return true;
}

bool util::IsStringEndsWith(const std::string& str, const char* end)
{
    std::string ending = end;
    if (str.size() < ending.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}

bool util::IsStringStartWith(const std::string& str, const char* end)
{
    if (end == NULL)return false;
    size_t len = SDL_strlen(end)-1;
    if (str.size() < len)
    {
        return false;
    }
    return 0==SDL_memcmp(str.data(), end, len);
}

std::string util::StringToUpper(const std::string& str)
{
    std::string temStr = str;
    for (char& c : temStr) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return temStr;
}

std::string& util::ReplaceString(std::string& org, const char* strToBeReplace, const char* newstr)
{
    // TODO: 在此处插入 return 语句


        // 找到第一个匹配位置
    size_t startPos = 0;
    std::string from(strToBeReplace);
    std::string to(newstr);
    while ((startPos = org.find(from, startPos)) != std::string::npos) {
        org.replace(startPos, from.length(), to);
        startPos += to.length(); // 继续往后找，避免死循环
    }
    return org;
}

std::string util::GetFileNameFromPath(const std::string& path)
{
    size_t pos = path.find_last_of("/\\"); // 兼容两种分隔符
    std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    return filename;
}

std::string util::RemoveExtension(const std::string& path)
{
    size_t pos = path.find_last_of("."); 
    std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    return filename;
}

void util::AppendSeparator(std::string& path)
{
    if (path.empty())
    {
        path = path + "/";
        return;
    }
    if (path[path.size() - 1] == '\\' || path[path.size() - 1] == '/')
    {
        return;
    }
    path = path + "/";
}

bool util::PathEqual(const std::string& path1, const std::string& path2)
{
    std::string _pathstr1 = path1;
    std::string _pathstr2 = path2;
    ReplaceString(_pathstr1,"\\","");
    ReplaceString(_pathstr1,"/","");
    ReplaceString(_pathstr2,"\\","");
    ReplaceString(_pathstr2,"/","");

    return _pathstr1== _pathstr2;
}

