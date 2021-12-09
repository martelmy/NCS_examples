#include <zephyr.h>
#include <logging/log.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>

// c18489a2-581f-11ec-bf63-0242ac130002

/** @brief UUID of the Remote Service. **/
#define BT_UUID_REMOTE_SERV_VAL \
   BT_UUID_128_ENCODE(0xc1840001, 0x581f, 0x11ec, 0xbf63, 0x0242ac130002)

#define BT_UUID_REMOTE_SERVICE  BT_UUID_DECLARE_128(BT_UUID_REMOTE_SERV_VAL)

/** @brief UUID of the Button Characteristic. **/
#define BT_UUID_REMOTE_BUTTON_CHRC_VAL \
	BT_UUID_128_ENCODE(0xc1840002, 0x581f, 0x11ec, 0xbf63, 0x0242ac130002)

enum bt_button_notifications_enabled {
	BT_BUTTON_NOTIFICATIONS_ENABLED,
	BT_BUTTON_NOTIFICATIONS_DISABLED,
};

struct bt_remote_service_cb {
	void (*notif_changed)(enum bt_button_notifications_enabled status);
};

int bluetooth_init();

void set_button_value();

int send_button_notification(struct bt_conn *conn, uint8_t value);