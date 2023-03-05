#include "il2cppCollector.h"

WORD UTF8TOUTF16(char* szUtf8) 
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::u16string dest = convert.from_bytes(szUtf8);
	return *(WORD*)&dest[0];
}

namespace UntiyMono
{
	void _cdecl vectorpushback(void* ptr, std::vector<DWORD_PTR>& v)
	{
		v.push_back((UINT_PTR)ptr);
	}
}

namespace Untiy3D
{
	il2cppCollector::~il2cppCollector()
	{

	}

	il2cppCollector::il2cppCollector(std::string ModuleName)
	{
		hModuleMono = GetModuleHandleA(ModuleName.c_str());

		if (!hModuleMono)
		{
			MessageBox(NULL, "Can not find Module", "error", NULL);
			return;
		}

		if (!GetProcAddress(hModuleMono, "il2cpp_thread_attach"))
		{
			MessageBox(NULL, "is not il2cpp", "error", NULL);
			return;
		}
		Getil2cppFunc();
	}

	void il2cppCollector::Getil2cppFunc()
	{
		#define DO_API(ret_type, name, args )\
		name = reinterpret_cast<name##_t>(GetProcAddress(hModuleMono, #name));
		#include "il2cppCollectorAPI.h"
		#undef DO_API
	}

	DWORD_PTR il2cppCollector::il2cpp_GetMethodMemAddress(MethodInfo* method)
	{
		if(!method) return 0;
		return *reinterpret_cast<DWORD_PTR*>(method);
	}

	bool il2cppCollector::il2cpp_GetMethodInstance(MethodInfo* method)
	{
		return il2cpp_method_is_instance(method);
	}

	MethodInfo* il2cppCollector::il2cpp_GetMethodFromName(Il2CppClass* klass, std::string name, DWORD argscount)
	{
		return (MethodInfo*)il2cpp_class_get_method_from_name(klass, name.c_str(), argscount);
	}

	Il2CppType* il2cppCollector::il2cpp_GetMethodParam(MethodInfo* Method, DWORD index)
	{
		return (Il2CppType*)il2cpp_method_get_param(Method, index);
	}

	std::string il2cppCollector::il2cpp_GetMethodParamName(MethodInfo* Method, DWORD index)
	{
		return il2cpp_method_get_param_name(Method, index);
	}

	DWORD il2cppCollector::il2cpp_GetMethodParamCount(MethodInfo* Method)
	{
		return il2cpp_method_get_param_count(Method);
	}

	Il2CppType* il2cppCollector::il2cpp_GetMethodRetType(MethodInfo* Method)
	{
		return (Il2CppType*)il2cpp_method_get_return_type(Method);
	}

	std::string il2cppCollector::il2cpp_GetMethodName(MethodInfo* Method)
	{
		return il2cpp_method_get_name(Method);
	}

	std::string il2cppCollector::il2cpp_GetClassNamespace(Il2CppClass* klass)
	{
		return il2cpp_class_get_namespace(klass);
	}

	DWORD il2cppCollector::il2cpp_GetClassCount(Il2CppImage* Image)
	{
		return il2cpp_image_get_class_count(Image);
	}

	Il2CppType* il2cppCollector::il2cpp_GetFieldType(FieldInfo* field)
	{
		return (Il2CppType*)il2cpp_field_get_type(field);
	}

	std::string il2cppCollector::il2cpp_GetTypeName(Il2CppType* type)
	{
		return il2cpp_type_get_name(type);
	}

	std::string il2cppCollector::il2cpp_GetFieldName(FieldInfo* field)
	{
		return il2cpp_field_get_name(field);
	}

	std::string il2cppCollector::il2cpp_GetClassName(Il2CppClass* klass)
	{
		return il2cpp_class_get_name(klass);
	}

	Il2CppClass* il2cppCollector::il2cpp_GetClassFromName(Il2CppImage* image, std::string name, std::string namespaze)
	{
		return il2cpp_class_from_name(image, name.c_str(), namespaze.c_str());
	}

	DWORD_PTR il2cppCollector::il2cpp_GetFieldStatic(FieldInfo* field)
	{
		void* val;
		il2cpp_field_static_get_value(field, &val);
		return (DWORD_PTR)val;
	}

	FieldInfo* il2cppCollector::il2cpp_GetFieldFromName(Il2CppClass* klass, std::string name)
	{
		return il2cpp_class_get_field_from_name(klass, name.c_str());
	}

	DWORD il2cppCollector::il2cpp_EnumMethods(Il2CppClass* klass, std::vector<MethodInfo*>& Methods)
	{
		void* iter = NULL;
		MethodInfo* method;
		do
		{
			method = (MethodInfo*)il2cpp_class_get_methods(klass, &iter);
			if (!method) continue;
			Methods.push_back(method);
		} while (method);
		return Methods.size();
	}

	DWORD il2cppCollector::il2cpp_EnumFields(Il2CppClass* klass, std::vector<FieldInfo*>& Fields)
	{
		void* iter = NULL;
		FieldInfo* field;
		do
		{
			field = il2cpp_class_get_fields(klass, &iter);
			if (!field) continue;
			Fields.push_back(field);
		} while (field);
		return Fields.size();
	}

	DWORD il2cppCollector::il2cpp_EnumClasses(Il2CppImage* Image, std::vector<Il2CppClass*>& Classes)
	{
		DWORD count = il2cpp_image_get_class_count(Image);
		Classes.reserve(count);
		for (DWORD i = 0; i < count; i++)
		{
			Il2CppClass* Class = (Il2CppClass*)il2cpp_image_get_class(Image, i);
			if (!Class) continue;
			Classes.push_back(Class);
		}
		return Classes.size();
	}

	std::string il2cppCollector::il2cpp_GetImageName(Il2CppImage* Image)
	{
		return il2cpp_image_get_name(Image);
	}

	std::string il2cppCollector::il2cpp_GetImageFile(Il2CppImage* Image)
	{
		return il2cpp_image_get_filename(Image);
	}

	Il2CppImage* il2cppCollector::il2cpp_GetImage(Il2CppAssembly* Assembly)
	{
		return (Il2CppImage*)il2cpp_assembly_get_image(Assembly);
	}

	DWORD il2cppCollector::il2cpp_EnumDomains(std::vector<Il2CppDomain*>& Domains)
	{
		Domains.push_back(il2cpp_domain_get());
		return Domains.size();
	}

	DWORD il2cppCollector::il2cpp_EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys)
	{
		SIZE_T nrofassemblies = 0;
		Il2CppAssembly** assemblies = (Il2CppAssembly**)il2cpp_domain_get_assemblies(il2cpp_domain_get(), &nrofassemblies);
		for (int i = 0; i < nrofassemblies; i++)
			Assemblys.push_back(assemblies[i]);
		return Assemblys.size();
	}
}
