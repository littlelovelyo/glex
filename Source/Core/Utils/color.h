#pragma once
#include "Core/commdefs.h"
#include <glm/gtc/constants.hpp>

namespace glex
{
	class ColorUtils : private StaticClass
	{
	public:
		/**
		 * Hue: 0~2pi.
		 * Saturation: 0~1.
		 * Value: 0~1.
		 */
		static glm::vec3 HSV2RGB(glm::vec3 hsv)
		{
			float const& h = hsv.r;
			float const& s = hsv.g;
			float const& v = hsv.b;

			float hh = h / glm::radians(60.0f);
			float hi = glm::floor(hh);
			float f = hh - hi;
			float p = v * (1 - s);
			float q = v * (1 - f * s);
			float t = v * (1 - (1 - f) * s);
			switch (static_cast<uint32_t>(hi))
			{
				case 0: return { v, t, p };
				case 1: return { q, v, p };
				case 2: return { p, v, t };
				case 3: return { p, q, v };
				case 4: return { t, p, v };
				default: return { v, p, q };
			}
		}

		static glm::vec3 RGB2HSV(glm::vec3 rgb)
		{
			float max = Max(rgb.r, rgb.g, rgb.b);
			float min = Min(rgb.r, rgb.g, rgb.b);
			float v = max;
			float s = v < glm::epsilon<float>() ? 0.0f : (v - min) / v;
			float h = v == rgb.r ? glm::radians(60.0f) * (rgb.g - rgb.b) / (v - min) :
				v == rgb.g ? glm::radians(120.0f) + glm::radians(60.0f) * (rgb.b - rgb.r) / (v - min) :
				glm::radians(240.0f) + glm::radians(60.0f) * (rgb.r - rgb.g) / (v - min);
			return { h, s, v };
		}
		
		constexpr static glm::vec3 RGBFromHex(uint32_t hex)
		{
			union
			{
				uint32_t fullColor;
				struct { uint8_t a, b, g, r; };
			} data;
			data.fullColor = hex;
			return glm::vec3(data.r / 255.0f, data.g / 255.0f, data.b / 255.0f);
		}

		constexpr static glm::vec4 RGBAFromHex(uint32_t hex)
		{
			union
			{
				uint32_t fullColor;
				struct { uint8_t a, b, g, r; };
			} data;
			data.fullColor = hex;
			return glm::vec4(data.r / 255.0f, data.g / 255.0f, data.b / 255.0f, data.a / 255.0f);
		}
	};
}