#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include <shell/shell.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(app);

#define BOARD_1 1
#define BOARD_2 2
#define BOARD_3 3

#define NUMBER_OF_DEVICES 3

#define ON 1
#define OFF 0 

struct device_config {
	int num_leds;
	int num_buttons;
	const struct gpio_dt_spec *button_spec;
	const struct gpio_dt_spec *led_spec;
};

#define DT_DRV_COMPAT nordic_gpio_device
#define MYDEV(idx) DT_NODELABEL(device ## idx)

int led_gpios_init(const struct device *dev)
{ 	

	const struct device_config *config = dev->config;
	
	int err = 0;
	if (!config->num_leds) {
		LOG_ERR("%s: no LEDs found (DT child nodes missing)", dev->name);
		err = -ENODEV;
	}

	for (size_t i = 0; (i < config->num_leds) && !err; i++) {

		const struct gpio_dt_spec *led_spec = &config->led_spec[i];

		if (device_is_ready(led_spec->port)) {
			err = gpio_pin_configure_dt(led_spec, GPIO_INPUT);

			if (err) {
				LOG_ERR("Cannot configure GPIO (err %d)", err);
			}
		} else {
			LOG_ERR("%s: GPIO device not ready", dev->name);
			err = -ENODEV;
		}
	}
	return err;
}


int button_gpios_init(const struct device *dev)
{ 	

	const struct device_config *config = dev->config;
	
	int err = 0;
	if (!config->num_buttons) {
		LOG_ERR("%s: no LEDs found (DT child nodes missing)", dev->name);
		err = -ENODEV;
	}

	for (size_t i = 0; (i < config->num_buttons) && !err; i++) {

		const struct gpio_dt_spec *button_spec = &config->button_spec[i];

		if (device_is_ready(button_spec->port)) {
			err = gpio_pin_configure_dt(button_spec, GPIO_OUTPUT);

			if (err) {
				LOG_ERR("Cannot configure GPIO (err %d)", err);
			}
			gpio_pin_set_dt(button_spec,OFF);
		} else {
			LOG_ERR("%s: GPIO device not ready", dev->name);
			err = -ENODEV;
		}
	}
	return err;
}


static int dev_init_function(const struct device *dev)
{
	int ret = 0;
	ret = led_gpios_init(dev);
	if (ret) {
		LOG_ERR("Could not init LED GPIOS");
		return -ENODEV;
	}
	ret = button_gpios_init(dev);
	if (ret) {
		LOG_ERR("Could not init button GPIOS");
		return -ENODEV;
	}
	return 0;
}


#define LED_GPIO_DT_SPEC(device_node_id,i) \
		GPIO_DT_SPEC_GET_BY_IDX(device_node_id, led_gpios,i)

#define BUTTON_GPIO_DT_SPEC(device_node_id,i) \
		GPIO_DT_SPEC_GET_BY_IDX(device_node_id, button_gpios,i)
                                 
#define CREATE_MY_DEVICE(idx)                                    	        \
	static const struct gpio_dt_spec led_gpio_dt_spec_##idx[] = {	        \
		LED_GPIO_DT_SPEC(MYDEV(idx),0),										\
		LED_GPIO_DT_SPEC(MYDEV(idx),1),										\
		LED_GPIO_DT_SPEC(MYDEV(idx),2),										\
		LED_GPIO_DT_SPEC(MYDEV(idx),3), 									\
	};																		\
	static const struct gpio_dt_spec button_gpio_dt_spec_##idx[] = {	    \
		BUTTON_GPIO_DT_SPEC(MYDEV(idx),0),                                  \
		BUTTON_GPIO_DT_SPEC(MYDEV(idx),1),                                  \
		BUTTON_GPIO_DT_SPEC(MYDEV(idx),2),                                  \
		BUTTON_GPIO_DT_SPEC(MYDEV(idx),3),                                  \
	};																		\
	 static const struct device_config device_config_##idx = {	   		    \
	.num_leds	  = ARRAY_SIZE(led_gpio_dt_spec_##idx),						\
	.led_spec	  = led_gpio_dt_spec_##idx,								    \
	.num_buttons  = ARRAY_SIZE(button_gpio_dt_spec_##idx),					\
	.button_spec  = button_gpio_dt_spec_##idx,								\
	};                                                				  		\
     DEVICE_DT_DEFINE(MYDEV(idx),                                     		\
                        dev_init_function,                        			\
                        NULL,                                      			\
                        NULL,                          			 			\
                        &device_config_##idx,                     			\
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,    \
                        NULL);

#if DT_NODE_HAS_STATUS(DT_NODELABEL(device1), okay)
CREATE_MY_DEVICE(1)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(device2), okay)
CREATE_MY_DEVICE(2)
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(device3), okay)
CREATE_MY_DEVICE(3)
#endif

