export module graphics:DirectionalLight;

import math;
import color;

import <cmath>;

export namespace graphics
{
	struct DirectionalLight
	{
		// The direction is normalized and reversed (i.e. it points to the light, not away from
		// it).
		math::Vec3 direction;
		float strength;
		float specularStrength;
		math::Vec3 specularColor;

		DirectionalLight() = default;
		explicit DirectionalLight(const math::Vec3& direction,
			const float strength = 1.0f, const float specularStrength = 1.0f,
			const math::Vec3& specularColor = color::white.subvector<3>()) :
			direction(direction.unit()), strength(strength), specularStrength(specularStrength),
			specularColor(specularColor) {}
	};
}
