#pragma once
#include "Core/Container/basic.h"
#include "Core/Container/sequence.h"
#include "Engine/Scripting/lib.h"

namespace glex
{
	struct ScriptStartupInfo
	{
		char const* scriptFolder = nullptr;
		Vector<LibraryInfo> libraries;
	};

	class Scripting : private StaticClass
	{
	private:
		friend class LibraryInfo;

		struct ModuleMetadata
		{
			PyModuleDef header;
			Vector<PyMethodDef> methodList;
			LibraryInfo::ClassList classes;	// Wasted.
		};
		inline static Vector<ModuleMetadata> s_modules;

	public:
#ifdef GLEX_INTERNAL
		static void Startup(ScriptStartupInfo const& info);
		static void Shutdown();
#endif

		static LibraryInfo MakeStandardLibrary();
		static bool PushPath(char const* path);
		static void PopPath();
	};
}