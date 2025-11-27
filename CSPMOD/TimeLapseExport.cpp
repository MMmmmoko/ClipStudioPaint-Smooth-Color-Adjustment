#include"AddressTable.h"
#include "TimeLapseExport.h"
#include"DUI/Dialog/TimeLapseExport_Dlg.h"
#include"CSPMOD.h"


void TimeLapseExport_hook::Init()
{

    void* pTimeLapseExportFunc = AddressTable::GetAddress("TimeLapseExport_Func");
    if (pTimeLapseExportFunc)
    {
        CSPMOD::Hook(pTimeLapseExportFunc, Hook_TimeLapseExport, (void**)&orig_TimeLapseExport);
        isAvailable = true;
    }



    //TimeLapseExport_Dlg::ShowModalDlg(nullptr,nullptr);


}

void TimeLapseExport_hook::Enable()
{
    if (!isEnabled && isAvailable)
    {
        isEnabled = true;
    }

}

void TimeLapseExport_hook::Disable()
{
    if (isEnabled && isAvailable)
    {
        isEnabled = false;
    }
}




int64_t TimeLapseExport_hook::Hook_TimeLapseExport(uintptr_t arg0, uintptr_t arg1)
{
    if(!isEnabled)return orig_TimeLapseExport(arg0, arg1);



    //arg0+0x98:视频时长（无符号8字节）单位纳秒
//arg0+0xa0:视频大小（4字节）单位像素


//不论结果如何，始终要禁用原始长度导出，不然自定义的参数无效

    if (CSPMOD::IsPtrValid(arg1 + 0x918))
    {
        *(uint32_t*)(arg1 + 0x918) = 1;
        *(uint32_t*)(arg0 - 0x20+0x8) = 1;
    }
    else{
        return orig_TimeLapseExport(arg0, arg1);
    }


    uint32_t* pVideoSize = (uint32_t*)(arg0 + 0xa0);
    uint64_t* pVideoDuration = (uint64_t*)(arg0 + 0x98);


    if (TimeLapseExport_hook::ShowExportDialog(pVideoSize, pVideoDuration))
    {

        return orig_TimeLapseExport(arg0, arg1);
    }
    *pVideoSize = 0;
    *pVideoDuration = 0;



    return 0;
}

bool TimeLapseExport_hook::ShowExportDialog(uint32_t* videoSize, uint64_t* videoDuration)
{
    return TimeLapseExport_Dlg::ShowModalDlg(videoSize, videoDuration);
    //SDL_Log("Size %d,Time: %llu,", *videoSize,*videoDuration/1000'000'000);
    return true;
}
