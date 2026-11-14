#include "../trax-kernel/include.hpp"
#include "../trax-kernel/custom-instr.hpp"

// This is the second-stage kernel scaffold. It currently applies a deliberately
// strong red tint so the two-kernel sequencing is visually obvious before
// adding radial lens distortion.
inline static void kernel(const TRaXKernelArgs& args)
{
	for(uint index = fchthrd(); index < args.framebuffer_size; index = fchthrd())
	{
		const uint32_t pixel = args.framebuffer[index];
		const uint32_t red = pixel & 0xffu;
		const uint32_t green = (pixel >> 8) & 0xffu;
		const uint32_t blue = (pixel >> 16) & 0xffu;
		const uint32_t alpha = pixel & 0xff000000u;

		// Preserve scene structure while making the post-process stage obvious.
		const uint32_t tinted_red = red + (255u - red) / 2u;
		args.postprocess_framebuffer[index] =
			tinted_red | ((green / 4u) << 8) | ((blue / 4u) << 16) | alpha;
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
