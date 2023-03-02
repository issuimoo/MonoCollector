#include "MonoCollector.hpp"

WORD UTF8TOUTF16(char* szUtf8) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::u16string dest = convert.from_bytes(szUtf8);
	return *(WORD*)&dest[0];
}

bool Isil2cpp(MonoCollector* _this)
{
	return _this->il2cpp;
}

void __cdecl customfreeimplementation(PVOID address)
{
	
}

void _cdecl AssemblyEnumerator(void* domain, std::vector<UINT64>* v)
{
	v->push_back((UINT_PTR)domain);
}

void _cdecl DomainEnumerator(void* domain, std::vector<UINT64>* v)
{
	v->push_back((UINT_PTR)domain);
}

void MonoCollector::EnumFieldsInClass(void* c, void*& field, void*& fieldtype,int& type, UINT_PTR& field_parent, DWORD& field_offset, int& field_flags,std::vector<std::string>& Names, std::vector<std::string>& Types)
{
	void* iter = NULL;

	do
	{
		field = mono_class_get_fields(c, &iter);

		if (field)
		{
			fieldtype = mono_field_get_type(field);
			type = mono_type_get_type(fieldtype);
			field_parent = (UINT_PTR)mono_field_get_parent(field);
			field_offset = (DWORD)mono_field_get_offset(field);
			field_flags = mono_field_get_flags(field);



			char* name = mono_field_get_name(field);
			char* type = mono_type_get_name(fieldtype);
			std::string sName = std::string(name);
			std::string sType = std::string(type);
			//check if name is x ...

			if ((BYTE)name[0] == 0xEE) {
				char szUeName[32];
				sprintf_s(szUeName, 32, "\\u%04X", UTF8TOUTF16(name));
				sName = szUeName;
			}
			if ((BYTE)type[0] == 0xEE) {
				char szUeName[32];
				sprintf_s(szUeName, 32, "\\u%04X", UTF8TOUTF16(type));
				sType = szUeName;
			}

			Names.push_back(sName);

			if (type)
			{
				Types.push_back(sType);
			}
			else
				Types.push_back("");

		}
	} while (field);
}

bool MonoCollector::EnumClassesInImage(void* image,std::vector<UINT_PTR>& ret_ptr, std::vector<std::string>& ret_string_class, std::vector<std::string>& ret_string_namespace)
{
	int i;

	ret_ptr.clear();
	ret_string_class.clear();
	ret_string_namespace.clear();

	if (image == NULL)
	{
		return false;
	}

	if (il2cpp)
	{
		int count = 0;
		if (il2cpp_image_get_class_count)
		{
			count = il2cpp_image_get_class_count(image);
		}

		ret_ptr.reserve(count);
		ret_string_class.reserve(count);
		ret_string_namespace.reserve(count);

		for (i = 0; i < count; i++)
		{
			if (il2cpp_image_get_class)
			{
				void* c = il2cpp_image_get_class(image, i);
				ret_ptr.push_back((UINT_PTR)c);

				if (c)
				{
					std::string name = mono_class_get_name(c);
					ret_string_class.push_back(name);

					name = mono_class_get_namespace(c);
					ret_string_namespace.push_back(name);
				}
				else
				{
					ret_string_class.push_back("");
					ret_string_namespace.push_back("");
				}
			}
			else
			{
				ret_string_class.push_back("");
				ret_string_namespace.push_back("");
				ret_ptr.push_back((UINT_PTR)0);
			}
		}
	}
	else
	{

		void* tdef = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		if (tdef)
		{
			int tdefcount = mono_table_info_get_rows(tdef);
			ret_ptr.clear();
			ret_ptr.reserve(tdefcount);

			for (i = 0; i < tdefcount; i++)
			{
				void* c = mono_class_get(image, MONO_TOKEN_TYPE_DEF | (i + 1));
				if (c != NULL)
				{
					char* name = mono_class_get_name(c);

					ret_ptr.push_back((UINT_PTR)c);


					std::string sName = std::string(name);

					if ((BYTE)name[0] == 0xEE) {
						char szUeName[32];
						sprintf_s(szUeName, 32, "\\u%04X", UTF8TOUTF16(name));
						sName = szUeName;
					}

					if (c)
					{
						std::string s = sName;
						ret_string_class.push_back(s);
					}
					else
						ret_string_class.push_back("");

					name = mono_class_get_namespace(c);
					if (name)
					{
						std::string s = name;
						ret_string_namespace.push_back(s);
					}
					else
						ret_string_namespace.push_back("");
				}
				else
				{
					ret_string_class.push_back("");
					ret_string_namespace.push_back("");
					ret_ptr.push_back((UINT_PTR)0);
				}
			}
		}
		else
		{
			return false;
		}
	}
}

