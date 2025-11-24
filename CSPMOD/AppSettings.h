#ifndef _AppSettings_H
#define _AppSettings_H

#include<SDL3/SDL.h>
#include<iostream>
#include"json/json.h"

//名称、变量类型、
//这里设置平台无关的项目
//
// LockSave指示是否保存锁定状态，如果不保存，则每次启动软件时为未锁定状态
// 
//坐标范围Mouse_InputArea是基于显示器
//Other设置项之后可能需要另外起一个宏或者使用平台前缀

//为了使用宏，需要同typedef把std::string的命名空间去掉//主要是去掉这个符号::
typedef std::string _CatString;
#define CATTUBER_APPSETTINGS_LIST(F) \
    F(FunctionEnable,HSV,bool,true)        \
    F(FunctionEnable,ColorBalance,bool,true)         \
    F(FunctionEnable,ToneCurve,bool,true) \
    F(FunctionEnable,TimeLapseExport,bool,true) \
    F(FunctionEnable,Script_ApplyEffects,bool,true) 

//MouseInputArea使用字符串，需要处理显示器名相同的情况
//有非默认加载写入行为的
//#define APPSETTINGS_LIST_NONDEFAULT(F) \
//    F(Mouse,RelativeMove,bool,true) \



//需要对SDL_Color函数进行重载
inline bool operator!=(SDL_Color const& lhs, SDL_Color const& rhs)
{
    return SDL_memcmp(&lhs, &rhs, sizeof(SDL_Color)) != 0;
}

class AppSettings
{
public:
    static AppSettings& GetIns() { return ins; };

    AppSettings();
    ~AppSettings();
    void Load();
    bool Save();


    static const std::vector<std::string>& GetLocalLanguageFullbackVec();



#define APPSETTINGS_Declarations(settingGroup,setting,type,defaultValue) \
public: \
static type Get##settingGroup##setting(){return ins._##settingGroup##setting;} \
static void Set##settingGroup##setting(const type##& value){ \
    if(ins._##settingGroup##setting!=value){ins._##settingGroup##setting=value;ins._On##settingGroup##setting##Change(value);}} \
private: \
type _##settingGroup##setting=defaultValue; \
void _On##settingGroup##setting##Change(const type##& value);

    CATTUBER_APPSETTINGS_LIST(APPSETTINGS_Declarations)
#undef APPSETTINGS_Declarations




private:
    //为了使用宏进行一层封装//没找到bool到Bool的映射方法，且后续有拓展类型的可能，所以用函数封装一下
    static bool _Is_double_Type(Json::Value& json);
    static void _Save_double_Type(Json::Value& json, double value);
    static double _Get_double(Json::Value& json);

    static bool _Is_int64_t_Type(Json::Value& json);
    static void _Save_int64_t_Type(Json::Value& json, int64_t value);
    static int64_t _Get_int64_t(Json::Value& json);

    static bool _Is_bool_Type(Json::Value& json);
    static void _Save_bool_Type(Json::Value& json, bool value);
    static bool _Get_bool(Json::Value& json);

    static bool _Is__CatString_Type(Json::Value& json);
    static void _Save__CatString_Type(Json::Value& json, const std::string& value);
    static std::string _Get__CatString(Json::Value& json);

    static bool _Is_SDL_Color_Type(Json::Value& json);
    static void _Save_SDL_Color_Type(Json::Value& json, const SDL_Color& value);
    static SDL_Color _Get_SDL_Color(Json::Value& json);



    static AppSettings ins;
    static SDL_Color _defaultBackgroundColor;


    std::string  prefpath ;
    //bool loaded = false;//这个变量用来帮助进行一些特殊处理，比如判断是否锁定
};



















#endif