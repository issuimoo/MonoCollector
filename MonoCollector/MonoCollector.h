#include <Windows.h>
#include <mfobjects.h>
#include <processthreadsapi.h>
#include <libloaderapi.h>
#include <handleapi.h>
#include <tlhelp32.h>
#include <minwindef.h>
#include <basetsd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include <format>

#define DO_API(ret_type,name,args)\
		using name##_t = ret_type(__cdecl*)args;
#include "CollectorAPI.h"
#undef DO_API

namespace Untiy3D
{
	class MonoCollector
	{
		bool il2cpp = false;
		HMODULE hModuleMono = nullptr;
	public:
		MonoCollector(std::string ModuleName);
		~MonoCollector();

		struct ClassInfo
		{
			Il2CppClass* ptr;
			std::string name;
			std::string name_space;
		};

		struct FieldsInfo
		{
			FieldInfo* ptr;
			std::string name;
			std::string name_type;
		};

		struct MethodsInfo
		{
			MethodInfo* ptr;
			std::string name;
		};

		DWORD il2cpp_EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys);
		DWORD il2cpp_EnumDomains(std::vector<Il2CppDomain*>& Domains);
		Il2CppImage* il2cpp_GetImageInAssembly(Il2CppAssembly* Assembly);
		std::string il2cpp_GetImageName(Il2CppImage* Image);
		std::string il2cpp_GetImageFile(Il2CppImage* Image);
		DWORD il2cpp_EnumClassesInImage(Il2CppImage* Image, std::vector<ClassInfo>& Classes);
		DWORD il2cpp_EnumFieldsInClass(Il2CppClass* klass, std::vector<FieldsInfo>& Fields);
		DWORD il2cpp_EnumMethodsInClass(Il2CppClass* klass, std::vector<MethodsInfo>& Methods);
		FieldInfo* il2cpp_GetClassFieldFromName(Il2CppClass* klass, std::string name);
		DWORD_PTR il2cpp_GetFieldStatic(FieldInfo* field);
		Il2CppClass* il2cpp_GetImageClassFromName(Il2CppImage* image, std::string name, std::string namespaze = "");
		std::string il2cpp_GetImageClassName(Il2CppClass* klass);
		std::string il2cpp_GetClassFieldName(FieldInfo* field);

	private:
		void GetMonoFunc();
		void Getil2cppFunc();
		#define DO_API(ret_type,name,args)\
		name##_t name = nullptr;
		#include "CollectorAPI.h"
		#undef DO_API
	};
}