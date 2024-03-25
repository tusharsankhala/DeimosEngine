#include "Common/stdafx.h"
#include "VK/Base/ResourceViewHeaps.h"
#include "VK/Base/Texture.h"
#include "Common/Misc/Misc.h"
#include "Common/Misc/DxgiFormatHelper.h"
#include "VK/Extensions/ExtDebugUtils.h"

namespace Engine_VK
{
    //--------------------------------------------------------------------------------------
   // Constructor of the Texture class
   // initializes all members
   //--------------------------------------------------------------------------------------
    Texture::Texture() {}

    //--------------------------------------------------------------------------------------
    // Destructor of the Texture class
    //--------------------------------------------------------------------------------------
    Texture::~Texture() {}

    void Texture::OnDestroy()
    {
#ifdef USE_VMA
        if ( m_pResource != VK_NULL_HANDLE )
        {
            vmaDestroyImage( m_pDevice->GetAllocator(), m_pResource, m_imageAlloc );
            m_pResource = VK_NULL_HANDLE;
        }
#else
        if ( m_deviceMemory != VK_NULL_HANDLE )
        {
            vkDestroyImage( m_pDevice->GetDevice(), m_pResource, nullptr );
            vkFreeMemory( m_pDevice->GetDevice(), m_deviceMemory, nullptr );
            m_deviceMemory = VK_NULL_HANDLE;
        }

#endif // USE_VMA

    }

    bool Texture::isCubemap() const
    {
        return m_header.arraySize == 6;
    }

    INT32 Texture::Init( Device* pDevice, VkImageCreateInfo* pCreateInfo, const char* name )
    {
        m_pDevice = pDevice;
        m_header.mipMapCount = pCreateInfo->mipLevels;
        m_header.width       = pCreateInfo->extent.width;
        m_header.height      = pCreateInfo->extent.height;
        m_header.depth       = pCreateInfo->extent.depth;
        m_header.arraySize   = pCreateInfo->arrayLayers;
        m_format             = pCreateInfo->format;
        if( name )
            m_name = name;

#ifdef USE_VMA
        VmaAllocationCreateInfo imageAllocaCreateInfo = {};
        imageAllocaCreateInfo.usage     = VMA_MEMORY_USAGE_GPU_ONLY;
        imageAllocaCreateInfo.flags     = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        imageAllocaCreateInfo.pUserData = (void*)m_name.c_str();
        VmaAllocationInfo gpuImageAllocInfo = {};
        VkResult res = vmaCreateImage( m_pDevice->GetAllocator(), pCreateInfo, &imageAllocaCreateInfo, &m_pResource, &m_imageAlloc, &gpuImageAllocInfo );
        assert( res == VK_SUCCESS );
        SetResourceName( pDevice->GetDevice(), VK_OBJECT_TYPE_IMAGE, ( uint64_t)m_pResource, m_name.c_str() );

       

#else
        /* Create Image */
        VkResult res = vmaCreateImage( m_pDevice->GetAllocator(), pCreateInfo, NULL, &m_pResource );
        assert(res == VK_SUCCESS);

        VkMemoryRequirements mem_reqs;
        vkGetImageMemoryRequirements( m_pDevice->GetDevice(), m_pResource, &mem_reqs );

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext            = NULL;
        alloc_info.allocationSize   = 0;
        alloc_info.allocationSize   = mem_reqs.size;
        alloc_info.memoryTypeIndex  = 0;

        bool pass = memory_type_from_properties( m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_info.memoryTypeIndex );
        
        assert( pass );

        /* Allocate memory */
        res = vkAllocateMemory( m_pDevice->GetDevice(), &alloc_info, NULL, &m_deviceMemory );
        assert( res == VK_SUCCESS );

        /* Bind memory */
        res = vkBindImageMemory( m_pDevice->GetDevice(), &m_pResource, m_deviceMemory, 0 );
        assert( res == VK_SUCCESS );

#endif

        return 0;
    }

