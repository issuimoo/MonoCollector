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
		hModuleMono = nullptr;
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

	MonoType* MonoCollector::Mono_GetMethodParam(MethodInfo* Method, DWORD index)
	{
		void* methodsignature = mono_method_signature(Method);

		int i = 0;

		void* iter = NULL;
		MonoType* method;
		do
		{
			method = mono_signature_get_params((MonoMethodSignature*)methodsignature, &iter);
			if (i == index)
			{
				return method;
			}
			i++;
		} while (method);
	}

	DWORD_PTR MonoCollector::Mono_GetMethodAddress(std::string klass, std::string Method, std::string Image, std::string namespaze)
	{
		std::vector<MonoAssembly*> Assemblys;
		Mono_EnummAssembly(Assemblys);
		for (size_t i = 0, max = Assemblys.size(); i < max; i++)
		{
			//判断是否在镜像内
			MonoImage* image = Mono_GetImage(Assemblys[i]);
			if (Image != "") if (Mono_GetImageName(image) != Image) continue;

			//枚举类
			std::vector<MonoClass*> Classes;
			Mono_EnumClasses(image, Classes);
			for (size_t i_c = 0, max_c = Classes.size(); i_c < max_c; i_c++)
			{
				//判断空间命名是否相等
				if (namespaze != "") if (Mono_GetClassNamespace(Classes[i_c]) != namespaze) continue;

				//判断类名是否相等
				if (Mono_GetClassName(Classes[i_c]) != klass) continue;

				//枚举函数
				std::vector<MethodInfo*> Methods;
				Mono_EnumMethods(Classes[i_c], Methods);
				for (size_t i_m = 0, max_m = Methods.size(); i_m < max_m; i_m++)
				{
					if (Mono_GetMethodName(Methods[i_m]) != Method) continue;
					return Mono_GetMethodAddress(Methods[i_m]);
				}
			}
		}
		return 0;
	}

	DWORD_PTR MonoCollector::Mono_GetMethodAddress(MethodInfo* method)
	{
		if (!method) return 0;
		return *reinterpret_cast<DWORD_PTR*>(method);
	}

	std::string MonoCollector::Mono_GetMethodParamName(MethodInfo* Method, DWORD index)
	{
		void* methodsignature = mono_method_signature(Method);
		int paramcount = mono_signature_get_param_count(methodsignature);
		char** names = (char**)calloc(sizeof(char*), paramcount);
		mono_method_get_param_names(Method, (const char**)names);
		std::string name = names[index];
		free(names);
		return name;
	}

	DWORD MonoCollector::Mono_GetMethodParamCount(MethodInfo* Method)
	{
		void* methodsignature = mono_method_signature(Method);
		return mono_signature_get_param_count(methodsignature);
	}

	MonoType* MonoCollector::Mono_GetMethodRetType(MethodInfo* Method)
	{
		void* methodsignature = mono_method_signature(Method);
		return mono_signature_get_return_type(methodsignature);
	}

	std::string MonoCollector::Mono_GetMethodName(MethodInfo* Method)
	{
		return mono_method_get_name(Method);
	}

	MethodInfo* MonoCollector::Mono_GetMethodFromName(MonoClass* klass, std::string name, DWORD argscount)
	{
		return mono_class_get_method_from_name(klass, name.c_str(), argscount);
	}

	DWORD MonoCollector::Mono_EnumMethods(MonoClass* klass, std::vector<MethodInfo*>& Methods)
	{
		void* iter = NULL;
		MethodInfo* method;
		do
		{
			method = (MethodInfo*)mono_class_get_methods(klass, &iter);
			if (!method) continue;
			Methods.push_back(method);
		} while (method);
		return Methods.size();
	}

	DWORD MonoCollector::Mono_GetFieldOffset(FieldInfo* field)
	{
		return mono_field_get_offset(field);
	}

	MonoType* MonoCollector::Mono_GetFieldType(FieldInfo* field)
	{
		return mono_field_get_type(field);
	}

	std::string MonoCollector::Mono_GetFieldName(FieldInfo* field)
	{
		return mono_field_get_name(field);
	}

	DWORD_PTR MonoCollector::Mono_GetFieldStatic(FieldInfo* field, MonoClass* klass)
	{
		QWORD val = 0;
		mono_field_static_get_value(mono_class_vtable(mono_get_root_domain(), klass), field, &val);
		return val;
	}

	FieldInfo* MonoCollector::Mono_GetFieldFromName(MonoClass* klass, std::string name)
	{
		return mono_class_get_field_from_name(klass, name.c_str());
	}

	DWORD MonoCollector::Mono_EnumFields(MonoClass* klass, std::vector<FieldInfo*>& Fields)
	{
		void* iter = NULL;
		FieldInfo* field;
		do
		{
			field = mono_class_get_fields(klass, &iter);
			if (!field) continue;
			Fields.push_back(field);
		} while (field);
		return Fields.size();
	}

	std::string MonoCollector::Mono_GetClassNamespace(MonoClass* klass)
	{
		return mono_class_get_namespace(klass);
	}

	std::string MonoCollector::Mono_GetClassName(MonoClass* klass)
	{
		return mono_class_get_name(klass);
	}

	MonoClass* MonoCollector::Mono_GetClassFromName(MonoImage* image, std::string name, std::string namespaze)
	{
		return mono_class_from_name(image, namespaze.c_str(), name.c_str());
	}

	DWORD MonoCollector::Mono_EnumClasses(MonoImage* Image, std::vector<MonoClass*>& Classes)
	{
		MonoTable* tdef = mono_image_get_table_info(Image, MONO_TABLE_TYPEDEF);
		if (!tdef) return 0;
		int tdefcount = mono_table_info_get_rows(tdef);
		for (int i = 0; i < tdefcount; i++)
		{
			MonoClass* c = mono_class_get(Image, MONO_TOKEN_TYPE_DEF | (i + 1));
			if (c != NULL) continue;
			Classes.push_back(c);
		}
	}

	MonoImage* MonoCollector::Mono_GetImage(MonoAssembly* Assembly)
	{
		return mono_assembly_get_image(Assembly);
	}

	std::string MonoCollector::Mono_GetImageName(MonoImage* Image)
	{
		return mono_image_get_name(Image);
	}

	std::string MonoCollector::Mono_GetImageFile(MonoImage* Image)
	{
		return mono_image_get_filename(Image);
	}

	DWORD MonoCollector::Mono_GetClassCount(MonoImage* Image)
	{
		MonoTable* tdef = mono_image_get_table_info(Image, MONO_TABLE_TYPEDEF);
		return mono_table_info_get_rows(tdef);
	}

	std::string MonoCollector::Mono_GetTypeName(MonoType* type)
	{
		return mono_type_get_name(type);
	}

	DWORD MonoCollector::Mono_EnummAssembly(std::vector<MonoAssembly*>& Assemblys)
	{
		mono_assembly_foreach((GFunc)UntiyMono::vectorpushback, &Assemblys);
		return Assemblys.size();
	}

	DWORD MonoCollector::Mono_EnumDomains(std::vector<MonoDomain*>& Domains)
	{

		mono_domain_foreach((GFunc)UntiyMono::vectorpushback, &Domains);
		return Domains.size();
	}

	void MonoCollector::il2cpp_Dump2File(std::string file)
	{
		std::ofstream io(file + "\\dump.cs");
		std::ofstream io2(file + "\\il2cpp.hpp");
		std::ofstream io3(file + "\\il2cpp-func.hpp");

		if (!io) return;
		std::stringstream str;
		std::stringstream str2;
		std::stringstream str3;

		std::vector<Il2CppAssembly*> Assemblys;
		for (size_t i = 0, max = il2cpp_EnummAssembly(Assemblys); i < max; i++)
		{
			auto image = il2cpp_GetImage(Assemblys[i]);
			str << std::format("// Image {}: {} - {}", i, il2cpp_GetImageName(image), il2cpp_GetClassCount(image)) << std::endl;
		}
		str << std::endl << std::endl << std::endl;
		for (size_t i = 0, max = Assemblys.size(); i < max; i++)
		{
			auto image = il2cpp_GetImage(Assemblys[i]);
			std::vector<Il2CppClass*> Classes;
			for (size_t i_c = 0,max_c = il2cpp_EnumClasses(image, Classes); i_c < max_c; i_c++)
			{
				str << std::format("// Namespace: {}\npublic class {}\n", il2cpp_GetClassNamespace(Classes[i_c]), il2cpp_GetClassName(Classes[i_c])) << "{\n\t// Fields" << std::endl;
				str2 << std::format("struct {}\n", il2cpp_GetClassName(Classes[i_c])) << "{" << std::endl;

				std::vector<FieldInfo*> Field;
				for (size_t i_f = 0,max_f = il2cpp_EnumFields(Classes[i_c], Field); i_f < max_f; i_f++)
				{
					auto Typename = il2cpp_GetTypeName(il2cpp_GetFieldType(Field[i_f]));
					std::string Type = Typename;
					if (Typename.find_last_of(".") != std::string::npos)
					{
						Type = Typename.substr(Typename.find_last_of(".") + 1, Typename.length());
					}
					str << std::format("\tpublic {} {}; // {:#X}", Type, il2cpp_GetFieldName(Field[i_f]), il2cpp_GetFieldOffset(Field[i_f])) << std::endl;
					str2 << std::format("\t{} {}; // {:#X}", Type, il2cpp_GetFieldName(Field[i_f]), il2cpp_GetFieldOffset(Field[i_f])) << std::endl;
				}
				str << std::endl << std::endl << std::endl << "\t// Methods" << std::endl;
				str2 << "\n};" << std::endl;

				std::vector<MethodInfo*> Methods;
				for (size_t i_m = 0,max_m = il2cpp_EnumMethods(Classes[i_c], Methods); i_m < max_m; i_m++)
				{
					str << std::format("\t// RVA: {:#08X}", il2cpp_GetMethodAddress(Methods[i_m]) - (DWORD_PTR)hModuleMono) << std::endl;

					auto retTypename = il2cpp_GetTypeName(il2cpp_GetMethodRetType(Methods[i_m]));
					std::string retType = retTypename;
					if (retTypename.find_last_of(".") != std::string::npos)
					{
						retType = retTypename.substr(retTypename.find_last_of(".") + 1, retTypename.length());
					}
					str << std::format("\tpublic {} {}(", retType, il2cpp_GetMethodName(Methods[i_m]));
					str3 << std::format("DO_API({}, {}, (", retType, il2cpp_GetMethodName(Methods[i_m]));

					for (size_t i_p = 0, max_p = il2cpp_GetMethodParamCount(Methods[i_m]); i_p < max_p; i_p++)
					{
						auto Typename = il2cpp_GetTypeName(il2cpp_GetMethodParam(Methods[i_m], i_p));
						std::string Type = Typename;
						if (Typename.find_last_of(".") != std::string::npos)
						{
							Type = Typename.substr(Typename.find_last_of(".") + 1, Typename.length());
						}
						
						str << Type << " " << il2cpp_GetMethodParamName(Methods[i_m], i_p) << (1 == (max_p - i_p) ? "" : ", ");
						str3 << Type << " " << il2cpp_GetMethodParamName(Methods[i_m], i_p) << (1 == (max_p - i_p) ? "" : ", ");

					}
					str << ");\n\n";
					str3 << "));\n";
				}
				str << "}" << std::endl << std::endl << std::endl;
			}
		}
		io << str.str();
		io2 << str2.str();
		io3 << str3.str();
		io.close();
		io2.close();
		io3.close();
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
