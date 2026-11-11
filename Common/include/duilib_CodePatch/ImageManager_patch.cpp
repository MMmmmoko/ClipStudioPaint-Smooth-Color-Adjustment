#include "duilib/Core/ImageManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/FilePathUtil.h"

//
//
//DString ImageLoadParam::GetLoadKey(uint32_t nLoadDpiScale) const
//{
//    //格式为(中括号内容为可选)：<图片路径>[@nLoadDpiScale]#IsImageDpiScaleEnabled()[$srcWidth:srcHeight]
//    ASSERT(!m_srcImageLoadPath.m_imageFullPath.IsEmpty());
//    DString fullPath = m_srcImageLoadPath.m_imageFullPath.ToString();
//    if ((nLoadDpiScale != 0) && (nLoadDpiScale != 100)) {
//        //追加缩放百分比
//        fullPath += _T("@");
//        fullPath += StringUtil::UInt32ToString(nLoadDpiScale);
//    }
//    fullPath += IsImageDpiScaleEnabled() ? _T("#1") : _T("#0");
//    if (!m_srcWidth.empty() || !m_srcHeight.empty()) {
//        fullPath += _T("$");
//        fullPath += m_srcWidth.c_str();
//        fullPath += _T(":");
//        fullPath += m_srcHeight.c_str();
//    }
//    return fullPath;
//}



static inline bool CmpPath(const DString& path, const DString& pathKey)
{
    if (pathKey[0] != L'C')return false;
    if (path.size() > pathKey.size())return false;
    for (int i = 0; i < path.size(); i++)
    {
        if (path[i] == pathKey[i])continue;
        if ((path[i] == L'\\'|| path[i]==L'/')&& (pathKey[i] == L'\\' || pathKey[i] == L'/'))
            continue;
        return false;
    }
    return true;


}

void ui::ImageManager::ReleaseImageIM(const DString& imgPath, int sizeW , int sizeH )
{
    //sizeW和sizeH默认传入0
    //如果赋予了长宽信息，那么删除缓存将更快

    //在 loadParam.GetLoadKey()中获取缓存字符串键值格式
    if (sizeW)
    {
        //手动构建
        ASSERT(false);

    }
    else
    {

        //for (auto it = m_imageInfoMap.begin(); it != m_imageInfoMap.end(); ) {
        //    //以路径的形式进行比对
        //    
        //    if (CmpPath(imgPath, it->first)) {
        //        it = m_imageInfoMap.erase(it); // erase 返回下一个有效迭代器
        //    }
        //    else {
        //        ++it;
        //    }
        //}

        auto it = m_imageDataMap.find(ui::FilePathUtil::NormalizeFilePath(imgPath));
        if(it!= m_imageDataMap.end())
        {
            m_imageDataMap.erase(it);
        }


    }


}