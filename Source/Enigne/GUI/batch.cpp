#include "Engine/GUI/batch.h"
#include "Engine/Renderer/renderer.h"
#include "Core/GL/context.h"
#include "Core/Platform/input.h"
#include "Core/Platform/window.h"
#include "Core/Platform/time.h"
#include "game.h"
#include <EASTL/sort.h>

using namespace glex::ui;
using namespace glex::gl;

bool BatchRenderer::Startup(uint32_t initialQuadBudget)
{
	PhysicalDevice const& cardInfo = Context::DeviceInfo();
	uint32_t textureCount = Min(cardInfo.MaxSamplerCount(), cardInfo.MaxTextureCount(), Limits::NUM_BATCH_TEXTURES);
	gl::DescriptorBinding textureBinding;
	textureBinding.type = gl::DescriptorType::CombinedImageSampler;
	textureBinding.bindingPoint = 0;
	textureBinding.arraySize = textureCount;
	textureBinding.shaderStage = gl::ShaderStage::Fragment;
	s_textureSetLayout = Renderer::GetDescriptorLayoutCache().GetDescriptorSetLayout(&textureBinding);
	if (s_textureSetLayout.GetHandle() == VK_NULL_HANDLE)
	{
		Logger::Error("Cannot create descriptor layout for quads.");
		return false;
	}
	s_textureDescriptorAllocator.Emplace(std::initializer_list<std::pair<gl::DescriptorType, uint32_t>> { { gl::DescriptorType::CombinedImageSampler, textureCount } }, 60);
	s_vertexBuffer.Emplace(gl::BufferUsage::Vertex, initialQuadBudget * 4 * sizeof(QuadVertex), true);
	if (!s_vertexBuffer->IsValid())
	{
		Logger::Error("Cannot create vertex buffer for quads.");
		Renderer::GetDescriptorLayoutCache().FreeDescriptorSetLayout(s_textureSetLayout);
		s_vertexBuffer.Destroy();
		return false;
	}
	s_indexBuffer.Emplace(gl::BufferUsage::Index, initialQuadBudget * 24, true);
	if (!s_indexBuffer->IsValid())
	{
		Logger::Error("Cannot create index buffer for quads.");
		Renderer::GetDescriptorLayoutCache().FreeDescriptorSetLayout(s_textureSetLayout);
		s_vertexBuffer.Destroy();
		s_indexBuffer.Destroy();
		return false;
	}
	s_vertexMap = reinterpret_cast<QuadVertex*>(s_vertexBuffer->Map());
	s_indexMap = reinterpret_cast<uint32_t*>(s_indexBuffer->Map());
	s_textures.resize(textureCount);
	s_doubleClickTime = Platform::GetDoubleClickTime();
	return true;
}

void BatchRenderer::Shutdown()
{
	s_vertexBuffer.Destroy();
	s_indexBuffer.Destroy();
	s_textureDescriptorAllocator.Destroy();
	Renderer::GetDescriptorLayoutCache().FreeDescriptorSetLayout(s_textureSetLayout);
#if GLEX_REPORT_MEMORY_LEAKS
	s_textures.clear();
	s_textures.shrink_to_fit();
	s_drawList.shrink_to_fit();
	s_descriptors.clear();
	s_descriptors.shrink_to_fit();
#endif
}

