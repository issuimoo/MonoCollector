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

#include "il2cpp-api-types.h"

namespace Untiy3D
{
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

	class MonoCollector
	{
		bool il2cpp = false;
	public:
		MonoCollector(std::string ModuleName);
		~MonoCollector();

		DWORD EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys);
		DWORD EnumDomains(std::vector<Il2CppDomain*>& Domains);
		Il2CppImage* GetImageInAssembly(Il2CppAssembly* Assembly);
		std::string GetImageName(Il2CppImage* Image);
		std::string GetImageFile(Il2CppImage* Image);
		DWORD EnumClassesInImage(Il2CppImage* Image, std::vector<ClassInfo>& Classes);
		DWORD EnumFieldsInClass(Il2CppClass* klass, std::vector<FieldsInfo>& Fields);
		DWORD EnumMethodsInClass(Il2CppClass* klass, std::vector<MethodsInfo>& Methods);
		
	private:
		void GetMonoFunc();
		void Getil2cppFunc();
	};

	static HMODULE hModuleMono = nullptr;
}