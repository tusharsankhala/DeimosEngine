#pragma once

#include "VK/Base/Device.h"
#include "VK/Base/UploadHeap.h"
#include "VK/Base/ResourceViewHeaps.h"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include "Common/Misc/DDSLoader.h"

namespace Engine_VK
{
	// This class provides functionality to create a 2D-texture/Render target.

	class Texture
	{
	public:
		Texture();
		virtual			~Texture();
		virtual void	OnDestroy();

		// Load File into heap.
		INT32 Init( Device* pDevice, VkImageCreateInfo* pCreateInfo, const char* name = nullptr );
		INT32 InitRenderTarget( Device* pDevice, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits msaa,
								VkImageUsageFlags usage, bool bUAV, const char* name = nullptr, VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0 );
		INT32 InitDepthStencil( Device* pDevice, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits msaa,const char* name = nullptr );
		bool InitFromFile( Device* pDevice, UploadHeap* pUploadHeap, const char *szFilename, bool useSRGB = false, VkImageUsageFlags usageFlags = 0, float cutOff = 1.0f );
		bool InitFromData( Device* pDevice, UploadHeap& pUploadHeap, const char* szFilename, bool useSRGB = false, VkImageUsageFlags usageFlags = 0, float cutOff = 1.0f);

		VkImage Resource() const { return m_pResource; }

		void CreateRTV( VkImageView* pImageView, int mipLevel = -1, VkFormat format = VK_FORMAT_UNDEFINED );
		void CreateSRV( VkImageView* pImageView, int mipLevel = -1 );
		void CreateDSV( VkImageView* pView );
		void CreateCubeSRV( VkImageView* pImageView );


		uint32_t GetWidth()			const { return m_header.width; }
		uint32_t GetHeight()		const { return m_header.height; }
		uint32_t GetMipCount()		const { return m_header.mipMapCount; }
		uint32_t GetArraySize()		const { return m_header.arraySize; }
		VkFormat GetFormat()		const { return m_format; }

	private:
		Device*				m_pDevice		= NULL;
		std::string			m_name			= "";
#ifdef USE_VMA
		VmaAllocation		m_imageAlloc	= VK_NULL_HANDLE;
#else
		VkDeviceMemory		m_deviceMemory	= VK_NULl_HANDLE;
#endif

		VkFormat			m_format;
		VkImage				m_pResource = VK_NULL_HANDLE;

		IMG_INFO			m_header;

	protected:

		struct FootPrint
		{
			UINT8*			pixels;
			uint32_t		width, height, offset;
		
		} footprints[6][12];

		VkImage CreateTextureCommitted( Device* pDevice, UploadHeap* pUploadHeap, const char* name, bool useSRGB = false, VkImageUsageFlags usageFlags = 0 );
		void	LoadAndUpload( Device* pDevice, UploadHeap* pUploadHeap, ImgLoader* pDds, VkImage pTexture2D );

		bool	isCubemap() const;
	};
}
