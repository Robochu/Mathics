export module graphics:Material;

export namespace graphics
{
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
	struct Material
	{
		float kA;
		float kT;
		unsigned int kE;
		float kM;
		float kX;
		float kR;
		float kF;
	};

	Material flat = {0.01f, 0.0f, 0u, 3.0f, 1.0f, 0.0f, 0.0f};
	Material mixed = {0.01f, 0.1f, 10u, 3.0f, 1.0f, 0.0f, 0.0f};
	Material shiny = {0.01f, 1.0f, 100u, 3.0f, 1.0f, 0.0f, 0.0f};
	Material reflective = {0.01f, 1.0f, 100u, 3.0f, 1.0f, 0.5f, 0.5f};
	Material chrome = {0.0f, 0.0f, 0u, 0.0f, 0.0f, 1.0f, 0.0f};
	Material specularChrome = {0.0f, 1.0f, 100u, 0.0f, 0.0f, 1.0f, 0.0f};
	Material defaultMaterial = mixed;
}
