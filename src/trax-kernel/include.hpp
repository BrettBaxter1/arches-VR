#pragma once
#include "stdafx.hpp"

#define TRAX_USE_RT_CORE 1
#define TRAX_USE_HARDWARE_INTERSECTORS 0
#define TRAX_KERNEL_ARGS_ADDRESS 256ull

#define USE_HECWBVH_V1 0

namespace rtm {
#if USE_HECWBVH_V1
typedef rtm::HECWBVH CWBVH;
#else
typedef rtm::HE2CWBVH CWBVH;
#endif
}

struct TRaXKernelArgs
{
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint32_t framebuffer_size;
	uint32_t* framebuffer;

	bool pregen_rays;

	rtm::Camera camera;
	rtm::vec3 light_dir;
	rtm::Ray* rays;
	rtm::CWBVH::Node* nodes;
	rtm::FTB* ft_blocks;

	//mesh data
	rtm::uvec3* vertex_indices;
	rtm::uvec3* normal_indices;
	rtm::uvec3* tex_coord_indices;

	rtm::vec3* vertices;
	rtm::vec3* normals;
	rtm::vec2* tex_coords;

	uint* material_indices;
	rtm::Material* materials;

	// Stereo rendering arguments.
	uint32_t eye_width;
	uint32_t eye_height;
	uint32_t eye_count;
	rtm::Camera cameras[2];
	rtm::vec2 projection_offsets[2];

	// Second-stage post-processing data. The stereo renderer writes framebuffer;
	// the post-processing kernel reads it and writes this buffer.
	uint32_t* postprocess_framebuffer;
	rtm::vec2 lens_centers[2];
	float lens_k1;
	float lens_k2;
	// Per-channel radial scale factors for chromatic aberration correction.
	// Green is the neutral channel; red and blue can use slightly different
	// radial mappings because the lens refracts wavelengths differently.
	float lens_chromatic_scale[3];

	// Final display dimensions. eye_width/eye_height remain the render
	// dimensions when overscan is enabled; the host crops to these dimensions.
	uint32_t display_width;
	uint32_t display_height;
};
