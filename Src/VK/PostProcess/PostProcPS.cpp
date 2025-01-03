
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/ShaderCompilerHelper.h"


#include "PostProcPS.h"

namespace Engine_VK
{

	void PostProcPS::OnCreate(Device* pDevice, VkRenderPass renderPass,
		const std::string& shaderFileName,
		const std::string& shaderEntryPoint, const std::string& shaderCompilerParams,
		StaticBufferPool* pStaticBufferPool,
		DynamicBufferRing* pDynamicBufferRing,
		VkDescriptorSetLayout descriptorSetLayout,
		VkPipelineColorBlendStateCreateInfo* pBlendDesc,
		VkSampleCountFlagBits sampleDescCount)
	{
		m_pDevice = pDevice;

		// Create the vertex shader.
		static const char* vertexShader =
			"static const float4 FullScreenVertPos[3] = { float4(-1, 1, 1, 1), float4(3, 1, 1, 1), float4(-1, -3, 1, 1) }; \
			 static const float2 FullScreenVertUVs[3] = { float2(0, 0), float2(2, 0), float2(0, 2) };\
			struct VERTEX_OUT\
			{\
				float2 vTexture  : TEXCOOORD;\
				float4 vPosition : SV_POSITION;\
			};\
			VERTEX_OUT mainVS(uint vertexId : SV_VertexID)\
			{\
				VERTEX_OUT Output;\
				output.vPosition = FullScreenVertesPos[vertexId];\
				output.vTexture  = FullScreenVertesUVs[vertexId];\
				return Output;\
			}";

		VkResult res;

		// Compile shaders
		//
		DefineList attributeDefines;

		VkPipelineShaderStageCreateInfo m_vertexShader;
#ifdef _DEBUG
		std::string CompileFlagsVS("-T vs_6_0 -Zi -Od");
#else
		std::string CompileFlagsVS("-T vs_6_0");
#endif // _DEBUG

		res = VKCompileFromString(m_pDevice->GetDevice(), SST_HLSL, VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "mainVS", CompileFlagsVS.c_str(), &attributeDefines, &m_vertexShader);
		assert(res == VK_SUCCESS);

		m_fragmentShaderName = shaderEntryPoint;
		VkPipelineShaderStageCreateInfo m_fragmentShader;
		res = VKCompileFromFile(m_pDevice->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT, shaderFileName.c_str(), m_fragmentShaderName.c_str(), shaderCompilerParams.c_str(), &attributeDefines, &m_fragmentShader);
		assert(res == VK_SUCCESS);

		m_shaderStages.clear();
		m_shaderStages.push_back(m_vertexShader);
		m_shaderStages.push_back(m_fragmentShader);

		// Create pipeline Layout
		//
		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
		pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pPipelineLayoutCreateInfo.pNext = NULL;
		pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
		pPipelineLayoutCreateInfo.setLayoutCount = 1;
		pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

		res = vkCreatePipelineLayout(pDevice->GetDevice(), &pPipelineLayoutCreateInfo, NULL, &m_pipelineLayout);
		assert(res == VK_SUCCESS);

		UpdatePipeline(renderPass, pBlendDesc, sampleDescCount);

	}
	//--------------------------------------------------------------------------------------
	//
	// UpdatePipeline
	//
	//--------------------------------------------------------------------------------------
	void PostProcPS::UpdatePipeline(VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo* pBlendDesc, VkSampleCountFlagBits sampleDescCount)
	{
		if (m_renderPass == VK_NULL_HANDLE)
			return;

		if (m_pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_pDevice->GetDevice(), m_pipeline, nullptr);
			m_pipeline = VK_NULL_HANDLE;
		}

		VkResult res;

		// Input assembly and state layout.
		VkPipelineVertexInputStateCreateInfo vi = {};
		vi.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext							= NULL;
		vi.flags							= 0;
		vi.vertexBindingDescriptionCount	= 0;
		vi.pVertexBindingDescriptions		= nullptr;
		vi.vertexAttributeDescriptionCount	= 0;
		vi.pVertexAttributeDescriptions		= nullptr;


		VkPipelineInputAssemblyStateCreateInfo ia = {};
		ia.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.pNext					= NULL;
		ia.flags					= 0;
		ia.primitiveRestartEnable	= VK_FALSE;
		ia.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		

		// rasterizer state

		VkPipelineRasterizationStateCreateInfo rs;
		rs.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.pNext					= NULL;
		rs.flags					= 0;
		rs.polygonMode				= VK_POLYGON_MODE_FILL;
		rs.cullMode					= VK_CULL_MODE_NONE;
		rs.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.depthClampEnable			= VK_FALSE;
		rs.rasterizerDiscardEnable	= VK_FALSE;
		rs.depthBiasEnable			= VK_FALSE;
		rs.depthBiasConstantFactor	= 0;
		rs.depthBiasClamp			= 0;
		rs.depthBiasSlopeFactor		= 0;
		rs.lineWidth				= 1.0f;

