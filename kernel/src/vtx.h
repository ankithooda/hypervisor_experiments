#include <stdint.h>

// VT-x //
static inline uint64_t getcr4(void)
{
	register uint64_t ret = 0;

	asm volatile("movq %%cr4, %0\n" : "=r"(ret));

	return ret;
}

static inline void setcr4(register uint64_t val)
{
	asm volatile("movq %0, %%cr4\n" : : "r"(val));
}

static inline void vmxon()
{
	asm volatile("vmxon");
}
