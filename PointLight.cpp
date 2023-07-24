export module graphics:PointLight;

import :CubeMap;

import math;
import color;

import <cmath>;

export namespace graphics
{
	class PointLight
	{
		math::Vec3 position;

	public:
		float strength;
		float specularStrength;
		math::Vec3 specularColor;
		CubeMap shadowMap;

		PointLight() = default;
		explicit PointLight(const unsigned int resolution, const math::Vec3& position,
			const float strength, const float specularStrength,
			const math::Vec3& specularColor = color::white.subvector<3>()) : position(position),
			strength(strength), specularStrength(specularStrength), specularColor(specularColor),
			shadowMap(resolution, position) {}

		math::Vec3 getPosition() const
		{
			return position;
		}
		void setPosition(const math::Vec3& position)
		{
			this->position = position;
			shadowMap.setPosition(position);
		}
	};
}