    INT32 Texture::InitRenderTarget( Device* pDevice, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits msaa,
        VkImageUsageFlags usage, bool bUAV, const char* name, VkImageCreateFlagBits flags )
    {
        VkImageCreateInfo image_info = {};
        image_info.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.pNext                    = NULL;
        image_info.imageType                = VK_IMAGE_TYPE_2D;
        image_info.format                   = format;
        image_info.extent.width             = width;
        image_info.extent.height            = height;
        image_info.extent.depth             = 1;
        image_info.mipLevels                = 1;
        image_info.arrayLayers              = 1;
        image_info.samples                  = msaa;
        image_info.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.queueFamilyIndexCount    = 0;
        image_info.pQueueFamilyIndices      = NULL;
        image_info.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
        image_info.usage                    = usage;
        image_info.flags                    = flags;
        image_info.tiling                   = VK_IMAGE_TILING_OPTIMAL;          // VK_IMAGE_TILING_LINEAR should never be used and will never be faster.

        return Init( pDevice, &image_info, name );
    }

    INT32 Texture::InitDepthStencil(Device* pDevice, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits msaa, const char* name )
    {
        VkImageCreateInfo image_info = {};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.pNext = NULL;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = format;
        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = msaa;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.queueFamilyIndexCount = 0;
        image_info.pQueueFamilyIndices = NULL;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_info.flags = 0;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;          // VK_IMAGE_TILING_LINEAR should never be used and will never be faster.

        return Init( pDevice, &image_info, name );
    }