void BatchRenderer::Tick()
{
	// Reset those stuff, or they'll be drawn thousands of times.
	s_textureDescriptorAllocator->Reset();
	s_emptySet = VK_NULL_HANDLE;
	s_vertexBegin = 0;
	s_vertexEnd = 0;
	s_indexBegin = 0;
	s_indexEnd = 0;
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Draw primitives.
————————————————————————————————————————————————————————————————————————————————————————————————————*/

void BatchRenderer::Flush()
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	WeakPtr<MaterialInstance> currentMaterial = Renderer::GetCurrentMaterialInstance();
	if (s_textures[0] != nullptr)
	{
		gl::DescriptorSet descriptorSet = s_textureDescriptorAllocator->AllocateDescriptorSet(s_textureSetLayout);
		if (descriptorSet.GetHandle() == VK_NULL_HANDLE)
		{
			Logger::Error("Cannot allocate descriptor for quads. Rendering failed.");
			s_vertexBegin = s_vertexEnd;
			s_indexBegin = s_indexEnd;
			return;
		}

		s_descriptors.resize(s_numTextures);
		for (uint32_t i = 0; i < s_numTextures; i++)
		{
			s_descriptors[i].image.imageView = s_textures[i]->GetImageView().GetImageViewObject();
			s_descriptors[i].image.imageLayout = gl::ImageLayout::ShaderRead;
			s_descriptors[i].sampler.sampler = s_textures[i]->GetSampler();
			s_textures[i] = nullptr;
		}

		gl::Descriptor descriptor;
		descriptor.bindingPoint = 0;
		descriptor.arrayIndex = 0;
		descriptor.type = gl::DescriptorType::CombinedImageSampler;
		descriptor.imageSamplers = s_descriptors;
		descriptorSet.BindDescriptors(&descriptor);

		commandBuffer.BindDescriptorSet(currentMaterial->GetShader()->GetDescriptorLayout(), Renderer::OBJECT_DESCRIPTOR_SET, descriptorSet);
	}
	else
	{
		gl::DescriptorSetLayout set = currentMaterial->GetShader()->GetObjectLayout();
		if (set.GetHandle() != VK_NULL_HANDLE)
		{
			if (s_emptySet.GetHandle() == VK_NULL_HANDLE)
			{
				s_emptySet = s_textureDescriptorAllocator->AllocateDescriptorSet(set);
				if (s_emptySet.GetHandle() == VK_NULL_HANDLE)
				{
					Logger::Error("Cannot allocate descriptor for quads. Rendering failed.");
					s_vertexBegin = s_vertexEnd;
					s_indexBegin = s_indexEnd;
					return;
				}
			}
			commandBuffer.BindDescriptorSet(currentMaterial->GetShader()->GetDescriptorLayout(), Renderer::OBJECT_DESCRIPTOR_SET, s_emptySet);
		}
	}
	s_vertexBuffer->GetMemoryObject().Flush(s_vertexBegin * sizeof(QuadVertex), (s_vertexEnd - s_vertexBegin) * sizeof(QuadVertex));
	s_indexBuffer->GetMemoryObject().Flush(s_indexBegin * 4, (s_indexEnd - s_indexBegin) * 4);
	commandBuffer.DrawIndexed(s_indexEnd - s_indexBegin, 1, 0, s_indexBegin);
	s_vertexBegin = s_vertexEnd;
	s_indexBegin = s_indexEnd;
}

void BatchRenderer::UseMaterial(WeakPtr<MaterialInstance> material)
{
	s_availableTextures = s_textures.size() - material->GetShader()->NumTextures();
	if (s_availableTextures < 16)
		Logger::Warn("This material contains too many textures. Only %d left for batching.", s_availableTextures);
	material->Bind();
	s_currentBatchMaterial = material;
}

void BatchRenderer::PushMeshData(SequenceView<QuadVertex const> vertexData, SequenceView<uint32_t const> indexData, WeakPtr<Texture> texture)
{
	// Allocate texture ID.
	uint32_t texID = UINT_MAX;
	if (texture != nullptr)
	{
		for (uint32_t i = 0; i < s_availableTextures; i++)
		{
			if (s_textures[i] == texture)
			{
				texID = i;
				goto TEXID_ALLOCATED;
			}
			else if (s_textures[i] == nullptr)
			{
				s_textures[i] = texture;
				texID = i;
				s_numTextures = i + 1;
				goto TEXID_ALLOCATED;
			}
		}
		Flush();
		s_textures[0] = texture;
		texID = 0;
		s_numTextures = 1;
	TEXID_ALLOCATED:;
	}

	// Check buffer memory size.
	if ((s_vertexEnd + vertexData.Size()) * sizeof(QuadVertex) > s_vertexBuffer->Size())
	{
		if (s_vertexBegin != s_vertexEnd)
			Flush();
		// 1.5x increase. No need to unmap.
		uint32_t newVertexBufferSize = s_vertexBuffer->Size() + (s_vertexBuffer->Size() >> 1);
		if (!s_vertexBuffer->Resize(newVertexBufferSize))
		{
			Logger::Error("Cannot draw quad batch anymore. VRAM ran out.");
			return;
		}
		s_vertexMap = reinterpret_cast<QuadVertex*>(s_vertexBuffer->Map());
		s_vertexBegin = 0;
		s_vertexEnd = 0;
		gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
		commandBuffer.BindVertexBuffer(s_vertexBuffer->GetBufferObject(), 0);
	}
	if ((s_indexEnd + indexData.Size()) * 4 > s_indexBuffer->Size())
	{
		if (s_indexBegin != s_indexEnd)
			Flush();
		uint32_t newIndexBufferSize = s_indexBuffer->Size() + (s_indexBuffer->Size() >> 1);
		if (!s_indexBuffer->Resize(newIndexBufferSize))
		{
			Logger::Error("Cannot draw quad batch anymore. VRAM ran out.");
			return;
		}
		s_indexMap = reinterpret_cast<uint32_t*>(s_indexBuffer->Map());
		s_indexBegin = 0;
		s_indexEnd = 0;
		gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
		commandBuffer.BindIndexBuffer(s_indexBuffer->GetBufferObject(), 0);
	}

	for (uint32_t index : indexData)
	{
		s_indexMap[s_indexEnd++] = s_vertexEnd + index;
	}
	for (QuadVertex const& vertex : vertexData)
	{
		QuadVertex copy = vertex;
		copy.texID = texID;
		s_vertexMap[s_vertexEnd++] = copy;
	}
}

