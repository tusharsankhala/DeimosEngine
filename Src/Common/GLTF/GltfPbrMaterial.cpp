#include "GltfPbrMaterial.h"

void SetDefaultMaterialParameters(PBRMaterialParameters* pPBRMaterialParameters)
{
	pPBRMaterialParameters->m_doubleSided = false;
	pPBRMaterialParameters->m_blending = false;

	pPBRMaterialParameters->m_params.m_emissiveFactor			= math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	pPBRMaterialParameters->m_params.m_baseColorFactor			= math::Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	pPBRMaterialParameters->m_params.m_metallicRoughnessValues	= math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	pPBRMaterialParameters->m_params.m_specularGlossinessFactor	= math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

void ProcessMaterials(const json::object_t& material, PBRMaterialParameters* tfMat, std::map<std::string, int>& textureIds)
{
	// Load material constants.
	//

}

bool DoesMaterialUseSemantic(DefineList& defines, const std::string semanticName)
{
	// Search if any *TexCoord mentions this channel
	//
	if (semanticName.substr(0, 9) == "TEXCOORD_")
	{
		char id = semanticName[9];

		for (auto def : defines)
		{
			uint32_t size = static_cast<uint32_t>(def.first.size());
			if (size <= 8)
				continue;

			if (def.first.substr(size - 8) == "TexCoord")
			{
				if (id == def.second.c_str()[0])
				{
					return true;
				}
			}
		}

		return false;
	}

	return false;
}


void GetSrgbAndCutOffOfImageGivenItsUse(int imageIndex, const json& materials, bool* pSrgbOut, float* pCutOff)
{
	*pSrgbOut = false;
	*pCutOff = 1.0f;			// no cutoff.

	for (int m = 0; m < materials.size(); ++m)
	{
		const json& material = materials[m];

		if (GetElementInt(material, "pbrMetallicRoughness/baseColorTexture/index", -1) == imageIndex)
		{
			*pSrgbOut = true;

			*pCutOff = GetElementFloat(material, "alphaCutoff", 0.5);

			return;
		}

		if (GetElementInt(material, "extensions/KHR_materials_pbrSpecularGlossiness/specularGlossinessTexture/index", -1) == imageIndex)
		{
			*pSrgbOut = true;

			return;
		}

		if (GetElementInt(material, "extensions/KHR_materials_pbrSpecularGlossiness/diffuseTexture/index", -1) == imageIndex)
		{
			*pSrgbOut = true;
			return;
		}

		if (GetElementInt(material, "emissiveTexture/index", -1) == imageIndex)
		{
			*pSrgbOut = true;
			return;
		}
	}
}