    void Texture::CreateRTV( VkImageView* pImageView, int mipLevel, VkFormat format )
    {
        VkImageViewCreateInfo info = {};
        info.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image      = m_pResource;
        info.viewType   = VK_IMAGE_VIEW_TYPE_2D;
        if( m_header.arraySize > 1 )
        {
            info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            info.subresourceRange.layerCount = m_header.arraySize;
        }
        else
        {
            info.subresourceRange.layerCount = 1;
        }

        if( format == VK_FORMAT_UNDEFINED )
            info.format = m_format;
        else
            info.format = format;

        if( m_format == VK_FORMAT_D32_SFLOAT )
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        else
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        std::string ResourceName = m_name;

        if( mipLevel == -1 )
        {
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = m_header.mipMapCount;
        }
        else
        {
            info.subresourceRange.baseMipLevel = mipLevel;
            info.subresourceRange.levelCount = 1;
            ResourceName += std::to_string( mipLevel );
        }

        info.subresourceRange.baseArrayLayer = 0;
        VkResult res = vkCreateImageView( m_pDevice->GetDevice(), &info, NULL, pImageView );
        assert( res == VK_SUCCESS );

        SetResourceName( m_pDevice->GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)*pImageView, ResourceName.c_str() );
    }
    
    void Texture::CreateSRV( VkImageView* pImageView, int mipLevel )
    {
        VkImageViewCreateInfo info = {};
        info.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image      = m_pResource;
        info.viewType   = VK_IMAGE_VIEW_TYPE_2D;

        if( m_header.arraySize > 1 )
        {
            info.viewType                       = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            info.subresourceRange.layerCount    = m_header.arraySize;
        }
        else
        {
            info.subresourceRange.layerCount = 1;
        }

        info.format = m_format;
        if( m_format == VK_FORMAT_D32_SFLOAT )
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        else
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if( mipLevel == -1 )
        {
            info.subresourceRange.baseMipLevel  = 0;
            info.subresourceRange.levelCount    = m_header.mipMapCount;
        }
        else
        {
            info.subresourceRange.baseMipLevel  = mipLevel;
            info.subresourceRange.levelCount    = 1;
        }

        info.subresourceRange.baseArrayLayer = 0;

        VkResult res = vkCreateImageView( m_pDevice->GetDevice(), &info, NULL, pImageView );
        assert( res == VK_SUCCESS );

        SetResourceName( m_pDevice->GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)*pImageView, m_name.c_str() );
    }

    void Texture::CreateDSV( VkImageView* pImageView )
    {
        VkImageViewCreateInfo view_info = {};
        view_info.sType                             = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.pNext                             = NULL;
        view_info.image                             = m_pResource;
        view_info.viewType                          = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format                            = m_format;
        view_info.subresourceRange.aspectMask       = VK_IMAGE_ASPECT_DEPTH_BIT;
        view_info.subresourceRange.baseMipLevel     = 0;
        view_info.subresourceRange.levelCount       = 1;
        view_info.subresourceRange.baseArrayLayer   = 0;
        view_info.subresourceRange.layerCount       = 1;

        if( m_format == VK_FORMAT_D16_UNORM_S8_UINT || m_format == VK_FORMAT_D24_UNORM_S8_UINT || m_format == VK_FORMAT_D32_SFLOAT_S8_UINT )
        {
            view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        
        m_header.mipMapCount = 1;
        
        VkResult res = vkCreateImageView( m_pDevice->GetDevice(), &view_info, NULL, pImageView);
        assert(res == VK_SUCCESS);

        SetResourceName( m_pDevice->GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)*pImageView, m_name.c_str() );
    }

    void Texture::CreateCubeSRV(VkImageView* pImageView)
    {
        VkImageViewCreateInfo info              = {};
        info.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image                              = m_pResource;
        info.viewType                           = VK_IMAGE_VIEW_TYPE_CUBE;
        info.format                             = m_format;
        info.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel      = 0;
        info.subresourceRange.levelCount        = m_header.mipMapCount;
        info.subresourceRange.baseArrayLayer    = 0;
        info.subresourceRange.layerCount        = m_header.arraySize;

        VkResult res = vkCreateImageView( m_pDevice->GetDevice(), &info, NULL, pImageView );
        assert(res == VK_SUCCESS);

        SetResourceName( m_pDevice->GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)*pImageView, m_name.c_str() );
    }

    //---------------------------------------------------------------------------------------------------------
    // Entry function to initialize and image from a .DDS texture
    //---------------------------------------------------------------------------------------------------------
    bool Texture::InitFromFile(Device* pDevice, UploadHeap* pUploadHeap, const char* pFilename, bool useSRGB, VkImageUsageFlags usageFlags, float cutOff )
    {
        m_pDevice = pDevice;
        assert( m_pResource == NULL );

        ImgLoader* img = CreateImageLoader( pFilename);
        bool result = img->Load( pFilename, cutOff, &m_header );
        if( result )
        {
            m_pResource = CreateTextureCommitted( pDevice, pUploadHeap, pFilename, useSRGB, usageFlags );
            LoadAndUpload( pDevice, pUploadHeap, img, m_pResource );
        }
        else
        {
            Trace( "Error Loading Texture from file: %s", pFilename );
            assert( result && "Could not load requested file. Please make sure it exists on disk." );
        }

        delete(img);

        return result;
    }

    //--------------------------------------------------------------------------------------
    // entry function to initialize an image from a .DDS texture
    //--------------------------------------------------------------------------------------
    bool Texture::InitFromData( Device* pDevice, UploadHeap& uploadHeap, const IMG_INFO& header, const void* data, const char* name ) 
    {
        assert( !m_pResource && !m_pDevice );
        assert( header.arraySize == 1 && header.mipMapCount == 1 );

        m_pDevice = pDevice;
        m_header = header;
        
        m_pResource = CreateTextureCommitted( m_pDevice, &uploadHeap, name, false );

        // Upload Image.
        {
            VkImageMemoryBarrier copy_barrier = {};
            copy_barrier.sType                          = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            copy_barrier.dstAccessMask                  = VK_ACCESS_TRANSFER_WRITE_BIT;
            copy_barrier.oldLayout                      = VK_IMAGE_LAYOUT_UNDEFINED;
            copy_barrier.newLayout                      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            copy_barrier.srcQueueFamilyIndex            = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier.dstQueueFamilyIndex            = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier.image                          = m_pResource;
            copy_barrier.subresourceRange.aspectMask    = VK_IMAGE_ASPECT_COLOR_BIT;
            copy_barrier.subresourceRange.baseMipLevel  = 0;
            copy_barrier.subresourceRange.levelCount    = m_header.mipMapCount;
            copy_barrier.subresourceRange.layerCount    = m_header.arraySize;
            vkCmdPipelineBarrier( uploadHeap.GetCommandList(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &copy_barrier );
        }

        // compute pixel size.
        //
        UINT32 bytePP = m_header.bitCount / 8;
        if (( m_header.format >= DXGI_FORMAT_BC1_TYPELESS ) && ( m_header.format <= DXGI_FORMAT_BC5_SNORM ) )
        {
            bytePP = (UINT32)GetPixelByteSize(( DXGI_FORMAT) m_header.format );
        }

        UINT8*  pixels = NULL;
        UINT64 uploadHeapSize = m_header.width * m_header.height * 4;
        pixels = uploadHeap.Suballocate( uploadHeapSize, 512 );
        assert( pixels != NULL );

        CopyMemory( pixels, data, m_header.width * m_header.height * bytePP );

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount      = 1;
        region.imageSubresource.baseArrayLayer  = 0;
        region.imageSubresource.mipLevel        = 0;
        region.imageExtent.width                = m_header.width;
        region.imageExtent.height               = m_header.height;
        region.imageExtent.depth                = 1;
        vkCmdCopyBufferToImage( uploadHeap.GetCommandList(), uploadHeap.GetResource(), m_pResource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

        // Prepare to shader read.
        //
        {
            VkImageMemoryBarrier use_barrier = {};
            use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier.image = m_pResource;
            use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            use_barrier.subresourceRange.levelCount = m_header.mipMapCount;
            use_barrier.subresourceRange.layerCount = m_header.arraySize;
            vkCmdPipelineBarrier( uploadHeap.GetCommandList(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  0, 0, NULL, 0, NULL, 1, &use_barrier);
        }

        return true;
    }

    VkFormat TranslateDxgiFormatIntoVulkans( DXGI_FORMAT format )
    {
        switch( format )
        {
            case DXGI_FORMAT_B8G8R8A8_UNORM:        return VK_FORMAT_B8G8R8A8_UNORM;
            case DXGI_FORMAT_R8G8B8A8_UNORM:        return VK_FORMAT_R8G8B8A8_UNORM;
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return VK_FORMAT_R8G8B8A8_SRGB;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
            case DXGI_FORMAT_BC1_UNORM:             return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            case DXGI_FORMAT_BC1_UNORM_SRGB:        return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            case DXGI_FORMAT_BC2_UNORM:             return VK_FORMAT_BC2_UNORM_BLOCK;
            case DXGI_FORMAT_BC2_UNORM_SRGB:        return VK_FORMAT_BC2_SRGB_BLOCK;
            case DXGI_FORMAT_BC3_UNORM:             return VK_FORMAT_BC3_UNORM_BLOCK;
            case DXGI_FORMAT_BC3_UNORM_SRGB:        return VK_FORMAT_BC3_SRGB_BLOCK;
            case DXGI_FORMAT_BC4_UNORM:             return VK_FORMAT_BC4_UNORM_BLOCK;
            case DXGI_FORMAT_BC4_SNORM:             return VK_FORMAT_BC4_UNORM_BLOCK;
            case DXGI_FORMAT_BC5_UNORM:             return VK_FORMAT_BC5_UNORM_BLOCK;
            case DXGI_FORMAT_BC5_SNORM:             return VK_FORMAT_BC5_UNORM_BLOCK;
            case DXGI_FORMAT_B5G6R5_UNORM:          return VK_FORMAT_B5G6R5_UNORM_PACK16;
            case DXGI_FORMAT_B5G5R5A1_UNORM:        return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            case DXGI_FORMAT_BC6H_UF16:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case DXGI_FORMAT_BC6H_SF16:             return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case DXGI_FORMAT_BC7_UNORM:             return VK_FORMAT_BC7_UNORM_BLOCK;
            case DXGI_FORMAT_BC7_UNORM_SRGB:        return VK_FORMAT_BC7_SRGB_BLOCK;
            case DXGI_FORMAT_R10G10B10A2_UNORM:     return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
            case DXGI_FORMAT_R16G16B16A16_FLOAT:    return VK_FORMAT_R16G16B16A16_SFLOAT;
            case DXGI_FORMAT_R32G32B32A32_FLOAT:    return VK_FORMAT_R32G32B32A32_SFLOAT;
            case DXGI_FORMAT_A8_UNORM:              return VK_FORMAT_R8_UNORM;
            default: assert(false);                 return VK_FORMAT_UNDEFINED;
        }
    }
}