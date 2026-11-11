
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

//渲染引擎
#include "duilib/RenderSkia/RenderFactory_Skia.h"

//图片解码接口
#include "duilib/Image/ImageDecoder_ICO.h"
#include "duilib/Image/ImageDecoder_Icon.h"
#include "duilib/Image/ImageDecoder_GIF.h"
#include "duilib/Image/ImageDecoder_PNG.h"
#include "duilib/Image/ImageDecoder_PAG.h"
#include "duilib/Image/ImageDecoder_SVG.h"
#include "duilib/Image/ImageDecoder_WEBP.h"
#include "duilib/Image/ImageDecoder_JPEG.h"
#include "duilib/Image/ImageDecoder_LOTTIE.h"
#include "duilib/Image/ImageDecoder_Common.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    //ToolTip/日期时间等标准控件，需要初始化commctrl
#include <commctrl.h>
#include <Objbase.h>
#endif

#include <filesystem>


//patch方法，复制globalmanager cpp源代码，添加window参数。 修改globalmanager头文件的函数声明并添加GlobalManagerCreateControlPatched
//修复window builder的pControl = GlobalManager::Instance().CreateControl
#ifndef GlobalManagerCreateControlPatched
static_assert(false, "Do Patch!");
#endif // !1


namespace ui
{
    Control* GlobalManager::CreateControl(const DString& strControlName,Window* window)
    {
        Control* pControl = nullptr;
        for (CreateControlCallback pfnCreateControlCallback : m_pfnCreateControlCallbackList) {
            if (pfnCreateControlCallback != nullptr) {
                pControl = pfnCreateControlCallback(strControlName, window);
                if (pControl != nullptr) {
                    break;
                }
            }
        }
        return pControl;
    }
}