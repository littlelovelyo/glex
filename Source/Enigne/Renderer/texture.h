/**
 * A texture encapsulates an image, an image view and a sampler.
 */
#pragma once
#include "Engine/Renderer/image.h"
#include "Core/Container/optional.h"

namespace glex
{
	class Texture : private Unmoveable
	{
	private:
		Optional<ImageView> m_imageView;
		gl::Sampler m_samplerObject; // External object.

	public:
		Texture(char const* imageFile, gl::Sampler sampler);
		Texture(char const* imageFile, gl::ImageFormat formatOverride, gl::Sampler sampler);
		Texture(char const* left, char const* right, char const* up, char const* bottom, char const* front, char const* back, gl::Sampler sampler);
		~Texture();
		bool IsValid() const { return m_samplerObject.GetHandle() != VK_NULL_HANDLE; }
		void SetSampler(gl::Sampler sampler);
		ImageView const& GetImageView() const { return *m_imageView; }
		gl::Sampler GetSampler() const { return m_samplerObject; }
		glm::uvec2 Size() const { return m_imageView->GetImage()->Size(); }
	};
}