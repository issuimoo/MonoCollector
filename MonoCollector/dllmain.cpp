﻿#include "MonoCollector.h"
#include <iostream>

HWND StartConsole(const char* title, bool close) {
	HANDLE g_hOutput = 0;
	HWND hwnd = NULL;
	HMENU hmenu = NULL;
	AllocConsole();
	g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle(title);
	while (NULL == hwnd) hwnd = ::FindWindow(NULL, (LPCTSTR)title);
	hmenu = ::GetSystemMenu(hwnd, FALSE);
	if (!close) { DeleteMenu(hmenu, SC_CLOSE, MF_BYCOMMAND); }
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_THICKFRAME);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
	return hwnd;
}

void ShowMsg() {
	MessageBox(NULL, "DumpTools By 遂沫 (a1992724048@outlook.com)", "DumpTools By 遂沫", NULL);
}

void Th()
{
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ShowMsg, NULL, NULL, NULL);
	MessageBox(NULL, "进入游戏后再dump", "DumpTools By 遂沫", NULL);
	system("pause");

	StartConsole("DumpTools By 遂沫 (a1992724048@outlook.com)",true);

	std::cout << "DumpTools By 遂沫 (a1992724048@outlook.com) \a" << std::endl;
	std::cout << "DumpTools By 遂沫 (a1992724048@outlook.com) \a" << std::endl;
	std::cout << "DumpTools By 遂沫 (a1992724048@outlook.com) \a" << std::endl;
	std::cout << "start dump ..." << std::endl;

	Untiy3D::MonoCollector* Mono;

	if (GetModuleHandle("mono.dll"))
	{
		Mono = new Untiy3D::MonoCollector("mono.dll");
		Mono->Mono_Dump2File(".\\");
	}
	else if (GetModuleHandle("mono-2.0-bdwgc.dll"))
	{
		Mono = new Untiy3D::MonoCollector("mono-2.0-bdwgc.dll");
		Mono->Mono_Dump2File(".\\");
	}
	else if (GetModuleHandle("GameAssembly.dll"))
	{
		Untiy3D::MonoCollector* Mono = new Untiy3D::MonoCollector("GameAssembly.dll");
		Mono->il2cpp_Dump2File(".\\");
	}
	else
	{
		MessageBox(NULL, "未找到相关dll", "DumpTools By 遂沫", NULL);
		return;
	}

	std::cout << "输出dump文件在游戏目录下 \a" << std::endl;

	std::cout << "dump done!" << std::endl;
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Th, NULL, NULL, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

