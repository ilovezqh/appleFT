#ifndef __BSP_TIMER_CAPTURE_H
#define __BSP_TIMER_CAPTURE_H

void pwm_measure_start(void);
void pwm_measure_reset(void);
u32 pwm_measure_stop(void);
    
void tick_init(void);
void set_tick(u32 us);
void wait_tick(void);
void reset_tick(void);

void tos_sem_create(uint32_t *var, uint32_t val);
void tos_knl_sched_lock(void);
void tos_knl_sched_unlock(void);

#endif
