#include "MonoCollector.h"

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
	MonoCollector::~MonoCollector()
	{

	}

	MonoCollector::MonoCollector(std::string ModuleName)
	{
		hModuleMono = GetModuleHandleA(ModuleName.c_str());

		if (!hModuleMono)
		{
			MessageBox(NULL, "Can not find Module", "error", NULL);
			return;
		}

		if (GetProcAddress(hModuleMono, "il2cpp_thread_attach"))
		{
			il2cpp = true;
		}

		if (il2cpp)
		{
			Getil2cppFunc();
		}
		else
		{
			GetMonoFunc();
		}
	}

	void MonoCollector::GetMonoFunc()
	{
		
	}

	void MonoCollector::Getil2cppFunc()
	{
		#define DO_API(ret_type, name, args )\
		name = reinterpret_cast<name##_t>(GetProcAddress(hModuleMono, #name));
		#include "CollectorAPI.h"
		#undef DO_API
	}

	std::string MonoCollector::il2cpp_GetClassFieldName(FieldInfo* field)
	{
		return il2cpp_field_get_name(field);
	}

	std::string MonoCollector::il2cpp_GetImageClassName(Il2CppClass* klass)
	{
		return il2cpp_class_get_name(klass);
	}

	Il2CppClass* MonoCollector::il2cpp_GetImageClassFromName(Il2CppImage* image, std::string name, std::string namespaze = "")
	{
		return il2cpp_class_from_name(image, name.c_str(), namespaze.c_str());
	}

	DWORD_PTR MonoCollector::il2cpp_GetFieldStatic(FieldInfo* field)
	{
		void* val;
		il2cpp_field_static_get_value(field, &val);
		return (DWORD_PTR)val;
	}

	FieldInfo* MonoCollector::il2cpp_GetClassFieldFromName(Il2CppClass* klass, std::string name)
	{
		return il2cpp_class_get_field_from_name(klass, name.c_str());
	}

	DWORD MonoCollector::il2cpp_EnumMethodsInClass(Il2CppClass* klass, std::vector<MethodsInfo>& Methods)
	{
		void* iter = NULL;
		MethodInfo* method;
		do
		{
			method = (MethodInfo*)il2cpp_class_get_methods(klass, &iter);
			if (!method)
				continue;
			std::string sName = il2cpp_method_get_name(method);
			Methods.push_back({ method , sName });
		} while (method);
	}

	DWORD MonoCollector::il2cpp_EnumFieldsInClass(Il2CppClass* klass, std::vector<FieldsInfo>& Fields)
	{
		void* iter = NULL;
		FieldInfo* field;
		do
		{
			field = il2cpp_class_get_fields(klass, &iter);
			if (!field)
				continue;
			Il2CppType* fieldtype = (Il2CppType*)il2cpp_field_get_type(field);
			std::string sName = il2cpp_field_get_name(field);
			std::string sType = il2cpp_type_get_name(fieldtype);
			Fields.push_back({ field , sName ,sType });
		} while (field);
	}

	DWORD MonoCollector::il2cpp_EnumClassesInImage(Il2CppImage* Image, std::vector<ClassInfo>& Classes)
	{
		if (!Image) return 0;
		if (il2cpp)
		{
			DWORD count = il2cpp_image_get_class_count(Image);
			Classes.reserve(count);
			for (DWORD i = 0; i < count; i++)
			{
				Il2CppClass* Class = (Il2CppClass*)il2cpp_image_get_class(Image, i);
				if (!Class) continue;
				std::string Name = il2cpp_class_get_name(Class);
				std::string Name_space = il2cpp_class_get_namespace(Class);
				Classes.push_back({ Class , Name , Name_space });
			}
			return Classes.size();
		}
		else
		{
			
		}
	}

	std::string MonoCollector::il2cpp_GetImageName(Il2CppImage* Image)
	{
		return il2cpp_image_get_name(Image);
	}

	std::string MonoCollector::il2cpp_GetImageFile(Il2CppImage* Image)
	{
		return il2cpp_image_get_filename(Image);
	}

	Il2CppImage* MonoCollector::il2cpp_GetImageInAssembly(Il2CppAssembly* Assembly)
	{
		return (Il2CppImage*)il2cpp_assembly_get_image(Assembly);
	}

	DWORD MonoCollector::il2cpp_EnumDomains(std::vector<Il2CppDomain*>& Domains)
	{
		if (il2cpp)
		{
			Domains.push_back(il2cpp_domain_get());
			return Domains.size();
		}
		else
		{
			
		}
	}

	DWORD MonoCollector::il2cpp_EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys)
	{
		if (il2cpp)
		{
			SIZE_T nrofassemblies = 0;
			 Il2CppAssembly** assemblies = (Il2CppAssembly**)il2cpp_domain_get_assemblies(il2cpp_domain_get(), &nrofassemblies);

			for (int i = 0; i < nrofassemblies; i++)
				Assemblys.push_back(assemblies[i]);
			return Assemblys.size();
		}
		else
		{
		
		}
	}
}
