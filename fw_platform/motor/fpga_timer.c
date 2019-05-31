#include "fpga_io.h"
#include "fpga_timer.h"

static int fpga_timer_apr_set(struct fpga_timer *ptimer, uint16_t val)
{
	int ret;
	uint32_t addr;
	
	struct fpga_timer_resource *ptimer_rc;

	//printf("fpga_timer_apr_set1\n\r");
	if (!ptimer)
		return -1;
	if (!ptimer->resource)
		return -1;
	
 	ptimer_rc =ptimer->resource;

	//printf("2\n\r");
	addr = (uint32_t)ptimer_rc->mmio_base + FPGA_REG_TIMER_APR_OFFSET+ptimer_rc->timer_index*FPGA_REG_TIMER_SIZE;
	ret = fpga_writel(val, (void *)addr); 
	//printf("3\n\r");
	return ret;
}

static int fpga_timer_int_period_set(struct fpga_timer *ptimer, uint16_t period)
{
	int ret;
	uint32_t addr;
	
	struct fpga_timer_resource *ptimer_rc;
	
	if (!ptimer)
		return -1;
	if (!ptimer->resource)
		return -1;
	
 	ptimer_rc =ptimer->resource;

	addr = (uint32_t)ptimer_rc->mmio_base + FPGA_REG_TIMER_PRD_OFFSET+ptimer_rc->timer_index*FPGA_REG_TIMER_SIZE;
	ret = fpga_writel(period, (void *)addr); 
	return ret;
}

static int fpga_timer_period_steps_get(struct fpga_timer *ptimer, uint16_t *psteps)
{
	int ret;
	uint32_t steps, addr;
	struct fpga_timer_resource *ptimer_rc;
	
	if (!ptimer)
		return -1;
	if (!ptimer->resource)
		return -1;
	ptimer_rc =ptimer->resource;

	addr = (uint32_t)ptimer_rc->mmio_base + FPGA_REG_TIMER_PRD_OFFSET+ptimer_rc->timer_index*FPGA_REG_TIMER_SIZE;
	ret = fpga_readl(&steps, (void *)addr); 
	*psteps = (uint16_t)steps;
	return ret;
}

static void fpga_timer_enable(struct fpga_timer *ptimer)
{
	struct fpga_timer_resource *ptimer_rc;
	
 	ptimer_rc =ptimer->resource;

	fpga_update_lbits((char *)ptimer_rc->mmio_base + FPGA_REG_TIMER_EN, FPGA_TIMER_INDEX(1, ptimer_rc->timer_index), FPGA_TIMER_INDEX(1, ptimer_rc->timer_index));
}

static void fpga_timer_disable(struct fpga_timer *ptimer)
{
	struct fpga_timer_resource *ptimer_rc;
	
 	ptimer_rc =ptimer->resource;

	fpga_update_lbits((char *)ptimer_rc->mmio_base + FPGA_REG_TIMER_EN, FPGA_TIMER_INDEX(1, ptimer_rc->timer_index), FPGA_TIMER_INDEX(0, ptimer_rc->timer_index));
}

static void fpga_timer_int_enable(struct fpga_timer *ptimer)
{
	struct fpga_timer_resource *ptimer_rc;
	
 	ptimer_rc =ptimer->resource;
	fpga_enable_interrupt(ptimer_rc->fpga_irq_mask);  
	
	//fpga_update_lbits((char *)ptimer_rc->mmio_base + FPGA_REG_TIMER_INT_EN, FPGA_TIMER_INDEX(1, ptimer_rc->timer_index), FPGA_TIMER_INDEX(1, ptimer_rc->timer_index));
}

static void fpga_timer_int_disable(struct fpga_timer *ptimer)
{
	struct fpga_timer_resource *ptimer_rc;
	
 	ptimer_rc =ptimer->resource;
	fpga_disable_interrupt(ptimer_rc->fpga_irq_mask); 
	//fpga_update_lbits((char *)ptimer_rc->mmio_base + FPGA_REG_TIMER_INT_EN, FPGA_TIMER_INDEX(1, ptimer_rc->timer_index), FPGA_TIMER_INDEX(0, ptimer_rc->timer_index));
}

static int fpga_timer_init(struct fpga_timer *ptimer)
{
	int ret;
	
	if (!ptimer)
		return -1;
	
	ret = fpga_timer_int_period_set(ptimer, 0);
	fpga_timer_int_disable(ptimer);
	fpga_timer_disable(ptimer);
	
	return ret;
}

static struct fpga_timer_ops fpga_timer_ops = {

	.init = fpga_timer_init,
	.apr_set = fpga_timer_apr_set,
	.int_period_set = fpga_timer_int_period_set,
	.period_steps_get = fpga_timer_period_steps_get,
	.enable = fpga_timer_enable,
	.disable = fpga_timer_disable,
	.int_enable = fpga_timer_int_enable,
	.int_disable = fpga_timer_int_disable

};

int fpga_timer_install(struct fpga_timer *ptimer)
{	
	if (!ptimer)
		return -1;
	if (!ptimer->resource)
		return -1;
	
	ptimer->ops = &fpga_timer_ops;
	ptimer->ops->init(ptimer);
	return 0;
}


int fpga_timer_drvinit(void)
{
  	return 0;
}

