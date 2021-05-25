/* FORMATS.hpp
 *   by Lut99
 *
 * Created:
 *   25/05/2021, 15:53:47
 * Last edited:
 *   25/05/2021, 16:05:20
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Provides a map for converting VkFormat enums to a string
 *   representation of that format.
**/

#ifndef COMPUTE_FORMATS_HPP
#define COMPUTE_FORMATS_HPP

#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>

/* Simple macro that sets the given enum to its string representation. */
#define MAP_VK_FORMAT(FORMAT) \
    { (FORMAT), (#FORMAT) }

namespace RayTracer::Compute {
    /* Static map of VkFormats to their respective string representations. */
    static std::unordered_map<VkFormat, std::string> vk_format_map({
        MAP_VK_FORMAT(VK_FORMAT_UNDEFINED),
        MAP_VK_FORMAT(VK_FORMAT_R4G4_UNORM_PACK8),
        MAP_VK_FORMAT(VK_FORMAT_R4G4B4A4_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_B4G4R4A4_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R5G6B5_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_B5G6R5_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R5G5B5A1_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_B5G5R5A1_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_A1R5G5B5_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R8_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R8_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R8_SRGB),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R8G8_SRGB),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8_SRGB),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_SINT),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8_SRGB),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R8G8B8A8_SRGB),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_SINT),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8A8_SRGB),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_UNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_SNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_USCALED_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_SSCALED_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_UINT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_SINT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A8B8G8R8_SRGB_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2R10G10B10_UNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2R10G10B10_SNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2R10G10B10_USCALED_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2R10G10B10_SSCALED_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2R10G10B10_UINT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2R10G10B10_SINT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2B10G10R10_UNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2B10G10R10_SNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2B10G10R10_USCALED_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2B10G10R10_SSCALED_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2B10G10R10_UINT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_A2B10G10R10_SINT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_R16_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R16_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R16_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_SNORM),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_USCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_SSCALED),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R16G16B16A16_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R32_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R32_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R32_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32B32_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32B32_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32B32_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32B32A32_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32B32A32_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R32G32B32A32_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R64_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R64_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R64_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64B64_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64B64_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64B64_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64B64A64_UINT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64B64A64_SINT),
        MAP_VK_FORMAT(VK_FORMAT_R64G64B64A64_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_B10G11R11_UFLOAT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_D16_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_X8_D24_UNORM_PACK32),
        MAP_VK_FORMAT(VK_FORMAT_D32_SFLOAT),
        MAP_VK_FORMAT(VK_FORMAT_S8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_D16_UNORM_S8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_D24_UNORM_S8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_D32_SFLOAT_S8_UINT),
        MAP_VK_FORMAT(VK_FORMAT_BC1_RGB_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC1_RGB_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC1_RGBA_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC1_RGBA_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC2_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC2_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC3_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC3_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC4_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC4_SNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC5_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC5_SNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC6H_UFLOAT_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC6H_SFLOAT_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC7_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_BC7_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_EAC_R11_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_EAC_R11_SNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_EAC_R11G11_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_EAC_R11G11_SNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_4x4_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_4x4_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_5x4_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_5x4_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_5x5_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_5x5_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_6x5_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_6x5_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_6x6_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_6x6_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x5_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x5_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x6_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x6_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x8_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x5_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x8_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x5_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x5_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x6_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x6_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x8_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x8_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x10_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x10_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_12x10_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_12x10_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_12x12_UNORM_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_12x12_SRGB_BLOCK),
        MAP_VK_FORMAT(VK_FORMAT_G8B8G8R8_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_B8G8R8G8_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_R10X6_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R10X6G10X6_UNORM_2PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16),
        MAP_VK_FORMAT(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R12X4_UNORM_PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R12X4G12X4_UNORM_2PACK16),
        MAP_VK_FORMAT(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16),
        MAP_VK_FORMAT(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16),
        MAP_VK_FORMAT(VK_FORMAT_G16B16G16R16_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_B16G16R16G16_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT),
        MAP_VK_FORMAT(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT),
        MAP_VK_FORMAT(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT),
        MAP_VK_FORMAT(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT),
        MAP_VK_FORMAT(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT),
        MAP_VK_FORMAT(VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT),
        MAP_VK_FORMAT(VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT)
    });
}

#endif