void BatchRenderer::DrawQuad(glm::vec4 const& border, WeakPtr<Texture> texture, glm::vec4 const& uv, glm::vec4 const& color)
{
	QuadVertex vertexData[4];
	vertexData[0].pos = glm::vec2(border.x, border.y);
	vertexData[0].uv = glm::vec2(uv.x, uv.y);
	vertexData[0].color = color;
	vertexData[1].pos = glm::vec2(border.x, border.y + border.w);
	vertexData[1].uv = glm::vec2(uv.x, uv.y - uv.w);
	vertexData[1].color = color;
	vertexData[2].pos = glm::vec2(border.x + border.z, border.y + border.w);
	vertexData[2].uv = glm::vec2(uv.x + uv.z, uv.y - uv.w);
	vertexData[2].color = color;
	vertexData[3].pos = glm::vec2(border.x + border.z, border.y);
	vertexData[3].uv = glm::vec2(uv.x + uv.z, uv.y);
	vertexData[3].color = color;
	PushMeshData(ArrayOf(vertexData), { 0, 1, 2, 0, 2, 3 }, texture);
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		BATCHING ALGORITHM
		I would say, it's really hard. X﹏X
————————————————————————————————————————————————————————————————————————————————————————————————————*/
uint32_t BatchRenderer::AddChildrenToDrawList(WeakPtr<Control> control)
{
	uint32_t childBegin = s_drawList.size();
	uint32_t numChildren = control->NumChildren();
	if (numChildren == 0)
		return UINT_MAX;

	// First let's just add all of its children to the list.
	uint32_t childEnd = childBegin + numChildren;
	s_drawList.reserve(childEnd);
	for (uint32_t i = 0; i < numChildren; i++)
		s_drawList.emplace_back(control->GetChild(i), 0, 0, 0);

	// Then sort them.
	eastl::sort(s_drawList.begin() + childBegin, s_drawList.begin() + childEnd, [](DrawElement const& lhs, DrawElement const& rhs)
	{
		// Maybe we should squeeze these two field into a single uint64_t.
		return lhs.control->GetZOrder() < rhs.control->GetZOrder() || lhs.control->GetZOrder() == rhs.control->GetZOrder() && lhs.control->GetMaterial() < rhs.control->GetMaterial();
	});

	// After that, build link list for potential batch group.
	uint32_t nextGroup = UINT_MAX;
	DrawElement& last = s_drawList[childEnd - 1];
	int32_t currentZOrder = last.control->GetZOrder();
	WeakPtr<MaterialInstance> currentMaterial = last.control->GetMaterial();
	for (int32_t i = childEnd - 2; i >= static_cast<int32_t>(childBegin); i--)
	{
		DrawElement& current = s_drawList[i];
		if (current.control->GetMaterial() != currentMaterial || current.control->GetZOrder() != currentZOrder)
		{
			// We only care about the first element in the group.
			s_drawList[i + 1].groupSize = (nextGroup == UINT_MAX ? childEnd : nextGroup) - i - 1;
			s_drawList[i + 1].nextGroup = nextGroup;
			nextGroup = i + 1;
			currentZOrder = current.control->GetZOrder();
			currentMaterial = current.control->GetMaterial();
		}
	}
	s_drawList[childBegin].groupSize = (nextGroup == UINT_MAX ? childEnd : nextGroup) - childBegin;
	s_drawList[childBegin].nextGroup = nextGroup;

	// Return the begin index of children, which can be used to set their parent's children pointer.
	return childBegin;
}

void BatchRenderer::BuildBatch(uint32_t rootIndex, glm::vec2 parentPosition)
{
	DrawElement* root = &s_drawList[rootIndex];

	// It's zero first. If it has no more children, it's UINT_MAX. If it has children, it can't be zero.
	bool alreadyDrawn = root->childIndex != 0;

	// Layout cascade.
	glm::vec4 const& rect = root->control->GetRect();
	parentPosition += glm::vec2(rect);

	if (!alreadyDrawn)
	{
		// If the control is not visible, skip it.
		if (root->control->GetVisibility() != Visibility::Visible)
		{
			root->childIndex = UINT_MAX;
			return;
		}

		// First draw the control itself.
		// Set material.
		// Material is always compatible. If they are different, we must flush.
		WeakPtr<MaterialInstance> rootMaterial = root->control->GetMaterial();
		if (rootMaterial != nullptr && rootMaterial != s_currentBatchMaterial)
		{
			if (s_indexEnd != s_indexBegin)
				Flush();
			UseMaterial(rootMaterial);
		}

		// Dispatch mouse event.
		glm::vec2 size = glm::vec2(rect.z, rect.w);
		// If we're pressing LMB, keep the hovered control unchanged. Until we release it.
		if (Input::IsPressing(KeyCode::LMB))
		{
			if (root->control == s_hoverControlLastFrame)
			{
				s_hoverControlThisFrame = s_hoverControlLastFrame;
				s_hoverControlPos = parentPosition;
			}
		}
		else if (root->control->GetAcceptMouseEvent())
		{
			float mouseX = Input::MouseX(), mouseY = Input::MouseY();
			if (mouseX >= parentPosition.x && mouseY >= parentPosition.y && mouseX <= parentPosition.x + size.x && mouseY <= parentPosition.y + size.y)
			{
				s_hoverControlThisFrame = root->control;
				s_hoverControlPos = parentPosition;
			}
		}
		if (rootMaterial != nullptr)
			root->control->OnPaint(parentPosition, size);

		// If we haven't add its children, add them.
		uint32_t childBegin = AddChildrenToDrawList(root->control); // UINT_MAX returned when there's no children.

		//
		// ROOT IS DANGLING NOW.
		//
		root = &s_drawList[rootIndex];
		root->childIndex = childBegin;
	}

	// Loop through its children, find the batch we can draw.
	if (root->childIndex != UINT_MAX)
	{
		uint32_t groupBegin = root->childIndex;
		uint32_t previousGroup = 0;
		// No matter what, we can only draw child controls who have this specific Z order.
		int32_t zOrder = s_drawList[groupBegin].control->GetZOrder();
		// Then we loop through all the groups.
		while (groupBegin != UINT_MAX)
		{
			DrawElement& head = s_drawList[groupBegin];
			// We cannot draw the control if there are any control beneath it.
			// I thought about adding occusion test so that we can draw a control anyway if it doesn't overlapping a control
			// beneath it, but it requires getting the bounding box of all the controls beneath it, which is an O(n) operation.
			if (head.control->GetZOrder() != zOrder)
				break;
			// Then we check if its material is compatible with the batch's material.
			if (head.control->GetMaterial() == nullptr || s_currentBatchMaterial == nullptr || head.control->GetMaterial() == s_currentBatchMaterial)
			{
				// Material is compatible. We can draw these children.
				uint32_t groupEnd = groupBegin + head.groupSize;
				bool childStillAlive = false;
				for (uint32_t i = groupBegin; i < groupEnd; i++)
				{
					BuildBatch(i, parentPosition);
					if (s_drawList[i].childIndex != UINT32_MAX)
						childStillAlive = true;
				}
				// The group is cleaned. Remove it from the list.
				if (!childStillAlive)
				{
					//
					// ROOT IS DANGLING NOW AGAIN.
					//
					root = &s_drawList[rootIndex];
					if (groupBegin == root->childIndex)
						root->childIndex = head.nextGroup;
					else
						s_drawList[previousGroup].nextGroup = head.nextGroup;
				}
			}
			previousGroup = groupBegin;
			groupBegin = head.nextGroup;
		}
	}
}

void BatchRenderer::BeginUIPass()
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	commandBuffer.BindVertexBuffer(s_vertexBuffer->GetBufferObject(), 0);
	commandBuffer.BindIndexBuffer(s_indexBuffer->GetBufferObject(), 0);
	s_hoverControlLastFrame = s_hoverControlThisFrame;
	s_hoverControlThisFrame = nullptr;
}

