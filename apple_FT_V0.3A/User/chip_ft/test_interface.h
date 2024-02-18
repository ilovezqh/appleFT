#ifndef __TEST_INTERFACE_H
#define __TEST_INTERFACE_H
 
#define VOUT_EN_PORT        GPIOD
#define VOUT_EN_PIN         GPIO_Pin_15
 
struct  test_power_ops  //test interface Operations
{
    void (*init)(void);
    void (*on)(void);
    void (*off)(void);
    void (*consumption_on)(void);
    void (*consumption_off)(void);
};
extern struct test_power_ops test_power;

#endif
