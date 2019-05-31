#ifndef __FPGA_TIMER_H__
#define __FPGA_TIMER_H__

#define FPGA_TIMER_INDEX(x, index)	(x<<index)	

#define FPGA_REG_TIMER_MASK	0x0f

#if 0
#define FPGA_REG_TIMER_APR_OFFSET	0x58
#define FPGA_REG_TIMER_PRD_OFFSET	0x5c
#define FPGA_REG_TIMER_STEPS_OFFSET	0x60

#else
#define FPGA_REG_TIMER_EN		0x0000

#define FPGA_REG_TIMER_APR_OFFSET	0x08
#define FPGA_REG_TIMER_PRD_OFFSET	0x0c
#define FPGA_REG_TIMER_STEPS_OFFSET	0x10

#endif
#define FPGA_REG_TIMER_SIZE	(3*sizeof(uint32_t))

#define TIME_VALUE_TO_TICKS(time_value, clock_period)	(time_value/clock_period)	//unit of time_value :ns

#define FPGA_TIMER_PRELOAD_MAX	0xffff
#define FPGA_TIMER_VALUE_MAX(clock_period)	(FPGA_TIMER_PRELOAD_MAX*clock_period)	//ns

struct fpga_timer_resource {
	void *mmio_base;		// registers base address
	int timer_index;
	int clock_period;		// motor unit clock period (in nanoseconds)
	uint32_t fpga_irq_mask;	
	int fabric_irq;
	uint32_t preload_max;
};

struct fpga_timer{
	struct fpga_timer_resource *resource;
	const struct fpga_timer_ops *ops;
};
struct fpga_timer_ops {
	int	(*init)(struct fpga_timer *ptimer);
	int	(*apr_set)(struct fpga_timer *ptimer, uint16_t val);

	int	(*int_period_set)(struct fpga_timer *ptimer, uint16_t period);
	int	(*period_steps_get)(struct fpga_timer *ptimer, uint16_t *psteps);
	void	(*enable)(struct fpga_timer *ptimer);
	void	(*disable)(struct fpga_timer *ptimer);
	void	(*int_enable)(struct fpga_timer *ptimer);

	void	(*int_disable)(struct fpga_timer *ptimer);        
};


extern int fpga_timer_install(struct fpga_timer *ptimer);

#endif