void BatchRenderer::PaintControl(WeakPtr<Control> control)
{
	control->Measure(glm::vec2(Window::Width(), Window::Height()));
	control->Arrange(glm::vec2(0.0f, 0.0f), glm::vec2(Window::Width(), Window::Height()));
	DrawElement root = { control, 0, 0, UINT_MAX };
	s_drawList.push_back(root);
	while (s_drawList[0].childIndex != UINT_MAX)
	{
		s_currentBatchMaterial = nullptr;
		BuildBatch(0, glm::vec2(0.0f, 0.0f));
	}
	s_drawList.clear();
}

void BatchRenderer::PaintPopup(WeakPtr<Control> control, glm::vec2 pos)
{
	glm::vec2 desiredSize = control->Measure(glm::vec2(0.0f, 0.0f));
	control->Arrange(glm::vec2(0.0f, 0.0f), desiredSize);
	DrawElement root = { control, 0, 0, UINT_MAX };
	s_drawList.push_back(root);
	while (s_drawList[0].childIndex != UINT_MAX)
	{
		s_currentBatchMaterial = nullptr;
		BuildBatch(0, pos);
	}
	s_drawList.clear();
}

void BatchRenderer::EndUIPass()
{
	if (s_indexEnd != s_indexBegin)
		Flush();

	// Dispatch mouse event.
	glm::vec2 mousePos = glm::vec2(Input::MouseX(), Input::MouseY()) - s_hoverControlPos;
	if (s_hoverControlThisFrame != s_hoverControlLastFrame)
	{
		if (s_hoverControlLastFrame != nullptr)
			s_hoverControlLastFrame->OnMouseEvent(MouseEvent::Leave, mousePos);
		if (s_hoverControlThisFrame != nullptr)
			s_hoverControlThisFrame->OnMouseEvent(MouseEvent::Enter, mousePos);
	}
	else if (s_hoverControlThisFrame != nullptr)
	{
		if (Input::MouseDeltaX() != 0.0f || Input::MouseDeltaY() != 0.0f)
			s_hoverControlThisFrame->OnMouseEvent(MouseEvent::Move, mousePos);
		if (Input::MouseScroll() != 0.0f)
			s_hoverControlLastFrame->OnMouseEvent(MouseEvent::Scroll, mousePos);
	}
	if (Input::HasPressed(KeyCode::LMB))
	{
		static double s_doubleClickTimer = 0.0;
		static WeakPtr<Control> s_clickedControl = nullptr;
		if (s_hoverControlThisFrame != nullptr)
		{
			if (s_hoverControlLastFrame == s_clickedControl && Time::Current() - s_doubleClickTimer <= s_doubleClickTime)
			{
				s_hoverControlThisFrame->OnMouseEvent(MouseEvent::DoubleClick, mousePos);
				s_clickedControl = nullptr;
			}
			else
			{
				s_hoverControlThisFrame->OnMouseEvent(MouseEvent::Press, mousePos);
				s_clickedControl = s_hoverControlThisFrame;
			}
		}
		else
			s_clickedControl = nullptr;
		s_doubleClickTimer = Time::Current();

		// Clear pop-ups when we press LMB elsewhere.
		GameInstance::ClearPopupExcept(s_hoverControlThisFrame);

		// Clear focus if we press LMB outside of a control.
		if (s_hoverControlThisFrame != s_activeControl)
			s_activeControl = nullptr;
	}
	else if (Input::HasReleased(KeyCode::LMB))
	{
		if (s_hoverControlLastFrame != nullptr)
			s_hoverControlLastFrame->OnMouseEvent(s_hoverControlThisFrame == s_hoverControlLastFrame ? MouseEvent::Release : MouseEvent::Abort, mousePos);
	}

	// Dispatch keyboard event.
	if (s_activeControl != nullptr && s_activeControl->GetAcceptKeyboardEvent())
	{
		uint32_t chr;
		while ((chr = Input::GetInputChar()) != Input::KEY_INPUT_END)
		{
			if (chr & Input::INPUT_CHAR_FLAG)
				s_activeControl->OnInput(KeyboardEvent::CharInput, chr & ~Input::INPUT_CHAR_FLAG);
			else
				s_activeControl->OnInput(KeyboardEvent::KeyPress, chr);
		}
	}
}

void BatchRenderer::SetFocus(WeakPtr<Control> control)
{
	if (s_activeControl != nullptr && s_activeControl->GetAcceptFocusEvent())
		s_activeControl->OnLoseFocus();
	s_activeControl = control;
}