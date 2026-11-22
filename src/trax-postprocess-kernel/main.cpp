#include "../trax-kernel/include.hpp"
#include "../trax-kernel/custom-instr.hpp"

inline static float bilinear_channel(const uint32_t* framebuffer, uint width, uint height, float x, float y, uint channel)
{
	const uint x0 = (uint)x;
	const uint y0 = (uint)y;
	const uint x1 = x0 + 1u < width ? x0 + 1u : x0;
	const uint y1 = y0 + 1u < height ? y0 + 1u : y0;
	const float tx = x - (float)x0;
	const float ty = y - (float)y0;

	const uint32_t p00 = framebuffer[y0 * width + x0];
	const uint32_t p10 = framebuffer[y0 * width + x1];
	const uint32_t p01 = framebuffer[y1 * width + x0];
	const uint32_t p11 = framebuffer[y1 * width + x1];

	const uint shift = channel * 8u;
	const float c00 = (float)((p00 >> shift) & 0xffu);
	const float c10 = (float)((p10 >> shift) & 0xffu);
	const float c01 = (float)((p01 >> shift) & 0xffu);
	const float c11 = (float)((p11 >> shift) & 0xffu);

	const float c0 = c00 + (c10 - c00) * tx;
	const float c1 = c01 + (c11 - c01) * tx;
	return c0 + (c1 - c0) * ty;
}

inline static uint32_t sample_channel(const uint32_t* framebuffer, uint width, uint height,
	float x, float y, uint channel)
{
	if(x < 0.0f || x > (float)(width - 1u) || y < 0.0f || y > (float)(height - 1u))
		return 0u;
	return (uint32_t)(bilinear_channel(framebuffer, width, height, x, y, channel) + 0.5f);
}

inline static uint32_t sample_foveated_channel(const TRaXKernelArgs& args,
	const uint32_t* framebuffer, float x, float y, uint channel)
{
	if(x < 0.0f || x > (float)(args.eye_width - 1u) ||
		y < 0.0f || y > (float)(args.eye_height - 1u))
		return 0u;

	const uint source_x = (uint)x;
	const uint source_y = (uint)y;
	const uint rate = foveation_rate(args, source_x, source_y);
	if(rate == 1u)
		return sample_channel(framebuffer, args.eye_width, args.eye_height, x, y, channel);

	uint sample_x;
	uint sample_y;
	foveation_source_pixel(args, source_x, source_y, sample_x, sample_y);
	const uint shift = channel * 8u;
	return (framebuffer[sample_y * args.eye_width + sample_x] >> shift) & 0xffu;
}

inline static void kernel(const TRaXKernelArgs& args)
{
	const uint pixels_per_eye = args.eye_width * args.eye_height;

	for(uint index = fchthrd(); index < args.framebuffer_size; index = fchthrd())
	{
		const uint eye = index / pixels_per_eye;
		const uint pixel = index % pixels_per_eye;
		const uint x = pixel % args.eye_width;
		const uint y = pixel / args.eye_width;

		// Map each destination pixel back into its eye's source image. This is
		// inverse radial pre-distortion, so every output pixel is written once.
		const rtm::vec2 center = args.lens_centers[eye];
		const float u = ((float)x + 0.5f) / (float)args.eye_width;
		const float v = ((float)y + 0.5f) / (float)args.eye_height;
		const float dx = (u - center.x) * 2.0f;
		const float dy = (v - center.y) * 2.0f;
		const float r2 = dx * dx + dy * dy;
		const uint source_index = eye * pixels_per_eye;
		const uint32_t* source_framebuffer = args.framebuffer + source_index;
		uint32_t channels[3];
		for(uint channel = 0; channel < 3u; ++channel)
		{
			const float channel_k1 = args.lens_k1 * args.lens_chromatic_scale[channel];
			const float channel_k2 = args.lens_k2 * args.lens_chromatic_scale[channel];
			const float radial = 1.0f + channel_k1 * r2 + channel_k2 * r2 * r2;
			const float source_u = center.x + dx * radial * 0.5f;
			const float source_v = center.y + dy * radial * 0.5f;
			const float source_x = source_u * (float)args.eye_width - 0.5f;
			const float source_y = source_v * (float)args.eye_height - 0.5f;
			channels[channel] = sample_foveated_channel(args, source_framebuffer,
				source_x, source_y, channel);
		}

		const uint32_t output = channels[0] | (channels[1] << 8) |
			(channels[2] << 16) | 0xff000000u;

		args.postprocess_framebuffer[index] = output;
	}
}

#ifdef __riscv
int main()
{
	kernel(*(const TRaXKernelArgs*)TRAX_KERNEL_ARGS_ADDRESS);
	return 0;
}
#else
int main()
{
	return 0;
}
#endif
