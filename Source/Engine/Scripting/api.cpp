#include "Engine/Scripting/api.h"

using namespace glex;
using namespace glex::py;

glex::PyRetVal<void> py::Image::Create(gl::ImageFormat format, gl::ImageUsage usages, glm::uvec3 size, uint32_t samples, bool usedAsCube)
{
	m_image = glex::MakeShared<glex::Image>(format, usages, size, samples, usedAsCube);
	if (!m_image->IsValid())
		return { glex::PyStatus::RaiseException };
	return { glex::PyStatus::Success };
}

void py::Image::Destroy()
{
	m_image = nullptr;
}

glex::PyRetVal<void> py::ImageView::Create(glex::Type<py::Image>* image, uint32_t layerIndex, uint32_t numLayers, gl::ImageType type, gl::ImageAspect aspect)
{
	m_imageView = glex::MakeShared<glex::ImageView>((*image)->m_image, layerIndex, numLayers, type, aspect);
	if (!m_imageView->IsValid())
		return { glex::PyStatus::RaiseException };
	return { glex::PyStatus::Success };
}

void py::ImageView::Destroy()
{
	m_imageView = nullptr;
}

PyRetVal<void> py::RenderPass::BeginRenderPass(PyObject* clearValueList)
{
	Py_ssize_t size = PyList_Size(clearValueList);
	if (size == -1)
		return { PyStatus::RaiseException };
	InlineVector<gl::ClearValue, 4> clearValues(size); // Use stack allocator to further optimize memory allocation.
	for (uint32_t i = 0; i < size; i++)
	{
		PyObject* item = PyList_GetItem(clearValueList, i);
		if (PyTuple_Check(item))
		{
			if (PyTuple_Size(item) != 4)
				return { PyStatus::RaiseException };
			PyObject* e0 = PyTuple_GetItem(item, 0);
			if (PyFloat_Check(e0))
			{
				PyObject* e1 = PyTuple_GetItem(item, 1);
				PyObject* e2 = PyTuple_GetItem(item, 2);
				PyObject* e3 = PyTuple_GetItem(item, 3);
				if (!PyFloat_Check(e1) || !PyFloat_Check(e2) || !PyFloat_Check(e3))
					return { PyStatus::RaiseException };
				glm::vec4& color = clearValues[i].fColor;
				color.r = PyFloat_AsDouble(e0);
				color.g = PyFloat_AsDouble(e1);
				color.b = PyFloat_AsDouble(e2);
				color.a = PyFloat_AsDouble(e3);
			}
			else if (PyLong_Check(e0))
			{
				PyObject* e1 = PyTuple_GetItem(item, 1);
				PyObject* e2 = PyTuple_GetItem(item, 2);
				PyObject* e3 = PyTuple_GetItem(item, 3);
				if (!PyLong_Check(e1) || !PyLong_Check(e2) || !PyLong_Check(e3))
					return { PyStatus::RaiseException };
				glm::uvec4& color = clearValues[i].uColor;
				color.r = PyLong_AsUnsignedLong(e0);
				color.g = PyLong_AsUnsignedLong(e1);
				color.b = PyLong_AsUnsignedLong(e2);
				color.a = PyLong_AsUnsignedLong(e3);
			}
			else
				return { PyStatus::RaiseException };
		}
		else if (PyFloat_Check(item))
			clearValues[i].depth = PyFloat_AsDouble(item);
		else if (PyLong_Check(item))
			clearValues[i].stencil = PyLong_AsUnsignedLong(item);
		else
			return { PyStatus::RaiseException };
	}
	m_renderPass->BeginRenderPass({ clearValues.begin(), clearValues.size() });
	return { PyStatus::Success };
}

void py::RenderPass::RenderMeshList(Type<RenderList>* list, uint32_t materialDomain)
{
	for (auto [mr, tr] : (*list)->m_meshList)
	{
		SharedPtr<MaterialInstance> const& mat = mr.GetMaterial(materialDomain);
		glm::mat4 const& modelMat = tr.GetModelMat();
		m_renderPass->BindMaterial(mat);
		m_renderPass->BindObjectData(&modelMat, sizeof(glm::mat4));
		m_renderPass->DrawMesh(mr.GetMesh());
	}
}

void py::MaterialDomainDefinition::Create(PyKeywordParameters kwds, Type<RenderPass>* renderPass, uint32_t subpass)
{
	m_renderPass = renderPass;
	m_subpass = subpass;
	for (auto [key, value] : kwds)
	{
		if (strcmp(key, "cull_mode") == 0)
		{
			gl::CullMode cullMode;
			PyParse(value, &cullMode);
			m_metaMaterial.cullMode = cullMode;
		}
		else if (strcmp(key, "depth_test") == 0)
		{
			bool enabled;
			PyParse(value, &enabled);
			m_metaMaterial.depthTest = enabled;
		}
		else if (strcmp(key, "depth_write") == 0)
		{
			bool enabled;
			PyParse(value, &enabled);
			m_metaMaterial.depthWrite = enabled;
		}
		else if (strcmp(key, "blend") == 0)
		{
			bool enabled;
			PyParse(value, &enabled);
			m_metaMaterial.blend = enabled;
		}
		else if (strcmp(key, "wireframe") == 0)
		{
			bool enabled;
			PyParse(value, &enabled);
			m_metaMaterial.wireframe = enabled;
		}
		else if (strcmp(key, "src_color_factor") == 0)
		{
			gl::BlendFactor factor;
			PyParse(value, &factor);
			m_metaMaterial.sourceColorFactor = factor;
		}
		else if (strcmp(key, "src_alpha_factor") == 0)
		{
			gl::BlendFactor factor;
			PyParse(value, &factor);
			m_metaMaterial.sourceAlphaFactor = factor;
		}
		else if (strcmp(key, "dst_color_factor") == 0)
		{
			gl::BlendFactor factor;
			PyParse(value, &factor);
			m_metaMaterial.destColorFactor = factor;
		}
		else if (strcmp(key, "dst_alpha_factor") == 0)
		{
			gl::BlendFactor factor;
			PyParse(value, &factor);
			m_metaMaterial.destAlphaFactor = factor;
		}
		else if (strcmp(key, "color_op") == 0)
		{
			gl::BlendOperation op;
			PyParse(value, &op);
			m_metaMaterial.colorBlendOperation = op;
		}
		else if (strcmp(key, "alpha_op") == 0)
		{
			gl::BlendOperation op;
			PyParse(value, &op);
			m_metaMaterial.alphaBlendOperation = op;
		}
	}
}