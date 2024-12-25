#pragma once
#include "GltfHelpers.h"
#include "VK/Base/ShaderCompilerHelper.h"

struct PBRMaterialParametersConstantBuffer
{
	math::Vector4						m_emissiveFactor;

	// pbrMetallicRoughness.
	math::Vector4						m_baseColorFactor;
	math::Vector4						m_metallicRoughnessValues;

	// KHR_materials_pbrSpecularGlossiness.
	math::Vector4						m_diffuseFactor;
	math::Vector4						m_specularGlossinessFactor;
};

struct PBRMaterialParameters
{
	bool								m_doubleSided = false;
	bool								m_blending = false;

	DefineList							m_defines;

	PBRMaterialParametersConstantBuffer	m_params;
};

// Read GLTF material and store it in our structure.
void SetDefaultMaterialParameters(PBRMaterialParameters* pPBRMaterialParameters);
void ProcessMaterials(const json::object_t& material, PBRMaterialParameters* tfMat, std::map<std::string, int>& textureIds);
bool DoesMaterialUseSemantic(DefineList& defines, const std::string semanticName);
void GetSrgbAndCutOffOfImageGivenItsUse( int imageIndex, const json& materials, bool* pSrgbOut, float* pCutOff);
