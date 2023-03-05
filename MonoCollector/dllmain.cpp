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
	std::cout << "size:" << Mono->il2cpp_EnummAssembly(Assemblys) << std::endl;
	for (size_t i = 0,max = Assemblys.size(); i < max; i++)
	{
		Il2CppImage* image = Mono->il2cpp_GetImage(Assemblys[i]);
		std::string imagename = Mono->il2cpp_GetImageName(image);
		if (imagename == "Assembly-CSharp.dll")
		{
			std::vector<Il2CppClass*> Class;
			Mono->il2cpp_EnumClasses(image, Class);
			for (size_t iw = 0, max = Class.size(); iw < max; iw++)
			{
				std::vector<FieldInfo*> Fields;
				Mono->il2cpp_EnumFields(Class[i], Fields);
				for (size_t iq = 0, max = Fields.size(); iq < max; iq++)
				{
					std::cout << std::format("[{}][{}] {}({})", imagename, Mono->il2cpp_GetClassName(Class[iw]), Mono->il2cpp_GetFieldName(Fields[iq]), Mono->il2cpp_GetTypeName(Mono->il2cpp_GetFieldType(Fields[iq]))) << std::endl;
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

