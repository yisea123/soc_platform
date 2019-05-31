#ifndef __DCMOTOR_H__
#define __DCMOTOR_H__

#include <stddef.h>
#include <stdint.h>
#include <motor.h>


/* definition of dcmotor status */
#define DCMOTOR_RUNNING				(1 << 0)
#define DCMOTOR_ERROR				(1 << 16)

/* macros to test dcmotor status */
#define dcmotor_is_running(s)			((s) & DCMOTOR_RUNNING)
#define dcmotor_is_error(s)			((s) & DCMOTOR_ERROR)


struct dcmotor_config {
	motion_dir dir;				// motor motion direction
	uint32_t time_to_run;			// uint:ms
};

struct dcmotor_feature{
	int feature_flag;			// feature flags
};

/*
 * struct dcmotor
 *
 * One for each dcmotor device.
 */
struct dcmotor {
	const void *resource;			// pointer to the resource block of a dcmotor instance
	int (*const install)(struct dcmotor *motor);	// pointer to the device installation function 
	struct dcmotor_feature feature;
	int status;
	struct dcmotor_config config;		// copy of current motor motion configuration
	const struct dcmotor_ops *ops;
	void (*callback)(struct dcmotor *motor, struct callback_data *data);	// callback handling dcmotor events after a motor interrupt
	struct callback_data callbackdata;
};


/*
 * struct dcmotor_ops - dcmotor operations
 * @config: set configure of this dcmotor
 * @status: get status of this dcmotor
 * @start: start running this dcmotor
 * @stop: stop running this dcmotor 
 */ 
struct dcmotor_ops {
	int	(*config)(struct dcmotor *motor, const struct dcmotor_config *config);
	int	(*status)(struct dcmotor *motor);

	int	(*start)(struct dcmotor *motor);
	void	(*stop)(struct dcmotor *motor);
	void	(*reset)(struct dcmotor *motor);
};


/* declaration of dcmotor list */
extern struct dcmotor * dcmotor_list[];
extern const int dcmotor_num;

/* function prototypes of dcmotor driver */
int dcmotor_install_devices(void);

struct dcmotor *dcmotor_get(int index);

int dcmotor_set_callback(struct dcmotor *motor, void (*callback)(struct dcmotor *, struct callback_data *), struct callback_data *data);

int dcmotor_start(struct dcmotor *motor);
void dcmotor_stop(struct dcmotor *motor);
int dcmotor_status(struct dcmotor *motor);

int dcmotor_get_config(struct dcmotor *sensor, struct dcmotor_config *config);
int dcmotor_set_config(struct dcmotor *sensor, const struct dcmotor_config *config);


#endif /* __DCMOTOR_H__ */