		VkPipelineColorBlendAttachmentState att_state[1];
		att_state[0].blendEnable			= VK_TRUE;
		att_state[0].srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
		att_state[0].dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		att_state[0].colorBlendOp			= VK_BLEND_OP_ADD;
		att_state[0].srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		att_state[0].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		att_state[0].alphaBlendOp			= VK_BLEND_OP_ADD;
		att_state[0].colorWriteMask			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo cb;
		cb.sType							= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.flags							= 0;
		cb.pNext							= NULL;
		cb.attachmentCount					= 1;
		cb.pAttachments						= att_state;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_NO_OP;
		cb.blendConstants[0] = 1.0f;
		cb.blendConstants[1] = 1.0f;
		cb.blendConstants[2] = 1.0f;
		cb.blendConstants[3] = 1.0f;

		std::vector<VkDynamicState> dynamicStateEnables = {
		   VK_DYNAMIC_STATE_VIEWPORT,
		   VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = NULL;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();

		// view port state

		VkPipelineViewportStateCreateInfo vp = {};
		vp.sType					= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext					= NULL;
		vp.flags					= 0;
		vp.viewportCount			= 1;
		vp.scissorCount				= 1;
		vp.pScissors				= NULL;
		vp.pViewports				= NULL;

		// depth stencil state

		VkPipelineDepthStencilStateCreateInfo ds;
		ds.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext					= NULL;
		ds.flags					= 0;
		ds.depthTestEnable			= VK_TRUE;
		ds.depthWriteEnable			= VK_FALSE;
		ds.depthCompareOp			= VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.depthBoundsTestEnable	= VK_FALSE;
		ds.stencilTestEnable		= VK_FALSE;
		ds.back.failOp				= VK_STENCIL_OP_KEEP;
		ds.back.passOp				= VK_STENCIL_OP_KEEP;
		ds.back.compareOp			= VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask			= 0;
		ds.back.reference			= 0;
		ds.back.depthFailOp			= VK_STENCIL_OP_KEEP;
		ds.back.writeMask			= 0;
		ds.minDepthBounds			= 0;
		ds.maxDepthBounds			= 0;
		ds.stencilTestEnable		= VK_FALSE;
		ds.front					= ds.back;

		// multi sample state

		VkPipelineMultisampleStateCreateInfo ms;
		ms.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pNext					= NULL;
		ms.flags					= 0;
		ms.pSampleMask				= NULL;
		ms.rasterizationSamples		= sampleDescCount;
		ms.sampleShadingEnable		= VK_FALSE;
		ms.alphaToCoverageEnable	= VK_FALSE;
		ms.alphaToOneEnable			= VK_FALSE;
		ms.minSampleShading			= 0.0;

		// Create pipeline.

		VkGraphicsPipelineCreateInfo pipeline = {};
		pipeline.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.pNext					= NULL;
		pipeline.layout					= m_pipelineLayout;
		pipeline.basePipelineHandle		= VK_NULL_HANDLE;
		pipeline.basePipelineIndex		= 0;
		pipeline.flags					= 0;
		pipeline.pVertexInputState		= &vi;
		pipeline.pInputAssemblyState	= &ia;
		pipeline.pRasterizationState	= &rs;
		pipeline.pColorBlendState		= (pBlendDesc == NULL) ? &cb : pBlendDesc;
		pipeline.pTessellationState		= NULL;
		pipeline.pMultisampleState		= &ms;
		pipeline.pDynamicState			= &dynamicState;
		pipeline.pViewportState			= &vp;
		pipeline.pDepthStencilState		= &ds;
		pipeline.pStages				= m_shaderStages.data();
		pipeline.stageCount				= (uint32_t)m_shaderStages.size();
		pipeline.renderPass				= renderPass;
		pipeline.subpass				= 0;

		res = vkCreateGraphicsPipelines(m_pDevice->GetDevice(), m_pDevice->GetPipelineCache(), 1, &pipeline, NULL, &m_pipeline);
		assert(res == VK_SUCCESS);
		SetResourceName(m_pDevice->GetDevice(), VK_OBJECT_TYPE_PIPELINE, (uint64_t)m_pipeline, "cacaca P");
	}

	//--------------------------------------------------------------------------------------
	//
	// OnDestroy
	//
	//--------------------------------------------------------------------------------------
	void PostProcPS::OnDestroy()
	{
		if (m_pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_pDevice->GetDevice(), m_pipeline, nullptr);
			m_pipeline = VK_NULL_HANDLE;
		}

		if (m_pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_pipelineLayout, nullptr);
			m_pipelineLayout = VK_NULL_HANDLE;
		}
	}

	//--------------------------------------------------------------------------------------
	//
	// OnDraw
	//
	//--------------------------------------------------------------------------------------
	void PostProcPS::Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet descriptorSet)
	{
		if (m_pipeline == VK_NULL_HANDLE)
			return;

		// Bind descriptor set.
		//
		int numUniformOffsets = 0;
		uint32_t uniformOffset = 0;

		if (pConstantBuffer != NULL && pConstantBuffer->buffer != NULL )
		{
			numUniformOffsets = 1;
			uniformOffset = (uint32_t)pConstantBuffer->offset;
		}

		VkDescriptorSet descriptprSets[1] = { descriptorSet };
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, descriptprSets, numUniformOffsets, &uniformOffset);

		// Bind Pipeline
		//
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		// Draw
		//
		vkCmdDraw( cmd_buf, 3, 1, 0, 0);
			
	}
}