/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
 * @brief Zigbee application template.
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>

#include <zboss_api.h>
#include <zigbee/zigbee_error_handler.h>
#include <zigbee/zigbee_app_utils.h>
#include <zb_nrf_platform.h>
#include <zb_multi_sensor.h>

/* Device endpoint, used to receive ZCL commands. */
#define MULTI_SENSOR_ENDPOINT 				12
#define MULTI_SENSOR_DEV                 	"SENSOR_SIM"

/* Type of power sources available for the device.
 * For possible values see section 3.2.2.2.8 of ZCL specification.
 */
#define TEMPLATE_INIT_BASIC_POWER_SOURCE    ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE

/* LED indicating that device successfully joined Zigbee network. */
#define ZIGBEE_NETWORK_STATE_LED            DK_LED3

/* LED used for device identification. */
#define IDENTIFY_LED                        DK_LED4

/* Button used to enter the Identify mode. */
#define IDENTIFY_MODE_BUTTON                DK_BTN4_MSK


LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* Main application customizable context.
 * Stores all settings and static values.
 */
struct zb_device_ctx {
	zb_zcl_basic_attrs_t                 basic_attr;
	zb_zcl_identify_attrs_t              identify_attr;
    zb_zcl_temp_measurement_attrs_t      temp_attr;
    zb_zcl_pressure_measurement_attrs_t  pressure_attr;
};

/* Zigbee device application context storage. */
static struct zb_device_ctx dev_ctx;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(
	identify_attr_list,
	&dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST(
	basic_attr_list,
	&dev_ctx.basic_attr.zcl_version,
	&dev_ctx.basic_attr.power_source);

ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST(
    temp_measurement_attr_list,
    &dev_ctx.temp_attr.measure_value,
    &dev_ctx.temp_attr.min_measure_value,
    &dev_ctx.temp_attr.max_measure_value,
    &dev_ctx.temp_attr.tolerance);

ZB_ZCL_DECLARE_PRESSURE_MEASUREMENT_ATTRIB_LIST(
    pressure_measurement_attr_list,
    &dev_ctx.pressure_attr.measure_value,
    &dev_ctx.pressure_attr.min_measure_value,
    &dev_ctx.pressure_attr.max_measure_value,
    &dev_ctx.pressure_attr.tolerance);

ZB_DECLARE_MULTI_SENSOR_CLUSTER_LIST(
    multi_sensor_clusters,
    basic_attr_list,
    identify_attr_list,
    temp_measurement_attr_list,
    pressure_measurement_attr_list);

ZB_HA_DECLARE_MULTI_SENSOR_EP(multi_sensor_ep, MULTI_SENSOR_ENDPOINT, multi_sensor_clusters);

ZB_HA_DECLARE_MULTI_SENSOR_CTX(multi_sensor_ctx, multi_sensor_ep);


/**@brief Function for initializing all clusters attributes. */
static void multi_sensor_clusters_attr_init(void)
{
	/* Basic cluster attributes data */
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.power_source = TEMPLATE_INIT_BASIC_POWER_SOURCE;

	/* Identify cluster attributes data. */
	dev_ctx.identify_attr.identify_time =
		ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

    dev_ctx.temp_attr.measure_value     = ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN;
    dev_ctx.temp_attr.min_measure_value = ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_MIN_VALUE;
    dev_ctx.temp_attr.max_measure_value = ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_MAX_VALUE;
    dev_ctx.temp_attr.tolerance         = ZB_ZCL_ATTR_TEMP_MEASUREMENT_TOLERANCE_MAX_VALUE;

    dev_ctx.pressure_attr.measure_value     = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_UNKNOWN;
    dev_ctx.pressure_attr.min_measure_value = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MIN_VALUE_MIN_VALUE;
    dev_ctx.pressure_attr.max_measure_value = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MAX_VALUE_MAX_VALUE;
    dev_ctx.pressure_attr.tolerance         = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_TOLERANCE_MAX_VALUE;
}


static void update_attributes_values()
{
    ZB_ZCL_SET_ATTRIBUTE(
        MULTI_SENSOR_ENDPOINT,
        ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.temp_attr.measure_value,
		NULL);

	ZB_ZCL_SET_ATTRIBUTE(
        MULTI_SENSOR_ENDPOINT,
        ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID,
		(zb_uint8_t *)&dev_ctx.pressure_attr.measure_value,
		NULL);
}

static void process_sample(const struct device *dev)
{
	struct sensor_value temp, pressure;

    if (sensor_sample_fetch(dev) < 0) {
		LOG_ERR("Sensor sample update error");
		return;
	}

    if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
		LOG_ERR("Cannot read sensor temperature channel");
		return;
	}
    if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &pressure) < 0) {
		LOG_ERR("Cannot read sensor pressure channel");
		return;
	}
    dev_ctx.temp_attr.measure_value = sensor_value_to_double(&temp);
	dev_ctx.pressure_attr.measure_value = sensor_value_to_double(&pressure);
}


