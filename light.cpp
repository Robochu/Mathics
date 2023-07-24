export module graphics:light;

import :DirectionalLight;
import :PointLight;
import :Material;
import :CubeMap;

import math;

import <vector>;
import <cmath>;

export namespace graphics
{
	// Global variables as a workaround to what I believe is an MSVC modules bug, although I might
	// just be dumb.
	CubeMap* reflectionMap;

	// Explanation for lighting factors:
	//  kA - ambience factor. Corresponds to the minimum possible value of kD
	//  kD - diffuse lighting factor
	//  kS - specular lighting color
	//  kT - specular strength multiplier
	//  kE - specular exponent
	//  kM - stretching factor in the final color curve
	//  kX - offset of the final color curve
	//  kR - how reflective the surface is
	//  kF - fresnel factor
	//  kU - combination of kR and kF
	math::Vec4 light(const math::Vec4& color, const math::Vec3& normal,
		const math::Vec3& surfacePoint, const math::Vec3& cameraPosition,
		std::vector<DirectionalLight>& directionalLights, std::vector<PointLight>& pointLights,
		const float w, const Material& material)
	{
		float kD = material.kA;
		math::Vec3 kS = math::Vec3(0.0f);

		const math::Vec3 ray = (cameraPosition - surfacePoint).unit();
		const math::Vec3 reflectedRay = 2.0f * ray.projectOnto(normal) - ray;
		if (material.kT == 0.0f && material.kR != 1.0f)
		{
			for (const DirectionalLight& directionalLight : directionalLights)
			{
				kD += std::max(0.0f, directionalLight.direction.dot(normal) *
					directionalLight.strength);
			}

			for (PointLight& pointLight : pointLights)
			{
				float visibility = pointLight.shadowMap.getVisibility(w);
				if (visibility != 0.0f)
				{
					math::Vec3 direction = pointLight.getPosition() - surfacePoint;
					const float distance = direction.norm();
					kD += visibility * std::max(0.0f, direction.dot(normal) * pointLight.strength /
						(distance * distance * distance));
				}
			}
			kD = (1.0f - 1.0f / (material.kM * kD + material.kX));
		}
		else if (material.kR == 1.0f)
		{
			for (const DirectionalLight& directionalLight : directionalLights)
			{
				kS += math::power(std::max(0.0f, reflectedRay.dot(directionalLight.direction)),
					material.kE) * directionalLight.specularStrength * material.kT *
					directionalLight.specularColor;
			}

			for (PointLight& pointLight : pointLights)
			{
				float visibility = pointLight.shadowMap.getVisibility(w);
				if (visibility != 0.0f)
				{
					math::Vec3 direction = pointLight.getPosition() - surfacePoint;
					const float distance = direction.norm();
					direction.normalize();
					kS += visibility * math::power(std::max(0.0f, reflectedRay.dot(direction)),
						material.kE) * pointLight.specularStrength * material.kT / distance *
						pointLight.specularColor;
				}
			}
		}
		else
		{
			for (const DirectionalLight& directionalLight : directionalLights)
			{
				kD += std::max(0.0f, directionalLight.direction.dot(normal) *
					directionalLight.strength);
				kS += math::power(std::max(0.0f, reflectedRay.dot(directionalLight.direction)),
					material.kE) * directionalLight.specularStrength * material.kT *
					directionalLight.specularColor;
			}

			for (PointLight& pointLight : pointLights)
			{
				float visibility = pointLight.shadowMap.getVisibility(w);
				if (visibility != 0.0f)
				{
					math::Vec3 direction = pointLight.getPosition() - surfacePoint;
					const float distance = direction.norm();
					direction.normalize();
					kD += visibility * std::max(0.0f, direction.dot(normal) * pointLight.strength /
						(distance * distance));
					kS += visibility * math::power(std::max(0.0f, reflectedRay.dot(direction)),
						material.kE) * pointLight.specularStrength * material.kT / distance *
						pointLight.specularColor;
				}
			}
			kD = (1.0f - 1.0f / (material.kM * kD + material.kX));
		}

		const math::Vec3 subcolor = color.subvector<3>();
		math::Vec3 result = kS;
		if (material.kR == 0.0f || !reflectionMap)
		{
			result += kD * subcolor;
		}
		else
		{
			float kU = material.kR - material.kF * ray.dot(normal);
			result += (1.0f - kU) * kD * subcolor + kU *
				math::Vec3(reflectionMap->lookup(reflectedRay));
		}

		if (result.max() > 1.0f)
		{
			result /= result.max();
		}
		return math::Vec4(result.r(), result.g(), result.b(), 1.0f) * color.a();
	}
}
