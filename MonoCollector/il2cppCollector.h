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
#include "il2cppCollectorAPI.h"
#undef DO_API

namespace Untiy3D
{
	class il2cppCollector
	{
	protected:
		bool il2cpp = false;
		HMODULE hModuleMono = nullptr;
	public:
		il2cppCollector(std::string ModuleName);
		~il2cppCollector();

		//Domains (域)
		DWORD il2cpp_EnumDomains(std::vector<Il2CppDomain*>& Domains);

		//Assembly (程序集)
		DWORD il2cpp_EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys);

		//Type (类型)
		std::string il2cpp_GetTypeName(Il2CppType* type);
		
		//Image (镜像)
		Il2CppImage* il2cpp_GetImage	(Il2CppAssembly* Assembly);
		std::string	 il2cpp_GetImageName(Il2CppImage* Image);
		std::string  il2cpp_GetImageFile(Il2CppImage* Image);

		//Class (类)
		DWORD		il2cpp_EnumClasses			(Il2CppImage* Image, std::vector<Il2CppClass*>& Classes);
		Il2CppClass*il2cpp_GetClassFromName		(Il2CppImage* image, std::string name, std::string namespaze = "");
		std::string il2cpp_GetClassName			(Il2CppClass* klass);
		std::string il2cpp_GetClassNamespace	(Il2CppClass* klass);
		DWORD		il2cpp_GetClassCount		(Il2CppImage* Image);

		//Field (变量)
		DWORD		il2cpp_EnumFields			(Il2CppClass* klass, std::vector<FieldInfo*>& Fields);
		FieldInfo*  il2cpp_GetFieldFromName		(Il2CppClass* klass, std::string name);
		DWORD_PTR   il2cpp_GetFieldStatic		(FieldInfo* field);
		std::string il2cpp_GetFieldName			(FieldInfo* field);
		Il2CppType* il2cpp_GetFieldType			(FieldInfo* field);

		//Method (函数)
		DWORD		il2cpp_EnumMethods			(Il2CppClass* klass, std::vector<MethodInfo*>& Methods);
		MethodInfo* il2cpp_GetMethodFromName	(Il2CppClass* klass, std::string name, DWORD argscount = -1);
		std::string il2cpp_GetMethodName		(MethodInfo* Method);
		Il2CppType* il2cpp_GetMethodRetType		(MethodInfo* Method);
		DWORD		il2cpp_GetMethodParamCount	(MethodInfo* Method);
		Il2CppType* il2cpp_GetMethodParam		(MethodInfo* Method, DWORD index);
		std::string il2cpp_GetMethodParamName	(MethodInfo* Method, DWORD index);
		bool		il2cpp_GetMethodInstance	(MethodInfo* method);
		DWORD_PTR	il2cpp_GetMethodMemAddress	(MethodInfo* method);

	private:
		void Getil2cppFunc();
	public:
		#define DO_API(ret_type,name,args)\
		name##_t name = nullptr;
		#include "il2cppCollectorAPI.h"
		#undef DO_API
	};
}