void button_set(uint8_t device, uint8_t button_number)
{
	switch(device){
		case BOARD_1:
			gpio_pin_set_dt(&(device_config_1.button_spec[button_number-1]), ON);
			k_sleep(K_MSEC(5));
			gpio_pin_set_dt(&(device_config_1.button_spec[button_number-1]),OFF);
		break;
		case BOARD_2:
			gpio_pin_set_dt(&(device_config_2.button_spec[button_number-1]),ON);
			k_sleep(K_MSEC(5));
			gpio_pin_set_dt(&(device_config_2.button_spec[button_number-1]),OFF);
		break;
		case BOARD_3:
			gpio_pin_set_dt(&(device_config_3.button_spec[button_number-1]), ON);
			k_sleep(K_MSEC(5));
			gpio_pin_set_dt(&(device_config_3.button_spec[button_number-1]),OFF);
		break;
	}
}

int led_get(uint8_t device, uint8_t led_number)
{
	int val = -1;
	switch(device){
		case BOARD_1:
			val = gpio_pin_get_dt(&(device_config_1.led_spec[led_number-1]));
		break;
		case BOARD_2:
			val = gpio_pin_get_dt(&(device_config_2.led_spec[led_number-1]));
		break;
		case BOARD_3:
			val = gpio_pin_get_dt(&(device_config_3.led_spec[led_number-1]));
		break;
	}
	return val;
}


static int cmd_gpio_set(const struct shell *shell,
			size_t argc, char **argv)
{
	uint8_t device = atoi(argv[1]);
	uint8_t button = atoi(argv[2]);

	shell_print(shell, "Pressing button %d on device %d", button, device);
	button_set(device,button);

	return 0;
}



static int cmd_led_get(const struct shell *shell,
			size_t argc, char **argv)
{
	uint8_t device = atoi(argv[1]);
	uint8_t led = atoi(argv[2]);
	int val;

	shell_print(shell, "Reading LED %d on device %d", led, device);
	val = led_get(device,led);
	if(val<0) {
		LOG_ERR("Could not get LED status, err: %d", val);
	}
	LOG_INF("LED status: %d",val);

	return 0;
}

static void cmd_gpio_view(const struct shell *shell,
			size_t argc, char **argv)
{
	int led_status_list[4];
	ARG_UNUSED(argc);
    ARG_UNUSED(argv);
	for (int i=1; i<=NUMBER_OF_DEVICES; i++){
		for (int j=0; j<4; j++) {
			led_status_list[j] = led_get(i,j+1);
		}
		shell_print(shell, "---------------------");
		shell_print(shell, "   Device number %d", i);
		shell_print(shell, "     LED1   LED2");
		shell_print(shell, "       %d      %d", led_status_list[0], led_status_list[1]);
		shell_print(shell, "     LED3   LED4");
		shell_print(shell, "       %d      %d", led_status_list[2], led_status_list[3]);
		shell_print(shell, "---------------------");
		shell_print(shell, "");
	}
}



SHELL_STATIC_SUBCMD_SET_CREATE(sub_lb,
	SHELL_CMD_ARG(set, NULL, "Press button", cmd_gpio_set, 3, 0),
	SHELL_CMD_ARG(get, NULL, "Get LED state", cmd_led_get, 3, 0),
	SHELL_CMD(view, NULL, "View all DKs and LEDs", cmd_gpio_view),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(lb, &sub_lb, "GPIO commands", NULL);


void main(void)
{
}