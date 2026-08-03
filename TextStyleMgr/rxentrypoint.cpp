// rxentrypoint.cpp : ZRX插件入口点
//

#include "stdafx.h"
#include "tchar.h"
#include <aced.h>
#include <rxregsvc.h>
#include <adui.h>
#include <acui.h>
#include "AcExtensionModule.h"
#include "TextStyleMgrDlg.h"

extern HWND adsw_acadMainWnd();

AC_IMPLEMENT_EXTENSION_MODULE(theArxDLL);

// 显示主对话框
void showTextStyleMgrDlg()
{
    CAcModuleResourceOverride resOverride;

    CTextStyleMgrDlg dlg(CWnd::FromHandle(adsw_acadMainWnd()));
    dlg.DoModal();
}

// 初始化：注册命令
static void initApp()
{
    CAcModuleResourceOverride resOverride;

    acedRegCmds->addCommand(
        _T("TEXTSTYLE_MGR_COMMANDS"),
        _T("TextStyleMgr"),
        _T("TSM"),
        ACRX_CMD_MODAL,
        showTextStyleMgrDlg,
        NULL,
        -1,
        theArxDLL.ModuleResourceInstance());
}

// 卸载：移除命令组
static void unloadApp()
{
    acedRegCmds->removeGroup(_T("TEXTSTYLE_MGR_COMMANDS"));
}

// DLL主函数
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        theArxDLL.AttachInstance(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        theArxDLL.DetachInstance();
    }
    return 1;
}

// ZRX入口函数
extern "C" AcRx::AppRetCode
zcrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{
    switch (msg)
    {
    case AcRx::kInitAppMsg:
        acrxDynamicLinker->unlockApplication(appId);
        acrxDynamicLinker->registerAppMDIAware(appId);
        initApp();
        acutPrintf(_T("\n文字样式与字体管理工具V4.0已加载。输入 TSM 命令启动。"));
        break;
    case AcRx::kUnloadAppMsg:
        unloadApp();
        break;
    case AcRx::kInitDialogMsg:
        break;
    default:
        break;
    }
    return AcRx::kRetOK;
}

// ZRX API版本导出（由SDK库提供实现）
#ifdef _WIN64
#pragma comment(linker, "/export:zcrxEntryPoint,PRIVATE")
#pragma comment(linker, "/export:zcrxGetApiVersion,PRIVATE")
#else
#pragma comment(linker, "/export:_zcrxEntryPoint,PRIVATE")
#pragma comment(linker, "/export:_zcrxGetApiVersion,PRIVATE")
#endif
