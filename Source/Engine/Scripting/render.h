#pragma once
#include "Resource/texture.h"
#include "Core/GL/render_pass.h"
#include "Core/GL/frame_buffer.h"
#include "Core/Memory/shared_ptr.h"
#include "Core/Container/optional.h"
#include "Core/GL/context.h"
#include "Engine/Scripting/ec.h"
#include "Engine/Scripting/type.h"
namespace glex::py
{
	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			Functions.
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	inline uint32_t NumRenderAheadFrames()
	{
		return Context::CurrentRenderSettings().renderAheadCount;
	}

	inline uint32_t CurrentFrame()
	{
		return Context::CurrentFrame();
	}

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			Render resources.
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	struct Texture
	{
		SharedPtr<glex::Texture> texture;

		PyRetVal<void> Create(uint32_t format, uint32_t usage, uint32_t aspect, uint32_t width, uint32_t height, uint32_t samples)
		{
			texture = MakeShared<glex::Texture>(static_cast<gl::ImageFormat>(format), static_cast<gl::ImageUsage>(usage),
				static_cast<gl::ImageAspect>(aspect), width, height, samples);
			if (*texture == nullptr)
			{
				texture = nullptr;
				return PyStatus::RaiseException;
			}
			return PyStatus::Success;
		}

		void Destroy()
		{
			texture = nullptr;
		}

		uint32_t Width() const { return texture->Width(); }
		uint32_t Height() const { return texture->Height(); }
	};

	class RenderPass;
	struct FrameBuffer
	{
		gl::FrameBuffer frameBuffer;
		uint32_t width, height;

		PyRetVal<void> Create(Type<RenderPass>* renderPass, PyObject* attachmentList, uint32_t width, uint32_t height);

		void Destroy()
		{
			Context::PendingDelete([=]() { frameBuffer.Destroy(); });
		}
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			Render pass.
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	struct AttachmentInfo
	{
		gl::AttachmentInfo m_info;
		void Create(uint32_t format, uint32_t beginState, uint32_t inUseState, uint32_t endState, uint32_t usage, uint32_t stencilUsage, uint32_t msaaLevel)
		{
			m_info.format = static_cast<gl::ImageFormat>(format);
			m_info.beginState = static_cast<gl::ImageState>(beginState);
			m_info.inUseState = static_cast<gl::ImageState>(inUseState);
			m_info.endState = static_cast<gl::ImageState>(endState);
			m_info.usage = static_cast<gl::AttachmentUsage>(usage);
			m_info.stencilUsage = static_cast<gl::AttachmentUsage>(stencilUsage);
			m_info.msaaLevel = msaaLevel;
		}
	};

	struct SubpassInfo
	{
		Vector<uint32_t> inputAttachments;
		Vector<uint32_t> outputColorAttachments;
		uint32_t outputDepthStencilAttachment;
		Vector<uint32_t> preserveAttachments;
		void Create(Vector<uint32_t> inputs, Vector<uint32_t> outputColors, uint32_t outputDepthStencil, Vector<uint32_t> preserves)
		{
			inputAttachments = std::move(inputs);
			outputColorAttachments = std::move(outputColors);
			outputDepthStencilAttachment = outputDepthStencil;
			preserveAttachments = std::move(preserves);
		}
	};

	struct DependencyInfo
	{
		gl::DependencyInfo m_info;
		void Create(uint32_t source, uint32_t dest, uint64_t sourceStage, uint64_t destStage, uint64_t sourceAccess, uint64_t destAccess)
		{
			m_info.source = source;
			m_info.dest = dest;
			m_info.sourceStage = static_cast<gl::PipelineStage>(sourceStage);
			m_info.destStage = static_cast<gl::PipelineStage>(destStage);
			m_info.sourceAccess = static_cast<gl::Access>(sourceAccess);
			m_info.destAccess = static_cast<gl::Access>(destAccess);
		}
	};

	struct RenderPass
	{
		gl::RenderPass renderPass;

