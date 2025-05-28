/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>

int main(void)
{
	int i = 0;
	while(1) {
		printf("Hello World! %d\n", i++);
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
