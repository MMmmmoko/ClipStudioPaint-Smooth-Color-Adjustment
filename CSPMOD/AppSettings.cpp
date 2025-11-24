
//#include"AppContext.h"
#include"AppSettings.h"

#include"Util/Util.h"




#include"TA_HSV.h"
#include"TA_ColorBalance.h"
#include"TA_ToneCurve.h"
#include"TimeLapseExport.h"
#include"ScriptAction_ApplyEffects.h"

AppSettings AppSettings::ins;
SDL_Color AppSettings::_defaultBackgroundColor = { 255,255,255,0 };//低到高 rgba


//不要修改文件名？
#define _SETTINGS_FILE_NAME "CSPMOD_Settings.json"





AppSettings::AppSettings()
{
	auto _prefpath = SDL_GetPrefPath("CatSoft", "CSPMOD");
	prefpath = _prefpath;
	SDL_free(_prefpath);
}

AppSettings::~AppSettings()
{
}

void AppSettings::Load()
{
	//重置默认
	*this = AppSettings();

	std::string settingsFilePath = prefpath;
	settingsFilePath += _SETTINGS_FILE_NAME;

	Json::Value configJson = util::BuildJsonFromFile(settingsFilePath.c_str());







	//先检查项是否存在 如下:
	//后续如果有非基础的数据类型（如rect）
	//则对所有类型封装一层读写：
	//IsRectType(Json::Value& json){json.isArray()......}
	//SaveRect(Json::Value& json,Rect rect){json[i]=recr.topleft....};

	//子代码如下
	/*
	if (configJson.isMember("Window")&& configJson["Window"].isMember("Top")&& configJson["Window"]["Top"].isBool())
	{
		SetWindowTop(configJson["Window"]["Top"].asBool());
	}
	else
	{
		SetWindowTop(_WindowTop);
	}
	*/

#define APPSETTINGS_Load(settingGroup,setting,type,defaultValue) \
if(configJson.isMember(#settingGroup)&& configJson[#settingGroup].isMember(#setting)&&\
 _Is_##type##_Type(configJson[#settingGroup][#setting])) \
	{ \
	Set##settingGroup##setting(_Get_##type(configJson[#settingGroup][#setting])); \
} \
else \
{\
 Set##settingGroup##setting(_##settingGroup##setting);\
}

	{
		if (configJson.isMember("FunctionEnable") && configJson["FunctionEnable"].isMember("HSV") && _Is_bool_Type(configJson["FunctionEnable"]["HSV"])) {
			SetFunctionEnableHSV(_Get_bool(configJson["FunctionEnable"]["HSV"]));
		}
		else {
			SetFunctionEnableHSV(_FunctionEnableHSV);
		} if (configJson.isMember("FunctionEnable") && configJson["FunctionEnable"].isMember("ColorBalance") && _Is_bool_Type(configJson["FunctionEnable"]["ColorBalance"])) {
			SetFunctionEnableColorBalance(_Get_bool(configJson["FunctionEnable"]["ColorBalance"]));
		}
		else {
			SetFunctionEnableColorBalance(_FunctionEnableColorBalance);
		} if (configJson.isMember("FunctionEnable") && configJson["FunctionEnable"].isMember("ToneCurve") && _Is_bool_Type(configJson["FunctionEnable"]["ToneCurve"])) {
			SetFunctionEnableToneCurve(_Get_bool(configJson["FunctionEnable"]["ToneCurve"]));
		}
		else {
			SetFunctionEnableToneCurve(_FunctionEnableToneCurve);
		} if (configJson.isMember("FunctionEnable") && configJson["FunctionEnable"].isMember("TimeLapseExport") && _Is_bool_Type(configJson["FunctionEnable"]["TimeLapseExport"])) {
			SetFunctionEnableTimeLapseExport(_Get_bool(configJson["FunctionEnable"]["TimeLapseExport"]));
		}
		else {
			SetFunctionEnableTimeLapseExport(_FunctionEnableTimeLapseExport);
		} if (configJson.isMember("FunctionEnable") && configJson["FunctionEnable"].isMember("Script_ApplyEffects") && _Is_bool_Type(configJson["FunctionEnable"]["Script_ApplyEffects"])) {
			SetFunctionEnableScript_ApplyEffects(_Get_bool(configJson["FunctionEnable"]["Script_ApplyEffects"]));
		}
		else {
			SetFunctionEnableScript_ApplyEffects(_FunctionEnableScript_ApplyEffects);
		}
	}

#undef APPSETTINGS_Load



	//根据参数应用
	_FunctionEnableHSV?TA_HSV::Enable(): TA_HSV::Disable();
	_FunctionEnableColorBalance ? TA_ColorBalance::Enable(): TA_ColorBalance::Disable();
	_FunctionEnableToneCurve ? TA_ToneCurve::Enable(): TA_ToneCurve::Disable();
	_FunctionEnableTimeLapseExport ? TimeLapseExport_hook::Enable(): TimeLapseExport_hook::Disable();
	_FunctionEnableScript_ApplyEffects ? ScriptAction_ApplyEffects::Enable() : ScriptAction_ApplyEffects::Disable();








}


bool AppSettings::Save()
{

	//构建Json
	Json::Value root;

	//_Save_bool_Type(root["Window"]["Top"], _WindowTop);
#define APPSETTINGS_Save(settingGroup,setting,type,defaultValue) \
	_Save_##type##_Type(root[#settingGroup][#setting],_##settingGroup##setting);
	{ CATTUBER_APPSETTINGS_LIST(APPSETTINGS_Save) }
#undef APPSETTINGS_Save


	////特殊处理，如果不保存Lock状态的情况下，保存为false
	//if (!root["Window"]["LockSave"].asBool())
	//{
	//	root["Window"]["Lock"] = false;
	//}

	//将json写入文件
	//需要判断是否可写
	std::string settingsFilePath = prefpath;
	settingsFilePath += _SETTINGS_FILE_NAME;

	if (!util::SaveJsonToFile(root, settingsFilePath.c_str()))
	{
		SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, "Save Settings Failed!");
		return false;
	}
	SDL_LogInfo(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, "Settings Saved.");
	return true;



}

const std::vector<std::string>& AppSettings::GetLocalLanguageFullbackVec()
{
	static std::vector<std::string> vec;
	if (!vec.empty())
		return vec;


	//构建fullback表
	int count;
	SDL_Locale** localeList = SDL_GetPreferredLocales(&count);
	if (NULL == localeList)
	{
		SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, "Can not get locale info! %s", SDL_GetError());
		//TODO/FIXME加入对steam语言的检测
		vec.push_back("english");
		return vec;
	}

#define PUSHLANG(_language,_lang) else if (0 == SDL_strcmp(localeList[i]->language, #_lang))vec.push_back(#_language)
	for (int i = 0; i < count; i++)
	{
		if (0 == SDL_strcmp(localeList[i]->language, "bg"))
		{
			vec.push_back("bulgarian");
		}
		else if (0 == SDL_strcmp(localeList[i]->language, "zh"))
		{
			if (0 == SDL_strcmp(localeList[i]->country, "CN")
				|| 0 == SDL_strcmp(localeList[i]->country, "SG")
				|| NULL == localeList[i]->country)
				vec.push_back("schinese");
			else
				vec.push_back("tchinese");
		}
		else if (0 == SDL_strcmp(localeList[i]->language, "cs"))
		{
			vec.push_back("czech");
		}
		PUSHLANG(czech, cs);
		PUSHLANG(danish, da);
		PUSHLANG(dutch, nl);
		PUSHLANG(english, en);
		PUSHLANG(finnish, fi);
		PUSHLANG(french, fr);
		PUSHLANG(german, de);
		PUSHLANG(greek, el);
		PUSHLANG(hungarian, hu);
		PUSHLANG(indonesian, id);
		PUSHLANG(italian, it);
		PUSHLANG(japanese, ja);
		PUSHLANG(koreana, ko);
		PUSHLANG(norwegian, no);
		PUSHLANG(polish, pl);
		else if (0 == SDL_strcmp(localeList[i]->language, "pt"))
		{
			if (0 == SDL_strcmp(localeList[i]->country, "BR"))
				vec.push_back("brazilian");
			else
				vec.push_back("portuguese");
		}
		PUSHLANG(romanian, ro);
		PUSHLANG(russian, ru);
		else if (0 == SDL_strcmp(localeList[i]->language, "es"))
		{
			if (0 == SDL_strcmp(localeList[i]->country, "ES"))
				vec.push_back("spanish");
			else
				vec.push_back("latam");
		}
		PUSHLANG(swedish, sv);
		PUSHLANG(thai, th);
		PUSHLANG(turkish, tr);
		PUSHLANG(ukrainian, uk);
		PUSHLANG(vietnamese, vi);
	}

	//对简体繁体互相添加一个Fullback
	for (auto it = vec.begin(); it != vec.end(); it++)
	{
		if (*it == "schinese")
		{
			vec.insert(it + 1, "tchinese");
			break;
		}
		if (*it == "tchinese")
		{
			vec.insert(it + 1, "schinese");
			break;
		}
	}
	//总以english结尾
	vec.push_back("english");

	SDL_free(localeList);




	return vec;
}













