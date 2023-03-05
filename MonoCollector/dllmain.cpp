#include "il2cppCollector.h"

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
	Untiy3D::il2cppCollector* il2cpp = new Untiy3D::il2cppCollector("GameAssembly.dll");
	StartConsole("",true);

	std::vector<Il2CppAssembly*> Assemblys;
	std::cout << "size:" << il2cpp->il2cpp_EnummAssembly(Assemblys) << std::endl;
	for (size_t i = 0,max = Assemblys.size(); i < max; i++)
	{
		Il2CppImage* image = il2cpp->il2cpp_GetImage(Assemblys[i]);
		std::string imagename = il2cpp->il2cpp_GetImageName(image);
		if (imagename == "Assembly-CSharp.dll")
		{
			std::vector<Il2CppClass*> Class;
			il2cpp->il2cpp_EnumClasses(image, Class);
			for (size_t iw = 0, max = Class.size(); iw < max; iw++)
			{
				std::vector<MethodInfo*> Methods;
				il2cpp->il2cpp_EnumMethods(Class[iw], Methods);
				for (size_t iq = 0, max = Methods.size(); iq < max; iq++)
				{
					std::cout << std::format("[{:#08x}][{}][{}] {} {}", il2cpp->il2cpp_GetMethodMemAddress(Methods[iq]), imagename, il2cpp->il2cpp_GetClassName(Class[iw]), il2cpp->il2cpp_GetTypeName(il2cpp->il2cpp_GetMethodRetType(Methods[iq])), il2cpp->il2cpp_GetMethodName(Methods[iq]));
					std::cout << "(";
					DWORD count = il2cpp->il2cpp_GetMethodParamCount(Methods[iq]);
					for (size_t i = 0; i < count; i++)
					{
						std::cout << il2cpp->il2cpp_GetTypeName(il2cpp->il2cpp_GetMethodParam(Methods[iq], i)) << " " << il2cpp->il2cpp_GetMethodParamName(Methods[iq], i) << ",";
					}
					if (count)
					{
						std::cout << "\b)\n";
					}
					else
					{
						std::cout << ")\n";
					}
				}
			}
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

