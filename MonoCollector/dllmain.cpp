#include "MonoCollector.h"

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

void Th()
{
	system("pause");
	Untiy3D::MonoCollector* Mono = new Untiy3D::MonoCollector("GameAssembly.dll");

	StartConsole("",true);

	std::vector<Il2CppAssembly*> Assemblys;
	std::cout << "size:" << Mono->EnummAssembly(Assemblys) << std::endl;
	for (size_t i = 0,max = Assemblys.size(); i < max; i++)
	{
		Il2CppImage* image = Mono->GetImageInAssembly(Assemblys[i]);
		std::cout << "[" << i << "] " << Mono->GetImageName(image) << std::endl;

		std::vector<Untiy3D::ClassInfo> Class;
		Mono->EnumClassesInImage(image, Class);
		for (size_t i = 0, max = Class.size(); i < max; i++)
		{
			std::cout << "[" << Class[i].name_space << "] " << Class[i].name << std::endl;
		}
	}
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

