#include "MonoCollector.hpp"

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
	MonoCollector* Mono = new MonoCollector();

	std::vector<UINT64> ret;
	printf("size: %d \n\n\n", Mono->EnumAssemblies(ret));
	for (size_t i = 0,max = ret.size(); i < max; i++)
	{
		std::string s;
		Mono->GetImageName((void*)Mono->GetImageFromAssembly((void*)ret[i]), s);
		std::cout << s << "   " << ret[i] << std::endl;
		std::vector<UINT_PTR> ret_ptr; std::vector<std::string> ret_string_class; std::vector<std::string> ret_string_namespace;
		Mono->EnumClassesInImage((void*)Mono->GetImageFromAssembly((void*)ret[i]), ret_ptr, ret_string_class, ret_string_namespace);
		
		std::cout << ret_string_namespace.size() << "   " << ret_string_class.size() << "   " << ret_string_namespace.size() << std::endl;
		
		for (size_t i2 = 0, max2 = ret_ptr.size(); i2 < max2; i2++)
		{
			std::cout << "---" << ret_string_namespace[i2] << "->" << ret_string_class[i2] << "   " << ret_ptr[i2] << std::endl;
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		StartConsole("123", false);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Th, NULL, NULL, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

