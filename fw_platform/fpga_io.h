#ifndef __FPGA_IO_H__
#define __FPGA_IO_H__

#include <hw_reg_io.h>
#include <string.h>

#define readb(addr)			(*((volatile uint8_t* )(addr)))
#define readw(addr)			(*((volatile uint16_t* )(addr)))
#define readl(addr)			(*((volatile uint32_t* )(addr)))
#define writeb(value, addr)	(*((volatile uint8_t* )(addr))=(uint8_t )(value))
#define writew(value, addr)	(*((volatile uint16_t* )(addr))=(uint16_t )(value))
#define writel(value, addr)	(*((volatile uint32_t* )(addr))=(uint32_t )(value))

static __INLINE int32_t fpga_readl(uint32_t *value, const volatile void  *addr)
{
	*value = (readw(addr) & 0x0000ffff);
	return 0;
}

static __INLINE int32_t fpga_writel(uint32_t value, volatile void  *addr)
{
	writew(value, addr);
	return 0;
}

static __INLINE int32_t fpga_readnl(volatile void *addr, void *buffer, int32_t count)
{
	uint32_t *lptr = buffer;
	if (addr == NULL || buffer == NULL || count <= 0)
		return -1;
	while (count-- > 0) {
		*lptr++ = readl(addr);
		addr = (uint8_t *)addr + sizeof(uint32_t);
	}
	return 0;
}

static __INLINE  int32_t fpga_writenl(volatile void *addr, const void *buffer, int32_t count)
{
	uint32_t *lptr = (uint32_t *)buffer;

	if (addr == NULL || buffer == NULL || count <= 0)
		return -1;

	while (count-- > 0) {
		writel(*lptr++, addr);
		addr = (uint8_t *)addr + sizeof(uint32_t);
	}
	return 0;
}

static __INLINE int32_t fpga_readnw(const volatile void *addr, void *buffer, int32_t count)
{
	uint32_t *lptr = buffer;
	if (addr == NULL || buffer == NULL || count <= 0)
		return -1;

	while (count-- > 0) {

		*lptr++ = readw(addr);
		addr = (uint8_t *)addr + sizeof(uint16_t);
	}
	return 0;
}

static __INLINE int32_t fpga_writenw(volatile void *addr, const void *buffer, int32_t count)
{
	uint16_t *lptr = (uint16_t *)buffer;
	if (addr == NULL || buffer == NULL || count <= 0)
		return -1;
	while (count-- > 0) {
		writel(*lptr++, addr);
		addr = (uint8_t *)addr + sizeof(uint16_t);
	}
	return 0;
}

static __INLINE int32_t fpga_update_lbits(volatile void *addr, uint32_t mask, uint32_t value)
{
	uint32_t val = readl(addr);
	val = (val & ~mask) | (value & mask);
	writel(val, addr);
	return 0;
}

//============================================================

#endif