void MonoCollector::GetImageFileName(void* image, std::string& s)
{
	s = mono_image_get_filename(image);
}

void MonoCollector::GetImageName(void* image,std::string& s)
{
	s = mono_image_get_name(image);
}

int MonoCollector::SetCurrentDomain(void* domain)
{
	if (mono_domain_set)
		return mono_domain_set(domain, FALSE);
	else
		return 0;
}

bool MonoCollector::Object_GetClass(void* object, UINT64& r_klass, std::string& classname_p)
{
	char* classname;
	void* klass;

	try
	{
		unsigned int i;

		klass = mono_object_get_class(object);
		classname = mono_class_get_name(klass);

		//small test to see if the classname is readable
		for (i = 0; i < strlen(classname); i++)
		{
			char x = classname[i];
			if (x == '\0')
				break;
		}

		if (klass != 0)
		{
			r_klass = (UINT64)klass;
			classname_p = classname;
		}
		else
		{
			return false;
		}


	}
	catch (...)
	{
		return false; //failure. Invalid object
	}
}

UINT_PTR MonoCollector::GetImageFromAssembly(void* Assembly)
{
	void* image = mono_assembly_get_image(Assembly);
	return (UINT_PTR)image;
}

size_t MonoCollector::EnumDomains(std::vector<UINT64>& ret)
{
	if (il2cpp)
	{
		ret.push_back(UINT_PTR(mono_domain_get()));
		return 1;
	}
	else
	{
		mono_domain_foreach((MonoDomainFunc)DomainEnumerator, &ret);
		return ret.size();
	}
}

size_t MonoCollector::EnumAssemblies(std::vector<UINT64>& ret)
{
	unsigned int i;
	std::vector<UINT64> v;

	if (il2cpp)
	{
		DWORD i;

		SIZE_T nrofassemblies = 0;
		UINT_PTR* assemblies;
		assemblies = il2cpp_domain_get_assemblies(mono_domain_get(), &nrofassemblies);

		for (i = 0; i < (DWORD)nrofassemblies; i++)
			ret.push_back(assemblies[i]);
		return nrofassemblies;
	}
	else
	{
		if (mono_assembly_foreach)
		{
			mono_assembly_foreach((GFunc)AssemblyEnumerator, &ret);

			return ret.size();
		}
		else
		{
			return 0;
		}
	}
}

MonoCollector::~MonoCollector()
{
	mono_thread_detach(mono_selfthread);
}

