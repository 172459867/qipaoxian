// qipaoxian.cpp : main source file for qipaoxian.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

CAppModule _Module;

int __stdcall LuaErrorHandle(lua_State* luaState,const wchar_t* pExtInfo,const wchar_t* luaErrorString, PXL_LRT_ERROR_STACK pStackInfo)
{
	static long s_lEnter = 0;
	long prev = ::InterlockedCompareExchange(&s_lEnter, 1, 0);
	if (prev == 0)
	{
		int ret = 0;
		if(pExtInfo != NULL)
		{
			assert(luaErrorString);

			std::wstring str = luaErrorString;
			str += L" @ ";
			str += pExtInfo;
			str += L"\r\n\r\n点击确定查看调用堆栈。";
			ret = ::MessageBox(0, str.c_str(), L"脚本错误", MB_ICONERROR | MB_OKCANCEL);
		}
		else
		{
			std::wstring str = luaErrorString;
			str += L"\r\n\r\n点击确定查看调用堆栈。";
			ret = ::MessageBox(0, str.c_str(), L"脚本错误", MB_ICONERROR | MB_OKCANCEL);
		}
		if (ret == IDOK)
		{
			std::string callstack;
			if (pStackInfo->logs != NULL)
			{
				const char* stack = NULL;
				while(XLLRT_RESULT_SUCCESS == XLLRT_DebugLogsPopNextLog(&stack, pStackInfo->logs) && stack != NULL)
				{
					callstack.append(stack);
				}
			}
			MessageBoxA(0, callstack.c_str(), "调用堆栈", MB_ICONERROR | MB_OK);
		}

		::InterlockedExchange(&s_lEnter, 0);
	}

	return 0;
}

bool InitXLUE()
{
	XLFS_Init();

	XLGraphicParam param;
	XL_PrepareGraphicParam(&param);
	param.textType = XLTEXT_TYPE_FREETYPE;
	XL_InitGraphicLib(&param);

	XL_SetFreeTypeEnabled(TRUE);

	XLUE_InitLoader(NULL);

	// 设置脚本错误回调
	XLLRT_ErrorHandle(&LuaErrorHandle);

	return true;
}

bool UninitXLUE()
{
	XLUE_Uninit(NULL);
	XLUE_UninitLuaHost(NULL);
	XL_UnInitGraphicLib();
	XLUE_UninitHandleMap(NULL);
	XLFS_Uninit();

	return true;
}

const wchar_t* GetXARPath()
{
	static wchar_t path[MAX_PATH] = {0};
	GetModuleFileName(NULL, path, MAX_PATH);
	PathAppend(path, L"..\\..\\samples\\BoltFox\\xar\\");
	return path;
}

bool LoadXAR()
{
	XLUE_AddXARSearchPath(GetXARPath());

	if(XLUE_LoadXAR("BoltFox") != 0)
	{
		::MessageBox(0, L"Load XAR failed!", 0 , 0);
		return false;
	}

	return true;
}

bool InitApp()
{
	if (!InitXLUE())
	{
		return false;
	}

	/*if (!InitLuaCore())
	{
		return false;
	}

	InitLuaHelper();*/

	if (!LoadXAR())
	{
		return false;
	}

	return true;
}

bool UninitApp()
{
	UninitXLUE();

	return true;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	if (!InitApp())
	{
		return -1;
	}

	int nRet = theLoop.Run();

	UninitApp();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE , LPTSTR lpstrCmdLine, int nCmdShow)
{
	OleInitialize(NULL);

	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	

	HRESULT hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
