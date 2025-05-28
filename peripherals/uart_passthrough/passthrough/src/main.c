/*
 * Copyright (c) 2024 Argentum Systems Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>

#define RING_BUF_SIZE 1024

struct uart_bridge {
	const uint8_t * const name;
	const struct device *rx_dev;
	const struct device *tx_dev;
	struct ring_buf *rx_ring_buf;
};

RING_BUF_DECLARE(rb_console, RING_BUF_SIZE);
RING_BUF_DECLARE(rb_other, RING_BUF_SIZE);

static struct uart_bridge bridge_c2o = {
	.name = "c2o",
	.rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console)),
	.tx_dev = DEVICE_DT_GET(DT_CHOSEN(uart_passthrough)),
	.rx_ring_buf = &rb_console,
};

static struct uart_bridge bridge_o2c = {
	.name = "o2c",
	.rx_dev = DEVICE_DT_GET(DT_CHOSEN(uart_passthrough)),
	.tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console)),
	.rx_ring_buf = &rb_other,
};

static void uart_cb(const struct device *dev, void *ctx)
{
	struct uart_bridge *bridge = (struct uart_bridge *)ctx;

		if (uart_irq_update(bridge->rx_dev) && uart_irq_rx_ready(bridge->rx_dev)) {
		uint32_t err = uart_err_check(bridge->rx_dev);
		if (err) {
			printk("<<%s: UART error: 0x%x>>\n", bridge->name, err);
			// Continue even on error
		}
	
		uint8_t *buf;
		uint32_t len = ring_buf_put_claim(bridge->rx_ring_buf, &buf, RING_BUF_SIZE);
		if (len == 0) {
			printk("<<%s: Ring buffer overflow>>\n", bridge->name);
		} else {
			int bytes = uart_fifo_read(bridge->rx_dev, buf, len);
			if (bytes > 0) {
				ring_buf_put_finish(bridge->rx_ring_buf, bytes);
				uart_irq_tx_enable(bridge->tx_dev);
			}
		}
	}
	
	if (uart_irq_tx_ready(bridge->tx_dev)) {
		uint8_t *buf;
		uint32_t len = ring_buf_get_claim(bridge->rx_ring_buf, &buf, RING_BUF_SIZE);
		if (len > 0) {
			int written = uart_fifo_fill(bridge->tx_dev, buf, len);
			if (written > 0) {
			ring_buf_get_finish(bridge->rx_ring_buf, written);
		} else {
				uart_irq_tx_disable(bridge->tx_dev);
			}
		} else {
			uart_irq_tx_disable(bridge->tx_dev);
		}
	}
}

int main(void)
{
	printk("Console Device: %p\n", bridge_c2o.rx_dev);
	printk("Other Device:   %p\n", bridge_o2c.rx_dev);

	uart_irq_callback_user_data_set(bridge_c2o.rx_dev, uart_cb, &bridge_c2o);
	uart_irq_callback_user_data_set(bridge_o2c.rx_dev, uart_cb, &bridge_o2c);

	uart_irq_rx_enable(bridge_c2o.rx_dev);
	uart_irq_rx_enable(bridge_o2c.rx_dev);

	while (1) {
		k_sleep(K_FOREVER);
	}
	return 0;
}