		PyRetVal<void> Create(PyObject* attachments, PyObject* subpasses, PyObject* dependencies)
		{
			auto numAttachments = PyList_Size(attachments);
			auto numSubpasses = PyList_Size(subpasses);
			auto numDependencies = PyList_Size(dependencies);
			if (numAttachments == -1 || numSubpasses == -1 || numDependencies == -1)
				return PyStatus::RelayException;
			Vector<gl::AttachmentInfo> attachmentInfo(numAttachments);
			Vector<gl::SubpassInfo> subpassInfo(numSubpasses);
			Vector<gl::DependencyInfo> dependencyInfo(numDependencies);
			for (uint32_t i = 0; i < numAttachments; i++)
			{
				Type<AttachmentInfo>* attachment = Type<AttachmentInfo>::Cast(PyList_GetItem(attachments, i));
				if (attachment == nullptr)
					return PyStatus::RaiseException;
				attachmentInfo[i] = attachment->Get().m_info;
			}
			for (uint32_t i = 0; i < numSubpasses; i++)
			{
				Type<SubpassInfo>* subpass = Type<SubpassInfo>::Cast(PyList_GetItem(subpasses, i));
				if (subpass == nullptr)
					return PyStatus::RaiseException;
				subpassInfo[i] = { subpass->Get().inputAttachments, subpass->Get().outputColorAttachments, subpass->Get().outputDepthStencilAttachment, subpass->Get().preserveAttachments };
			}
			for (uint32_t i = 0; i < numDependencies; i++)
			{
				Type<DependencyInfo>* dependency = Type<DependencyInfo>::Cast(PyList_GetItem(dependencies, i));
				if (dependency == nullptr)
					return PyStatus::RaiseException;
				dependencyInfo[i] = dependency->Get().m_info;
			}
			return renderPass.Create(attachmentInfo, subpassInfo, dependencyInfo) ? PyStatus::Success : PyStatus::RaiseException;
		}

		void Destroy()
		{
			Context::PendingDelete([=]() { renderPass.Destroy(); });
		}

		PyRetVal<void> Begin(Type<FrameBuffer>* frameBuffer, PyObject* clearValues)
		{
			// Heavy 'generic' parameter parsing. This is why script is slow.
			auto size = PyList_Size(clearValues);
			if (size == -1)
				return PyStatus::RelayException;
			Vector<gl::ClearValue> cvs(size);
			for (uint32_t i = 0; i < size; i++)
			{
				PyObject* item = PyList_GetItem(clearValues, i);
				if (item->ob_type == &PyLong_Type)
				{
					if (!PyParse(item, &cvs[i].stencil))
						return PyStatus::RelayException;
				}
				else if (item->ob_type == &PyFloat_Type)
				{
					if (!PyParse(item, &cvs[i].depth))
						return PyStatus::RelayException;
				}
				else
				{
					auto tupleSize = PyTuple_Size(item);
					if (tupleSize == -1)
						return PyStatus::RelayException;
					if (tupleSize == 0)
						return PyStatus::RaiseException;
					PyObject* tupItem = PyTuple_GetItem(item, 0);
					if (tupItem->ob_type == &PyFloat_Type)
					{
						if (!PyParse(tupItem, &cvs[i].fColor[0]))
							return PyStatus::RelayException;
						for (uint32_t j = 1; j < tupleSize; j++)
						{
							tupItem = PyTuple_GetItem(item, j);
							if (!PyParse(tupItem, &cvs[i].fColor[j]))
								return PyStatus::RelayException;
						}
					}
					else if (tupItem->ob_type == &PyLong_Type)
					{
						if (!PyParse(tupItem, &cvs[i].uColor[0]))
							return PyStatus::RelayException;
						for (uint32_t j = 1; j < tupleSize; j++)
						{
							tupItem = PyTuple_GetItem(item, j);
							if (!PyParse(tupItem, &cvs[i].uColor[j]))
								return PyStatus::RelayException;
						}
					}
					else
						return PyStatus::RaiseException;
				}
			}

			gl::CommandBuffer commandBuffer = Context::GetCommandBuffer();
			FrameBuffer& fb = frameBuffer->Get();
			commandBuffer.BeginRenderPass(renderPass, fb.frameBuffer, glm::uvec2(fb.width, fb.height), cvs);
			return PyStatus::Success;
		}

		void NextSubpass()
		{
			gl::CommandBuffer commandBuffer = Context::GetCommandBuffer();
			commandBuffer.NextSubpass();
		}

		void End()
		{
			gl::CommandBuffer commandBuffer = Context::GetCommandBuffer();
			commandBuffer.EndRenderPass();
		}
	};
}