#include "Engine/Scripting/scripting.h"
#include "Engine/Scripting/helper.h"
#include "Engine/Scripting/api.h"

using namespace glex;
using namespace glex::py;

void Scripting::Startup(ScriptStartupInfo const& info)
{
	if (info.libraries.size() != 0)
	{
		s_modules.reserve(info.libraries.size());
		static uint32_t i = 0;
		for (LibraryInfo const& lib : info.libraries)
		{
			ModuleMetadata& data = s_modules.emplace_back();
			data.header = { PyModuleDef_HEAD_INIT, lib.Name(), nullptr, -1 };
			py::MethodList& methodList = lib.GetMethodList();
			LibraryInfo::ClassList& classes = lib.GetClassList();
			if (!methodList.empty())
			{
				data.methodList = std::move(methodList);
				data.methodList.emplace_back(nullptr, nullptr, 0, nullptr);
				data.header.m_methods = data.methodList.data();
			}
			data.classes = std::move(classes);

			// This function was called way after Py_Initialize
			// so that LibraryInfo went out of scope.
			// Fuck any one who wrote this shit.
			if (PyImport_AppendInittab(lib.Name(), []()
			{
				ModuleMetadata& data = s_modules[i++];
				Logger::Trace("Loading library %s.", data.header.m_name);
				PyObject* module = PyModule_Create(&data.header);
				GLEX_ASSERT_MSG(module != nullptr, "Cannot load module.") {}
				for (LibraryInfo::Class& type : data.classes)
					PyModule_AddObject(module, type.name, reinterpret_cast<PyObject*>(type.desc(type.name)));
				// No use anymore. Still 32 bytes wasted.
				data.classes.clear();
				data.classes.shrink_to_fit();
				return module;
			}) == -1)
				Logger::Fatal("Cannot export module %s.", lib.Name());
		}
	}

	Logger::Trace("Starting Python interpreter.");
	Py_Initialize();

	if (info.scriptFolder != nullptr)
		PushPath(info.scriptFolder);
}

void Scripting::Shutdown()
{
	if (s_modules.empty())
		return;
	// py::CleanCache();
	Py_Finalize();
#if GLEX_REPORT_MEMORY_LEAKS
	LibraryInfo::ForceFreeMemory();
	s_modules.clear();
	s_modules.shrink_to_fit();
#endif
}

LibraryInfo Scripting::MakeStandardLibrary()
{
	LibraryInfo lib("glex", 0, 0);

	lib.Register<py::RenderWidth>("render_width");
	lib.Register<py::RenderHeight>("render_height");
	lib.Register<py::WindowWidth>("window_width");
	lib.Register<py::WindowHeight>("window_height");
	lib.Register<py::MouseX>("mouse_x");
	lib.Register<py::MouseY>("mouse_y");
	lib.Register<py::MouseDeltaX>("mouse_delta_x");
	lib.Register<py::MouseDeltaY>("mouse_delta_y");
	lib.Register<py::Pressed>("pressed");
	lib.Register<py::Released>("released");
	lib.Register<py::Pressing>("pressing");
	lib.Register<py::Time>("time");
	lib.Register<py::DeltaTime>("delta_time");

	Type<py::Image>::RegisterInit<&py::Image::Create>();
	Type<py::Image>::RegisterMethod<&py::Image::Destroy>("destroy");
	Type<py::Image>::RegisterMethod<&py::Image::Resize>("resize");
	lib.Register<py::Image>("Image");

	Type<py::ImageView>::RegisterInit<&py::ImageView::Create>();
	Type<py::ImageView>::RegisterMethod<&py::ImageView::Destroy>("destroy");
	Type<py::ImageView>::RegisterMethod<&py::ImageView::Recreate>("recreate");
	lib.Register<py::ImageView>("ImageView");

	Type<py::RenderPassBuilder>::RegisterMethod<&py::RenderPassBuilder::PushSubpass>("push_subpass");
	Type<py::RenderPassBuilder>::RegisterMethod<&py::RenderPassBuilder::Read>("read");
	Type<py::RenderPassBuilder>::RegisterMethod<&py::RenderPassBuilder::Write>("write");
	Type<py::RenderPassBuilder>::RegisterMethod<&py::RenderPassBuilder::Clear>("clear");
	Type<py::RenderPassBuilder>::RegisterMethod<&py::RenderPassBuilder::Output>("output");
	lib.Register<py::RenderPassBuilder>("RenderPassBuilder");

	lib.Register<py::RenderList>("RenderList");

	Type<py::RenderPass>::RegisterInit<&py::RenderPass::Create>();
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::Destroy>("destroy");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::BeginRenderPassDefinition>("begin_renderpass_definition");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::EndRenderPassDefinition>("end_renderpass_definition");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::BeginRenderPass>("begin_renderpass");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::NextSubpass>("next_subpass");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::EndRenderPass>("end_renderpass");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::RenderMeshList>("render_meshlist");
	Type<py::RenderPass>::RegisterMethod<&py::RenderPass::Recreate>("recreate");
	Type<py::RenderPass>::EnableInheritance();
	lib.Register<py::RenderPass>("RenderPass");

	Type<py::MaterialDomainDefinition>::RegisterInit<&py::MaterialDomainDefinition::Create>();
	lib.Register<py::MaterialDomainDefinition>("MaterialDomainDefinition");

	return lib;
}

bool Scripting::PushPath(char const* folder)
{
	PyObject* path = PySys_GetObject("path");
	PyObject* pathString = PyUnicode_FromString(folder);
	bool result = PyList_Append(path, pathString) == 0;
	Py_DECREF(pathString);
	return result;
}

void Scripting::PopPath()
{
	PyObject* path = PySys_GetObject("path");
	Py_DECREF(py::PyList_Pop(path));
}