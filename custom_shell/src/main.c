/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>

static int cmd_hello_world(const struct shell *shell, size_t argc,
			       char **argv)
{
    shell_print(shell, "Hello World!\n");
    return 0;
}

static int cmd_zb_forget_device(const struct shell *shell, size_t argc,
			       char **argv)
{
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	int c = a + b;
	shell_print(shell, "%d + %d = %d", a, b, c);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_custom_shell,
    SHELL_CMD(hello, NULL, "Print 'Hello World'", cmd_hello_world),
    SHELL_CMD_ARG(add_number, NULL, "Add two numbers", cmd_zb_forget_device, 3, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(custom_shell, &sub_custom_shell, "Custom Zigbee commands", NULL);

int main(void)
{
	printf("Starting custom shell example\nUse custom_shell for the custom commands\n");
	return 0;
}
