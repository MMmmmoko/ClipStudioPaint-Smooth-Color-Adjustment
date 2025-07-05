// pch.cpp: 与预编译标头对应的源文件
#include"../HookTool.h"



#include<Windows.h>
#include"../resource.h"
#include"CspData.h"
#include "TimeLapseExport.h"

#include"DUI/Dialog/TimeLapseExport_Dlg.h"




static int64_t(*orig_TimeLapseExport)(uintptr_t arg0, uintptr_t arg1) = NULL;
int64_t TimeLapseExport_hook::Hook_TimeLapseExport(uintptr_t arg0, uintptr_t arg1)
{
	//arg0+0x98:视频时长（无符号8字节）单位纳秒
	//arg0+0xa0:视频大小（4字节）单位像素


    //不论结果如何，始终要禁用原始长度导出，不然自定义的参数无效
    *(uint32_t*)(arg1 + 0x918) = 1;

    uint32_t*  pVideoSize =(uint32_t*) (arg0 + 0xa0);
    uint64_t* pVideoDuration =(uint64_t*) (arg0 + 0x98);

    if (TimeLapseExport_hook::ShowExportDialog(pVideoSize, pVideoDuration))
    {
        return orig_TimeLapseExport(arg0, arg1);
    }
    *pVideoSize = 0;
    *pVideoDuration = 0;


	return 0;
}


void TimeLapseExport_hook::SetUpHook()
{
    //缩时摄影导出，hook以使导出时再手动设置视频时长和尺寸：
//
//寻找思路，直接搜索videosize即可找到函数位置
    CatHook::Hook((void*)(CatHook::baseAddr + 0x141035350), (void*)TimeLapseExport_hook::Hook_TimeLapseExport, (void**)&orig_TimeLapseExport, 19);




}

uint32_t* pVideoSize;
uint64_t* pVideoDuration;
bool* runResult;
// 对话框过程函数
static LONGLONG CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
    {
        HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_VIDEOSIZE); // 获取编辑框句柄
        TCHAR szText[256];
        _i64tow_s(*pVideoSize, szText,sizeof(szText)/sizeof(TCHAR), 10);
        SetWindowText(hEdit, szText);

        hEdit = GetDlgItem(hDlg, IDC_EDIT_VIDEODURATION);
        _i64tow_s(*pVideoDuration/1000000000, szText, sizeof(szText) / sizeof(TCHAR), 10);
        SetWindowText(hEdit, szText);
    
    
    }


        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) { // 点击确定按钮
            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_VIDEOSIZE); // 获取编辑框句柄
            TCHAR szText[256];
            GetWindowText(hEdit, szText, 256); // 获取文本内容
            *pVideoSize=_wtoi(szText);

            hEdit = GetDlgItem(hDlg, IDC_EDIT_VIDEODURATION);
            GetWindowText(hEdit, szText, 256);
            *pVideoDuration = _wtof(szText) * 1000000000;//1000,000,000;

            EndDialog(hDlg, IDOK);
            *runResult = true;
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            *runResult = true;
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDC_BUTTON_CANVASSIZE) {
            //按下了填入画布尺寸的按钮

            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_VIDEOSIZE); // 获取编辑框句柄
            TCHAR szText[256];


            uint32_t width,height;
            CspData::GetCurrentProjCanvasSize(&width,&height);

            _i64tow_s(max(width,height), szText, sizeof(szText) / sizeof(TCHAR), 10);
            SetWindowText(hEdit, szText);

            return TRUE;
        }
        break;
    }
    return FALSE;
}
bool TimeLapseExport_hook::ShowExportDialog(uint32_t* videoSize, uint64_t* videoDuration)
{
    pVideoSize = videoSize;
    pVideoDuration = videoDuration;


    return TimeLapseExport_Dlg::ShowModalDlg(pVideoSize, pVideoDuration);







    bool reslut;
    runResult = &reslut;
    int result= DialogBoxParam((HINSTANCE)CatHook::thisModule,MAKEINTRESOURCE(IDD_DIALOG1),GetForegroundWindow(), DialogProc, 0);

    return (result == IDOK);
}

