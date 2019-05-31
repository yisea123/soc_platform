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
	int i;
	uint32_t tmp_val;

	for(i=0; i<5; i++)
	{
		writew(value, addr);
		fpga_readl(&tmp_val, addr);
		if(tmp_val==value)
			break;
	}
	if(i==5)
		printf("fpga_writel error!addr=0x%x w_value=0x%x r_value=%x\n\r", addr, value, tmp_val);
	
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
	int i;
	uint32_t tmp_val;
	
	val = (val & ~mask) | (value & mask);

	for(i=0; i<5; i++)
	{
		writel(val, addr);
		fpga_readl(&tmp_val, addr);
		if(tmp_val==val)
			break;
	}
	if(i==5)
		printf("fpga_update_lbits error!addr=0x%x w_value=0x%x r_value=%x\n\r", addr, val, tmp_val);

	return 0;
}

//============================================================

#endif