//************************************************
//实现_OnXXXXXChange()
//注意设置项的修改应该是发生在主线程中的
//与渲染线程有关的需要进行转发
//************************************************

void AppSettings::_OnFunctionEnableHSVChange(const bool& value)
{
	if (TA_HSV::Available())
	{
		value ? TA_HSV::Enable() : TA_HSV::Disable();
		Save();
	}
}

void AppSettings::_OnFunctionEnableColorBalanceChange(const bool& value)
{
	if (TA_ColorBalance::Available())
	{
		value ? TA_ColorBalance::Enable() : TA_ColorBalance::Disable();
		Save();
	}
}

void AppSettings::_OnFunctionEnableToneCurveChange(const bool& value)
{
	if (TA_ToneCurve::Available())
	{
		value ? TA_ToneCurve::Enable() : TA_ToneCurve::Disable();
		Save();
	}
}

void AppSettings::_OnFunctionEnableTimeLapseExportChange(const bool& value)
{
	if (TimeLapseExport_hook::Available())
	{
		value ? TimeLapseExport_hook::Enable() : TimeLapseExport_hook::Disable();
		Save();
	}
}

void AppSettings::_OnFunctionEnableScript_ApplyEffectsChange(const bool& value)
{
	if (ScriptAction_ApplyEffects::Available())
	{
		value ? ScriptAction_ApplyEffects::Enable() : ScriptAction_ApplyEffects::Disable();
		Save();
	}
}































