MonoCollector::MonoCollector()
{
	HMODULE hMono = GetModuleHandle("mono.dll");

	if (!hMono)
	{
		//this process doesn't use mono.dll  Perhaps it's renamed.  Find a module that exports mono_thread_attach
		HANDLE ths = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
		if (ths != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 me;
			me.dwSize = sizeof(me);

			if (Module32First(ths, &me))
			{
				do
				{
					if (GetProcAddress(me.hModule, "mono_thread_attach"))
						hMono = me.hModule;

					if (GetProcAddress(me.hModule, "il2cpp_thread_attach"))
					{
						il2cpp = true;
						hMono = me.hModule;
					}

				} while (!hMono && Module32Next(ths, &me));

			}
			CloseHandle(ths);
		}
	}

	if (il2cpp)
	{
		g_free = (G_FREE)GetProcAddress(hMono, "g_free");

		if (!g_free)
			g_free = (G_FREE)GetProcAddress(hMono, "il2cpp_unity_g_free");

		if (!g_free)
			g_free = customfreeimplementation;

		mono_free = (MONO_FREE)GetProcAddress(hMono, "il2cpp_free");

		mono_get_root_domain = (MONO_GET_ROOT_DOMAIN)GetProcAddress(hMono, "il2cpp_get_root_domain");
		mono_thread_attach = (MONO_THREAD_ATTACH)GetProcAddress(hMono, "il2cpp_thread_attach");
		mono_thread_detach = (MONO_THREAD_DETACH)GetProcAddress(hMono, "il2cpp_thread_detach");

		mono_object_get_class = (MONO_OBJECT_GET_CLASS)GetProcAddress(hMono, "il2cpp_object_get_class");

		mono_domain_foreach = (MONO_DOMAIN_FOREACH)GetProcAddress(hMono, "il2cpp_domain_foreach");
		mono_domain_set = (MONO_DOMAIN_SET)GetProcAddress(hMono, "il2cpp_domain_set");
		mono_domain_get = (MONO_DOMAIN_GET)GetProcAddress(hMono, "il2cpp_domain_get");
		mono_assembly_foreach = (MONO_ASSEMBLY_FOREACH)GetProcAddress(hMono, "il2cpp_assembly_foreach");
		mono_assembly_get_image = (MONO_ASSEMBLY_GET_IMAGE)GetProcAddress(hMono, "il2cpp_assembly_get_image");
		mono_image_get_assembly = (MONO_IMAGE_GET_ASSEMBLY)GetProcAddress(hMono, "il2cpp_image_get_assembly");

		mono_image_get_name = (MONO_IMAGE_GET_NAME)GetProcAddress(hMono, "il2cpp_image_get_name");
		mono_image_get_table_info = (MONO_IMAGE_GET_TABLE_INFO)GetProcAddress(hMono, "mono_image_get_table_info");
		mono_image_rva_map = (MONO_IMAGE_RVA_MAP)GetProcAddress(hMono, "il2cpp_image_rva_map");

		mono_table_info_get_rows = (MONO_TABLE_INFO_GET_ROWS)GetProcAddress(hMono, "il2cpp_table_info_get_rows");
		mono_metadata_decode_row_col = (MONO_METADATA_DECODE_ROW_COL)GetProcAddress(hMono, "il2cpp_metadata_decode_row_col");
		mono_metadata_string_heap = (MONO_METADATA_STRING_HEAP)GetProcAddress(hMono, "il2cpp_metadata_string_heap");


		mono_class_get = (MONO_CLASS_GET)GetProcAddress(hMono, "il2cpp_class_get");
		mono_class_from_typeref = (MONO_CLASS_FROM_TYPEREF)GetProcAddress(hMono, "il2cpp_class_from_typeref");
		mono_class_name_from_token = (MONO_CLASS_NAME_FROM_TOKEN)GetProcAddress(hMono, "il2cpp_class_name_from_token");
		mono_class_from_name_case = (MONO_CLASS_FROM_NAME_CASE)GetProcAddress(hMono, "il2cpp_class_from_name_case");
		mono_class_from_name = (MONO_CLASS_FROM_NAME_CASE)GetProcAddress(hMono, "il2cpp_class_from_name");
		mono_class_get_name = (MONO_CLASS_GET_NAME)GetProcAddress(hMono, "il2cpp_class_get_name");
		mono_class_get_namespace = (MONO_CLASS_GET_NAMESPACE)GetProcAddress(hMono, "il2cpp_class_get_namespace");
		mono_class_get_methods = (MONO_CLASS_GET_METHODS)GetProcAddress(hMono, "il2cpp_class_get_methods");
		mono_class_get_method_from_name = (MONO_CLASS_GET_METHOD_FROM_NAME)GetProcAddress(hMono, "il2cpp_class_get_method_from_name");
		mono_class_get_fields = (MONO_CLASS_GET_FIELDS)GetProcAddress(hMono, "il2cpp_class_get_fields");
		mono_class_get_parent = (MONO_CLASS_GET_PARENT)GetProcAddress(hMono, "il2cpp_class_get_parent");
		mono_class_get_image = (MONO_CLASS_GET_IMAGE)GetProcAddress(hMono, "il2cpp_class_get_image");

		mono_class_is_generic = (MONO_CLASS_IS_GENERIC)GetProcAddress(hMono, "il2cpp_class_is_generic");
		mono_class_vtable = (MONO_CLASS_VTABLE)GetProcAddress(hMono, "il2cpp_class_vtable");
		mono_class_from_mono_type = (MONO_CLASS_FROM_MONO_TYPE)GetProcAddress(hMono, "il2cpp_class_from_mono_type");
		mono_class_get_element_class = (MONO_CLASS_GET_ELEMENT_CLASS)GetProcAddress(hMono, "il2cpp_class_get_element_class");
		mono_class_instance_size = (MONO_CLASS_INSTANCE_SIZE)GetProcAddress(hMono, "il2cpp_class_instance_size");

		mono_class_num_fields = (MONO_CLASS_NUM_FIELDS)GetProcAddress(hMono, "il2cpp_class_num_fields");
		mono_class_num_methods = (MONO_CLASS_NUM_METHODS)GetProcAddress(hMono, "il2cpp_class_num_methods");


		mono_field_get_name = (MONO_FIELD_GET_NAME)GetProcAddress(hMono, "il2cpp_field_get_name");
		mono_field_get_type = (MONO_FIELD_GET_TYPE)GetProcAddress(hMono, "il2cpp_field_get_type");
		mono_field_get_parent = (MONO_FIELD_GET_PARENT)GetProcAddress(hMono, "il2cpp_field_get_parent");
		mono_field_get_offset = (MONO_FIELD_GET_OFFSET)GetProcAddress(hMono, "il2cpp_field_get_offset");
		mono_field_get_flags = (MONO_FIELD_GET_FLAGS)GetProcAddress(hMono, "il2cpp_field_get_flags");

		mono_type_get_name = (MONO_TYPE_GET_NAME)GetProcAddress(hMono, "il2cpp_type_get_name");
		mono_type_get_type = (MONO_TYPE_GET_TYPE)GetProcAddress(hMono, "il2cpp_type_get_type");
		mono_type_get_name_full = (MONO_TYPE_GET_NAME_FULL)GetProcAddress(hMono, "il2cpp_type_get_name_full");

		mono_method_get_name = (MONO_METHOD_GET_NAME)GetProcAddress(hMono, "il2cpp_method_get_name");
		mono_method_get_class = (MONO_METHOD_GET_CLASS)GetProcAddress(hMono, "il2cpp_method_get_class");
		mono_method_get_header = (MONO_METHOD_GET_HEADER)GetProcAddress(hMono, "il2cpp_method_get_header");
		mono_method_get_flags = (MONO_METHOD_GET_FLAGS)GetProcAddress(hMono, "il2cpp_method_get_flags");
		mono_method_signature = (MONO_METHOD_SIG)GetProcAddress(hMono, "il2cpp_method_signature");
		mono_method_get_param_names = (MONO_METHOD_GET_PARAM_NAMES)GetProcAddress(hMono, "il2cpp_method_get_param_names");



		mono_signature_get_desc = (MONO_SIGNATURE_GET_DESC)GetProcAddress(hMono, "il2cpp_signature_get_desc");
		mono_signature_get_params = (MONO_SIGNATURE_GET_PARAMS)GetProcAddress(hMono, "il2cpp_signature_get_params");
		mono_signature_get_param_count = (MONO_SIGNATURE_GET_PARAM_COUNT)GetProcAddress(hMono, "il2cpp_signature_get_param_count");
		mono_signature_get_return_type = (MONO_SIGNATURE_GET_RETURN_TYPE)GetProcAddress(hMono, "il2cpp_signature_get_return_type");



		mono_compile_method = (MONO_COMPILE_METHOD)GetProcAddress(hMono, "il2cpp_compile_method");
		mono_free_method = (MONO_FREE_METHOD)GetProcAddress(hMono, "il2cpp_free_method");
		mono_jit_info_table_find = (MONO_JIT_INFO_TABLE_FIND)GetProcAddress(hMono, "il2cpp_jit_info_table_find");
		mono_jit_info_get_method = (MONO_JIT_INFO_GET_METHOD)GetProcAddress(hMono, "il2cpp_jit_info_get_method");
		mono_jit_info_get_code_start = (MONO_JIT_INFO_GET_CODE_START)GetProcAddress(hMono, "il2cpp_jit_info_get_code_start");
		mono_jit_info_get_code_size = (MONO_JIT_INFO_GET_CODE_SIZE)GetProcAddress(hMono, "il2cpp_jit_info_get_code_size");
		mono_jit_exec = (MONO_JIT_EXEC)GetProcAddress(hMono, "il2cpp_jit_exec");

		mono_method_header_get_code = (MONO_METHOD_HEADER_GET_CODE)GetProcAddress(hMono, "il2cpp_method_header_get_code");
		mono_disasm_code = (MONO_DISASM_CODE)GetProcAddress(hMono, "il2cpp_disasm_code");

		mono_vtable_get_static_field_data = (MONO_VTABLE_GET_STATIC_FIELD_DATA)GetProcAddress(hMono, "il2cpp_vtable_get_static_field_data");

		mono_method_desc_new = (MONO_METHOD_DESC_NEW)GetProcAddress(hMono, "il2cpp_method_desc_new");;
		mono_method_desc_from_method = (MONO_METHOD_DESC_FROM_METHOD)GetProcAddress(hMono, "il2cpp_method_desc_from_method");;
		mono_method_desc_free = (MONO_METHOD_DESC_FREE)GetProcAddress(hMono, "il2cpp_method_desc_free");;

		mono_string_new = (MONO_STRING_NEW)GetProcAddress(hMono, "il2cpp_string_new");
		mono_string_to_utf8 = (MONO_STRING_TO_UTF8)GetProcAddress(hMono, "il2cpp_string_to_utf8");
		mono_array_new = (MONO_ARRAY_NEW)GetProcAddress(hMono, "il2cpp_array_new");
		mono_value_box = (MONO_VALUE_BOX)GetProcAddress(hMono, "il2cpp_value_box");
		mono_object_unbox = (MONO_OBJECT_UNBOX)GetProcAddress(hMono, "il2cpp_object_unbox");
		mono_object_new = (MONO_OBJECT_NEW)GetProcAddress(hMono, "il2cpp_object_new");

		mono_class_get_type = (MONO_CLASS_GET_TYPE)GetProcAddress(hMono, "il2cpp_class_get_type");

		mono_method_desc_search_in_image = (MONO_METHOD_DESC_SEARCH_IN_IMAGE)GetProcAddress(hMono, "il2cpp_method_desc_search_in_image");
		mono_runtime_invoke = (MONO_RUNTIME_INVOKE)GetProcAddress(hMono, "il2cpp_runtime_invoke");
		mono_runtime_object_init = (MONO_RUNTIME_OBJECT_INIT)GetProcAddress(hMono, "il2cpp_runtime_object_init");


		mono_assembly_name_new = (MONO_ASSEMBLY_NAME_NEW)GetProcAddress(hMono, "il2cpp_assembly_name_new");
		mono_assembly_loaded = (MONO_ASSEMBLY_LOADED)GetProcAddress(hMono, "il2cpp_assembly_loaded");
		mono_assembly_open = (MONO_ASSEMBLY_OPEN)GetProcAddress(hMono, "il2cpp_assembly_open");
		mono_image_open = (MONO_IMAGE_OPEN)GetProcAddress(hMono, "il2cpp_image_open");
		mono_image_get_filename = (MONO_IMAGE_GET_FILENAME)GetProcAddress(hMono, "il2cpp_image_get_filename");

		mono_class_get_nesting_type = (MONO_CLASS_GET_NESTING_TYPE)GetProcAddress(hMono, "mono_class_get_nesting_type");

		il2cpp_field_static_get_value = (IL2CPP_FIELD_STATIC_GET_VALUE)GetProcAddress(hMono, "il2cpp_field_static_get_value");
		il2cpp_field_static_set_value = (IL2CPP_FIELD_STATIC_SET_VALUE)GetProcAddress(hMono, "il2cpp_field_static_set_value");


		il2cpp_domain_get_assemblies = (IL2CPP_DOMAIN_GET_ASSEMBLIES)GetProcAddress(hMono, "il2cpp_domain_get_assemblies");
		il2cpp_image_get_class_count = (IL2CPP_IMAGE_GET_CLASS_COUNT)GetProcAddress(hMono, "il2cpp_image_get_class_count");
		il2cpp_image_get_class = (IL2CPP_IMAGE_GET_CLASS)GetProcAddress(hMono, "il2cpp_image_get_class");

		il2cpp_type_get_name = (IL2CPP_TYPE_GET_NAME)GetProcAddress(hMono, "il2cpp_type_get_name");
		il2cpp_type_get_assembly_qualified_name = (IL2CPP_TYPE_GET_ASSEMBLY_QUALIFIED_NAME)GetProcAddress(hMono, "il2cpp_type_get_assembly_qualified_name");

		il2cpp_method_get_param_count = (IL2CPP_METHOD_GET_PARAM_COUNT)GetProcAddress(hMono, "il2cpp_method_get_param_count");
		il2cpp_method_get_param_name = (IL2CPP_METHOD_GET_PARAM_NAME)GetProcAddress(hMono, "il2cpp_method_get_param_name");
		il2cpp_method_get_param = (IL2CPP_METHOD_GET_PARAM)GetProcAddress(hMono, "il2cpp_method_get_param");
		il2cpp_method_get_return_type = (IL2CPP_METHOD_GET_RETURN_TYPE)GetProcAddress(hMono, "il2cpp_method_get_return_type");

		il2cpp_class_from_type = (IL2CPP_CLASS_FROM_TYPE)GetProcAddress(hMono, "il2cpp_class_from_type");
		il2cpp_string_chars = (IL2CPP_STRING_CHARS)GetProcAddress(hMono, "il2cpp_string_chars");

		//mono_runtime_is_shutting_down = (MONO_RUNTIME_IS_SHUTTING_DOWN)GetProcAddress(hMono, "il2cpp_runtime_is_shutting_down");  //doesn't seem to exist in il2cpp....

		mono_runtime_is_shutting_down = (MONO_RUNTIME_IS_SHUTTING_DOWN)GetProcAddress(hMono, "mono_runtime_is_shutting_down"); //some do, with this name...
		domain = mono_domain_get();

	}
	else
	{
		g_free = (G_FREE)GetProcAddress(hMono, "g_free");

		if (!g_free)
			g_free = (G_FREE)GetProcAddress(hMono, "mono_unity_g_free");

		if (!g_free)
			g_free = customfreeimplementation;

		mono_free = (MONO_FREE)GetProcAddress(hMono, "mono_free");

		mono_get_root_domain = (MONO_GET_ROOT_DOMAIN)GetProcAddress(hMono, "mono_get_root_domain");
		mono_thread_attach = (MONO_THREAD_ATTACH)GetProcAddress(hMono, "mono_thread_attach");
		mono_thread_detach = (MONO_THREAD_DETACH)GetProcAddress(hMono, "mono_thread_detach");
		mono_thread_cleanup = (MONO_THREAD_CLEANUP)GetProcAddress(hMono, "mono_thread_cleanup");

		mono_object_get_class = (MONO_OBJECT_GET_CLASS)GetProcAddress(hMono, "mono_object_get_class");

		mono_domain_foreach = (MONO_DOMAIN_FOREACH)GetProcAddress(hMono, "mono_domain_foreach");
		mono_domain_set = (MONO_DOMAIN_SET)GetProcAddress(hMono, "mono_domain_set");
		mono_domain_get = (MONO_DOMAIN_GET)GetProcAddress(hMono, "mono_domain_get");
		mono_assembly_foreach = (MONO_ASSEMBLY_FOREACH)GetProcAddress(hMono, "mono_assembly_foreach");
		mono_assembly_get_image = (MONO_ASSEMBLY_GET_IMAGE)GetProcAddress(hMono, "mono_assembly_get_image");
		mono_image_get_assembly = (MONO_IMAGE_GET_ASSEMBLY)GetProcAddress(hMono, "mono_image_get_assembly");

		mono_image_get_name = (MONO_IMAGE_GET_NAME)GetProcAddress(hMono, "mono_image_get_name");
		mono_image_get_filename = (MONO_IMAGE_GET_FILENAME)GetProcAddress(hMono, "mono_image_get_filename");

		mono_image_get_table_info = (MONO_IMAGE_GET_TABLE_INFO)GetProcAddress(hMono, "mono_image_get_table_info");
		mono_image_rva_map = (MONO_IMAGE_RVA_MAP)GetProcAddress(hMono, "mono_image_rva_map");

		mono_table_info_get_rows = (MONO_TABLE_INFO_GET_ROWS)GetProcAddress(hMono, "mono_table_info_get_rows");
		mono_metadata_decode_row_col = (MONO_METADATA_DECODE_ROW_COL)GetProcAddress(hMono, "mono_metadata_decode_row_col");
		mono_metadata_string_heap = (MONO_METADATA_STRING_HEAP)GetProcAddress(hMono, "mono_metadata_string_heap");


		mono_class_get = (MONO_CLASS_GET)GetProcAddress(hMono, "mono_class_get");
		mono_class_from_typeref = (MONO_CLASS_FROM_TYPEREF)GetProcAddress(hMono, "mono_class_from_typeref");
		mono_class_name_from_token = (MONO_CLASS_NAME_FROM_TOKEN)GetProcAddress(hMono, "mono_class_name_from_token");
		mono_class_from_name_case = (MONO_CLASS_FROM_NAME_CASE)GetProcAddress(hMono, "mono_class_from_name_case");
		mono_class_from_name = (MONO_CLASS_FROM_NAME_CASE)GetProcAddress(hMono, "mono_class_from_name");
		mono_class_get_name = (MONO_CLASS_GET_NAME)GetProcAddress(hMono, "mono_class_get_name");
		mono_class_get_namespace = (MONO_CLASS_GET_NAMESPACE)GetProcAddress(hMono, "mono_class_get_namespace");
		mono_class_get_methods = (MONO_CLASS_GET_METHODS)GetProcAddress(hMono, "mono_class_get_methods");
		mono_class_get_method_from_name = (MONO_CLASS_GET_METHOD_FROM_NAME)GetProcAddress(hMono, "mono_class_get_method_from_name");
		mono_class_get_fields = (MONO_CLASS_GET_FIELDS)GetProcAddress(hMono, "mono_class_get_fields");
		mono_class_get_parent = (MONO_CLASS_GET_PARENT)GetProcAddress(hMono, "mono_class_get_parent");
		mono_class_get_image = (MONO_CLASS_GET_IMAGE)GetProcAddress(hMono, "mono_class_get_image");
		mono_class_is_generic = (MONO_CLASS_IS_GENERIC)GetProcAddress(hMono, "mono_class_is_generic");
		mono_class_vtable = (MONO_CLASS_VTABLE)GetProcAddress(hMono, "mono_class_vtable");
		mono_class_from_mono_type = (MONO_CLASS_FROM_MONO_TYPE)GetProcAddress(hMono, "mono_class_from_mono_type");
		mono_class_get_element_class = (MONO_CLASS_GET_ELEMENT_CLASS)GetProcAddress(hMono, "mono_class_get_element_class");
		mono_class_instance_size = (MONO_CLASS_INSTANCE_SIZE)GetProcAddress(hMono, "mono_class_instance_size");

		mono_class_num_fields = (MONO_CLASS_NUM_FIELDS)GetProcAddress(hMono, "mono_class_num_fields");
		mono_class_num_methods = (MONO_CLASS_NUM_METHODS)GetProcAddress(hMono, "mono_class_num_methods");


		mono_field_get_name = (MONO_FIELD_GET_NAME)GetProcAddress(hMono, "mono_field_get_name");
		mono_field_get_type = (MONO_FIELD_GET_TYPE)GetProcAddress(hMono, "mono_field_get_type");
		mono_field_get_parent = (MONO_FIELD_GET_PARENT)GetProcAddress(hMono, "mono_field_get_parent");
		mono_field_get_offset = (MONO_FIELD_GET_OFFSET)GetProcAddress(hMono, "mono_field_get_offset");
		mono_field_get_flags = (MONO_FIELD_GET_FLAGS)GetProcAddress(hMono, "mono_field_get_flags");

		mono_type_get_name = (MONO_TYPE_GET_NAME)GetProcAddress(hMono, "mono_type_get_name");
		mono_type_get_type = (MONO_TYPE_GET_TYPE)GetProcAddress(hMono, "mono_type_get_type");
		mono_type_get_name_full = (MONO_TYPE_GET_NAME_FULL)GetProcAddress(hMono, "mono_type_get_name_full");

		mono_method_get_name = (MONO_METHOD_GET_NAME)GetProcAddress(hMono, "mono_method_get_name");
		mono_method_get_class = (MONO_METHOD_GET_CLASS)GetProcAddress(hMono, "mono_method_get_class");
		mono_method_get_header = (MONO_METHOD_GET_HEADER)GetProcAddress(hMono, "mono_method_get_header");
		mono_method_get_flags = (MONO_METHOD_GET_FLAGS)GetProcAddress(hMono, "mono_method_get_flags");
		mono_method_signature = (MONO_METHOD_SIG)GetProcAddress(hMono, "mono_method_signature");
		mono_method_get_param_names = (MONO_METHOD_GET_PARAM_NAMES)GetProcAddress(hMono, "mono_method_get_param_names");



		mono_signature_get_desc = (MONO_SIGNATURE_GET_DESC)GetProcAddress(hMono, "mono_signature_get_desc");
		mono_signature_get_params = (MONO_SIGNATURE_GET_PARAMS)GetProcAddress(hMono, "mono_signature_get_params");
		mono_signature_get_param_count = (MONO_SIGNATURE_GET_PARAM_COUNT)GetProcAddress(hMono, "mono_signature_get_param_count");
		mono_signature_get_return_type = (MONO_SIGNATURE_GET_RETURN_TYPE)GetProcAddress(hMono, "mono_signature_get_return_type");



		mono_compile_method = (MONO_COMPILE_METHOD)GetProcAddress(hMono, "mono_compile_method");
		mono_free_method = (MONO_FREE_METHOD)GetProcAddress(hMono, "mono_free_method");
		mono_jit_info_table_find = (MONO_JIT_INFO_TABLE_FIND)GetProcAddress(hMono, "mono_jit_info_table_find");
		mono_jit_info_get_method = (MONO_JIT_INFO_GET_METHOD)GetProcAddress(hMono, "mono_jit_info_get_method");
		mono_jit_info_get_code_start = (MONO_JIT_INFO_GET_CODE_START)GetProcAddress(hMono, "mono_jit_info_get_code_start");
		mono_jit_info_get_code_size = (MONO_JIT_INFO_GET_CODE_SIZE)GetProcAddress(hMono, "mono_jit_info_get_code_size");
		mono_jit_exec = (MONO_JIT_EXEC)GetProcAddress(hMono, "mono_jit_exec");

		mono_method_header_get_code = (MONO_METHOD_HEADER_GET_CODE)GetProcAddress(hMono, "mono_method_header_get_code");
		mono_disasm_code = (MONO_DISASM_CODE)GetProcAddress(hMono, "mono_disasm_code");

		mono_vtable_get_static_field_data = (MONO_VTABLE_GET_STATIC_FIELD_DATA)GetProcAddress(hMono, "mono_vtable_get_static_field_data");

		mono_method_desc_new = (MONO_METHOD_DESC_NEW)GetProcAddress(hMono, "mono_method_desc_new");;
		mono_method_desc_from_method = (MONO_METHOD_DESC_FROM_METHOD)GetProcAddress(hMono, "mono_method_desc_from_method");;
		mono_method_desc_free = (MONO_METHOD_DESC_FREE)GetProcAddress(hMono, "mono_method_desc_free");;

		mono_string_new = (MONO_STRING_NEW)GetProcAddress(hMono, "mono_string_new");
		mono_string_to_utf8 = (MONO_STRING_TO_UTF8)GetProcAddress(hMono, "mono_string_to_utf8");
		mono_array_new = (MONO_ARRAY_NEW)GetProcAddress(hMono, "mono_array_new");
		mono_value_box = (MONO_VALUE_BOX)GetProcAddress(hMono, "mono_value_box");
		mono_object_unbox = (MONO_OBJECT_UNBOX)GetProcAddress(hMono, "mono_object_unbox");
		mono_object_new = (MONO_OBJECT_NEW)GetProcAddress(hMono, "mono_object_new");

		mono_class_get_type = (MONO_CLASS_GET_TYPE)GetProcAddress(hMono, "mono_class_get_type");
		mono_class_get_nesting_type = (MONO_CLASS_GET_NESTING_TYPE)GetProcAddress(hMono, "mono_class_get_nesting_type");

		mono_method_desc_search_in_image = (MONO_METHOD_DESC_SEARCH_IN_IMAGE)GetProcAddress(hMono, "mono_method_desc_search_in_image");
		mono_runtime_invoke = (MONO_RUNTIME_INVOKE)GetProcAddress(hMono, "mono_runtime_invoke");
		mono_runtime_object_init = (MONO_RUNTIME_OBJECT_INIT)GetProcAddress(hMono, "mono_runtime_object_init");


		mono_assembly_name_new = (MONO_ASSEMBLY_NAME_NEW)GetProcAddress(hMono, "mono_assembly_name_new");
		mono_assembly_loaded = (MONO_ASSEMBLY_LOADED)GetProcAddress(hMono, "mono_assembly_loaded");
		mono_assembly_open = (MONO_ASSEMBLY_OPEN)GetProcAddress(hMono, "mono_assembly_open");
		mono_image_open = (MONO_IMAGE_OPEN)GetProcAddress(hMono, "mono_image_open");

		mono_field_static_get_value = (MONO_FIELD_STATIC_GET_VALUE)GetProcAddress(hMono, "mono_field_static_get_value");
		mono_field_static_set_value = (MONO_FIELD_STATIC_SET_VALUE)GetProcAddress(hMono, "mono_field_static_set_value");

		mono_runtime_is_shutting_down = (MONO_RUNTIME_IS_SHUTTING_DOWN)GetProcAddress(hMono, "mono_runtime_is_shutting_down");
		domain = mono_get_root_domain();

		mono_selfthread = NULL;

		if (mono_thread_attach)
		{

			if (il2cpp)
				mono_selfthread = mono_thread_attach(mono_domain_get());
			else
			{
				if (mono_get_root_domain)
				{
					void* domain = mono_get_root_domain();
					mono_selfthread = mono_thread_attach(domain);
				}
			}
		}
	}
		
}