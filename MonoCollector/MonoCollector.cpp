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
		GetFunc();
	}

	void MonoCollector::GetFunc()
	{
		#define DO_API(ret_type, name, args )\
		name = reinterpret_cast<name##_t>(GetProcAddress(hModuleMono, #name));
		#include "MonoCollectorAPI.h"
		#undef DO_API
	}

	void MonoCollector::il2cpp_Dump2File(std::string file)
	{
		std::ofstream io(file);
		if (!io) return;
		std::stringstream str;
		std::vector<Il2CppAssembly*> Assemblys;
		for (size_t i = 0, max = il2cpp_EnummAssembly(Assemblys); i < max; i++)
		{
			auto image = il2cpp_GetImage(Assemblys[i]);
			str << std::format("// Image {}: {} - {}", i, il2cpp_GetImageName(image), il2cpp_GetCount(image)) << std::endl;
		}
		str << std::endl << std::endl << std::endl;
		for (size_t i = 0, max = Assemblys.size(); i < max; i++)
		{
			auto image = il2cpp_GetImage(Assemblys[i]);
			std::vector<Il2CppClass*> Classes;
			for (size_t i_c = 0,max_c = il2cpp_EnumClasses(image, Classes); i_c < max_c; i_c++)
			{
				str << std::format("// Namespace: {}\npublic class {}\n", il2cpp_GetClassNamespace(Classes[i_c]), il2cpp_GetClassName(Classes[i_c])) << "{\n\t// Fields" << std::endl;


				std::vector<FieldInfo*> Field;
				for (size_t i_f = 0,max_f = il2cpp_EnumFields(Classes[i_c], Field); i_f < max_f; i_f++)
				{
					str << std::format("\tpublic {} {}; // {:#08X}", il2cpp_GetTypeName(il2cpp_GetFieldType(Field[i_f])), il2cpp_GetFieldName(Field[i_f]), il2cpp_GetFieldOffset(Field[i_f])) << std::endl;
				}
				str << std::endl << std::endl << std::endl << "\t// Methods" << std::endl;

				std::vector<MethodInfo*> Methods;
				for (size_t i_m = 0,max_m = il2cpp_EnumMethods(Classes[i_c], Methods); i_m < max_m; i_m++)
				{
					str << std::format("\t// RVA: {:#08X}", il2cpp_GetMethodAddress(Methods[i_m]) - (DWORD_PTR)hModuleMono) << std::endl;
					str << std::format("\tpublic {} {}(", il2cpp_GetTypeName(il2cpp_GetMethodRetType(Methods[i_m])), il2cpp_GetMethodName(Methods[i_m]));

					for (size_t i_p = 0, max_p = il2cpp_GetMethodParamCount(Methods[i_m]); i_p < max_p; i_p++)
					{
						str << il2cpp_GetTypeName(il2cpp_GetMethodParam(Methods[i_m], i_p)) << " " << il2cpp_GetMethodParamName(Methods[i_m], i_p) << (1 == (max_p - i_p) ? "" : ", ");
					}
					str << ");\n\n";
				}
				str << "}" << std::endl << std::endl << std::endl;
			}
		}
		io << str.str();
		io.close();
	}

	DWORD MonoCollector::il2cpp_GetCount(Il2CppImage* Image)
	{
		return il2cpp_image_get_class_count(Image);
	}

	DWORD MonoCollector::il2cpp_GetFieldOffset(FieldInfo* field)
	{
		return il2cpp_field_get_offset(field);
	}

	DWORD_PTR MonoCollector::il2cpp_GetMethodAddress(std::string klass, std::string Method, std::string Image, std::string namespaze)
	{
		std::vector<Il2CppAssembly*> Assemblys;
		il2cpp_EnummAssembly(Assemblys);
		for (size_t i = 0, max = Assemblys.size(); i < max; i++)
		{
			//判断是否在镜像内
			Il2CppImage* image = il2cpp_GetImage(Assemblys[i]);
			if (Image != "") if (il2cpp_GetImageName(image) != Image) continue;

			//枚举类
			std::vector<Il2CppClass*> Classes;
			il2cpp_EnumClasses(image, Classes);
			for (size_t i_c = 0, max_c = Classes.size(); i_c < max_c; i_c++)
			{
				//判断空间命名是否相等
				if (namespaze != "") if (il2cpp_GetClassNamespace(Classes[i_c]) != namespaze) continue;

				//判断类名是否相等
				if (il2cpp_GetClassName(Classes[i_c]) != klass) continue;

				//枚举函数
				std::vector<MethodInfo*> Methods;
				il2cpp_EnumMethods(Classes[i_c], Methods);
				for (size_t i_m = 0, max_m = Methods.size(); i_m < max_m; i_m++)
				{
					if (il2cpp_GetMethodName(Methods[i_m]) != Method) continue;
					return il2cpp_GetMethodAddress(Methods[i_m]);
				}
			}
		}
		return 0;
	}

	DWORD_PTR MonoCollector::il2cpp_GetMethodAddress(MethodInfo* method)
	{

		if(!method) return 0;
		return *reinterpret_cast<DWORD_PTR*>(method);
	}

	bool MonoCollector::il2cpp_GetMethodInstance(MethodInfo* method)
	{
		return il2cpp_method_is_instance(method);
	}

	MethodInfo* MonoCollector::il2cpp_GetMethodFromName(Il2CppClass* klass, std::string name, DWORD argscount)
	{
		return (MethodInfo*)il2cpp_class_get_method_from_name(klass, name.c_str(), argscount);
	}

	Il2CppType* MonoCollector::il2cpp_GetMethodParam(MethodInfo* Method, DWORD index)
	{
		return (Il2CppType*)il2cpp_method_get_param(Method, index);
	}

	std::string MonoCollector::il2cpp_GetMethodParamName(MethodInfo* Method, DWORD index)
	{
		return il2cpp_method_get_param_name(Method, index);
	}

	DWORD MonoCollector::il2cpp_GetMethodParamCount(MethodInfo* Method)
	{
		return il2cpp_method_get_param_count(Method);
	}

	Il2CppType* MonoCollector::il2cpp_GetMethodRetType(MethodInfo* Method)
	{
		return (Il2CppType*)il2cpp_method_get_return_type(Method);
	}

	std::string MonoCollector::il2cpp_GetMethodName(MethodInfo* Method)
	{
		return il2cpp_method_get_name(Method);
	}

	std::string MonoCollector::il2cpp_GetClassNamespace(Il2CppClass* klass)
	{
		return il2cpp_class_get_namespace(klass);
	}

	DWORD MonoCollector::il2cpp_GetClassCount(Il2CppImage* Image)
	{
		return il2cpp_image_get_class_count(Image);
	}

	Il2CppType* MonoCollector::il2cpp_GetFieldType(FieldInfo* field)
	{
		return (Il2CppType*)il2cpp_field_get_type(field);
	}

	std::string MonoCollector::il2cpp_GetTypeName(Il2CppType* type)
	{
		return il2cpp_type_get_name(type);
	}

	std::string MonoCollector::il2cpp_GetFieldName(FieldInfo* field)
	{
		return il2cpp_field_get_name(field);
	}

	std::string MonoCollector::il2cpp_GetClassName(Il2CppClass* klass)
	{
		return il2cpp_class_get_name(klass);
	}

	Il2CppClass* MonoCollector::il2cpp_GetClassFromName(Il2CppImage* image, std::string name, std::string namespaze)
	{
		return il2cpp_class_from_name(image, name.c_str(), namespaze.c_str());
	}

	DWORD_PTR MonoCollector::il2cpp_GetFieldStatic(FieldInfo* field)
	{
		void* val;
		il2cpp_field_static_get_value(field, &val);
		return (DWORD_PTR)val;
	}

	FieldInfo* MonoCollector::il2cpp_GetFieldFromName(Il2CppClass* klass, std::string name)
	{
		return il2cpp_class_get_field_from_name(klass, name.c_str());
	}

	DWORD MonoCollector::il2cpp_EnumMethods(Il2CppClass* klass, std::vector<MethodInfo*>& Methods)
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

	DWORD MonoCollector::il2cpp_EnumFields(Il2CppClass* klass, std::vector<FieldInfo*>& Fields)
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

	DWORD MonoCollector::il2cpp_EnumClasses(Il2CppImage* Image, std::vector<Il2CppClass*>& Classes)
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

	std::string MonoCollector::il2cpp_GetImageName(Il2CppImage* Image)
	{
		return il2cpp_image_get_name(Image);
	}

	std::string MonoCollector::il2cpp_GetImageFile(Il2CppImage* Image)
	{
		return il2cpp_image_get_filename(Image);
	}

	Il2CppImage* MonoCollector::il2cpp_GetImage(Il2CppAssembly* Assembly)
	{
		return (Il2CppImage*)il2cpp_assembly_get_image(Assembly);
	}

	DWORD MonoCollector::il2cpp_EnumDomains(std::vector<Il2CppDomain*>& Domains)
	{
		Domains.push_back(il2cpp_domain_get());
		return Domains.size();
	}

	DWORD MonoCollector::il2cpp_EnummAssembly(std::vector<Il2CppAssembly*>& Assemblys)
	{
		size_t nrofassemblies = 0;
		Il2CppAssembly** assemblies = (Il2CppAssembly**)il2cpp_domain_get_assemblies(il2cpp_domain_get(), &nrofassemblies);
		for (int i = 0; i < nrofassemblies; i++)
			Assemblys.push_back(assemblies[i]);
		return Assemblys.size();
	}
}