static void sensor_handler(const struct device *dev,
			    const struct sensor_trigger *trig)
{
	process_sample(dev);
	update_attributes_values();
}


static void sensor_init()
{
	const struct device *dev = device_get_binding(MULTI_SENSOR_DEV);

    if (dev == NULL) {
    	LOG_ERR("Could not get sensor device");
    	return;
    }

    struct sensor_trigger trig = {
    		.type = SENSOR_TRIG_DATA_READY,
    		.chan = SENSOR_CHAN_ALL,
    };
        if (sensor_trigger_set(dev, &trig, sensor_handler) < 0) {
    		LOG_ERR("Cannot configure trigger");
    		return;
    	}
}

/**@brief Function to toggle the identify LED
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void toggle_identify_led(zb_bufid_t bufid)
{
	static int blink_status;

	dk_set_led(IDENTIFY_LED, (++blink_status) % 2);
	ZB_SCHEDULE_APP_ALARM(toggle_identify_led, bufid, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
}

/**@brief Function to handle identify notification events on the first endpoint.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void identify_cb(zb_bufid_t bufid)
{
	zb_ret_t zb_err_code;

	if (bufid) {
		/* Schedule a self-scheduling function that will toggle the LED */
		ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
	} else {
		/* Cancel the toggling function alarm and turn off LED */
		zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
		ZVUNUSED(zb_err_code);

		dk_set_led(IDENTIFY_LED, 0);
	}
}

/**@breif Starts identifying the device.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void start_identifying(zb_bufid_t bufid)
{
	zb_ret_t zb_err_code;

	ZVUNUSED(bufid);

	/* Check if endpoint is in identifying mode,
	 * if not put desired endpoint in identifying mode.
	 */
	if (dev_ctx.identify_attr.identify_time ==
	    ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {
		LOG_INF("Enter identify mode");
		zb_err_code = zb_bdb_finding_binding_target(
			MULTI_SENSOR_ENDPOINT);
		ZB_ERROR_CHECK(zb_err_code);
	} else {
		LOG_INF("Cancel identify mode");
		zb_bdb_finding_binding_target_cancel();
	}
}

/**@brief Callback for button events.
 *
 * @param[in]   button_state  Bitmask containing buttons state.
 * @param[in]   has_changed   Bitmask containing buttons
 *                            that have changed their state.
 */
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	/* Calculate bitmask of buttons that are pressed
	 * and have changed their state.
	 */
	uint32_t buttons = button_state & has_changed;

	if (buttons & IDENTIFY_MODE_BUTTON) {
		ZB_SCHEDULE_APP_CALLBACK(start_identifying, 0);
	}
}

/**@brief Function for initializing LEDs and Buttons. */
static void configure_gpio(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}
}

/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
	/* Update network status LED. */
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);

	/* No application-specific behavior is required.
	 * Call default signal handler.
	 */
	ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

	/* All callbacks should either reuse or free passed buffers.
	 * If bufid == 0, the buffer is invalid (not passed).
	 */
	if (bufid) {
		zb_buf_free(bufid);
	}
}

void main(void)
{
	LOG_INF("Starting Zigbee application template example");

	/* Initialize */
	configure_gpio();

	/* Register device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&multi_sensor_ctx);

	multi_sensor_clusters_attr_init();
	sensor_init();

	/* Register handlers to identify notifications */
	ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(MULTI_SENSOR_ENDPOINT, identify_cb);

	/* Start Zigbee default thread */
	zigbee_enable();

	LOG_INF("Zigbee application template started");

}
