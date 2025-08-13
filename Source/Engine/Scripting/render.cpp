#include "Engine/Scripting/render.h"

using namespace glex;
using namespace glex::py;

PyRetVal<void> FrameBuffer::Create(Type<RenderPass>* renderPass, PyObject* attachmentList, uint32_t width, uint32_t height)
{
	auto size = PyList_Size(attachmentList);
	if (size == -1)
		return PyStatus::RelayException;
	Vector<gl::ImageView> attachments(size);
	for (uint32_t i = 0; i < size; i++)
	{
		Type<Texture>* item = Type<Texture>::Cast(PyList_GetItem(attachmentList, i));
		if (item == nullptr)
			PyStatus::RaiseException;
		attachments[i] = item->Get().texture->GetImageView();
	}
	this->width = width;
	this->height = height;
	return frameBuffer.Create(renderPass->Get().renderPass, attachments, width, height) ? PyStatus::Success : PyStatus::RaiseException;
}