bool AppSettings::_Is_double_Type(Json::Value& json)
{
	return json.isDouble();
}
void AppSettings::_Save_double_Type(Json::Value& json, double value)
{
	json = value;
}
double AppSettings::_Get_double(Json::Value& json)
{
	return json.asDouble();
}



bool AppSettings::_Is_int64_t_Type(Json::Value& json)
{
	return json.isInt64();
}
void AppSettings::_Save_int64_t_Type(Json::Value& json, int64_t  value)
{
	json = value;
}
int64_t AppSettings::_Get_int64_t(Json::Value& json)
{
	return json.asInt64();
}


bool AppSettings::_Is_bool_Type(Json::Value& json)
{
	return json.isBool();
}
void AppSettings::_Save_bool_Type(Json::Value& json, bool value)
{
	json = value;
}
bool AppSettings::_Get_bool(Json::Value& json)
{
	return json.asBool();
}

bool AppSettings::_Is__CatString_Type(Json::Value& json)
{
	return json.isString();
}
void AppSettings::_Save__CatString_Type(Json::Value& json, const std::string& value)
{
	json = value;
}
std::string AppSettings::_Get__CatString(Json::Value& json)
{
	return json.asString();
}



bool AppSettings::_Is_SDL_Color_Type(Json::Value& json)
{
	//颜色由4个整数数值组成数组//不面向用户所以不对3元的颜色进行判断
	//json[0]
	if (json.isArray() && json.size() == 4)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!(json[i].isUInt64() && json[i].asUInt64() <= 255))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}
void AppSettings::_Save_SDL_Color_Type(Json::Value& json, const SDL_Color& value)
{
	json[0] = value.r;
	json[1] = value.g;
	json[2] = value.b;
	json[3] = value.a;
}
SDL_Color AppSettings::_Get_SDL_Color(Json::Value& json)
{
	SDL_Color result;
	result.r = static_cast<uint8_t>(json[0].asUInt64());
	result.g = static_cast<uint8_t>(json[1].asUInt64());
	result.b = static_cast<uint8_t>(json[2].asUInt64());
	result.a = static_cast<uint8_t>(json[3].asUInt64());
	return result;
}


