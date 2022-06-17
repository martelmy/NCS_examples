#include <string.h>
#include <shell/shell.h>
#include <zboss_api.h>
#include <zigbee/zigbee_error_handler.h>
#include <zb_nrf_platform.h>
#include "zigbee_cli.h"
#include "zigbee_cli_utils.h"


/**@brief Stop bdb top level commissioning process.
 *
 * @code
 * > zb_custom stop
 * Commissioning stopped
 * Done
 * @endcode
 * 
 * @pre Only after @ref start "bdb start".
 * 
 * Will stop cinnussioning by broadcasting a Mgmt_Permit_Joining_req with PermitDuration of 0.
 * See Base Device Behaviour specification for details.
 */

static int cmd_zb_stop(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	zb_bool_t ret = ZB_TRUE;

    if ((!zigbee_is_stack_started())) {
        zb_cli_print_error(shell, "Stack is not started",
					   ZB_FALSE);
		return -ENOEXEC;
    }

    ret = zb_bdb_close_network(NULL);
    if (ret != RET_OK)
    {
		zb_cli_print_error(shell, "Cannot stop commissioning", ZB_FALSE);
	}
}


SHELL_STATIC_SUBCMD_SET_CREATE(sub_zb_custom,
	SHELL_CMD_ARG(stop, NULL, "Stop commissionning", cmd_zb_stop, 1, 0),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(zb_custom, &sub_zb_custom, "Custom Zigbee shell commands.", NULL);