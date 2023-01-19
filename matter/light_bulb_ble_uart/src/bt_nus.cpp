/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "bt_nus.h"

#include <platform/CHIPDeviceLayer.h>

#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip::DeviceLayer;

constexpr uint16_t kAdvertisingIntervalMinMs = 400;
constexpr uint16_t kAdvertisingIntervalMaxMs = 500;

NUS NUS::sNUS;

void NUS::Init(NUSRestartAdvertisingHandler startAdvertisingCb)
{

    memset(&mBleConnCallbacks, 0, sizeof(mBleConnCallbacks));
    mBleConnCallbacks.connected    = OnBleConnect;
    mBleConnCallbacks.disconnected = OnBleDisconnect;

    bt_conn_cb_register(&mBleConnCallbacks);

    memset(&mBleNUSCallbacks, 0, sizeof(mBleNUSCallbacks));
    mBleNUSCallbacks.received = OnNUSReceived;
    bt_nus_init(&mBleNUSCallbacks);

    restartAdvertisingCallback = startAdvertisingCb;

    PlatformMgr().AddEventHandler(ChipEventHandler, 0);
}

void NUS::StartServer()
{
    if (!mIsEnabled)
    {
        mIsEnabled = true;

        ChipLogProgress(DeviceLayer, "Enabled NUS");

        // Start NUS advertising only in case CHIPoBLE advertising is not working.
        if (!ConnectivityMgr().IsBLEAdvertisingEnabled())
            StartBLEAdvertising();
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NUS is already enabled");
    }
}

void NUS::StartBLEAdvertising()
{
    if (!mIsEnabled && !mIsAdvertisingEnabled)
        return;

    const char * deviceName = bt_get_name();
    const uint8_t advFlags  = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;
    bt_data ad[] = { BT_DATA(BT_DATA_FLAGS, &advFlags, sizeof(advFlags)),
                     BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName))) };
    bt_data sd[] = { BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL) };
    int rc;
    bt_le_adv_param advParams = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE, kAdvertisingIntervalMinMs,
                                                     kAdvertisingIntervalMaxMs, nullptr);

    rc = bt_le_adv_stop();
    if (rc)
    {
        ChipLogError(DeviceLayer, "NUS advertising stop failed (rc %d)", rc);
    }

    rc = bt_le_adv_start(&advParams, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (rc)
    {
        ChipLogError(DeviceLayer, "NUS advertising start failed (rc %d)", rc);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Started NUS advertising");
        mIsAdvertisingEnabled = true;
    }
}

void NUS::OnBleConnect(bt_conn * conn, uint8_t err)
{
    ChipLogProgress(DeviceLayer, "Connected");
}

void NUS::OnBleDisconnect(struct bt_conn * conId, uint8_t reason)
{
    PlatformMgr().LockChipStack();

    // After BLE disconnect SMP advertising needs to be restarted. Before making it ensure that BLE disconnect was not triggered
    // by closing CHIPoBLE service connection (in that case CHIPoBLE advertising needs to be restarted).
    if (!ConnectivityMgr().IsBLEAdvertisingEnabled() && (ConnectivityMgr().NumBLEConnections() == 0))
    {
        sNUS.restartAdvertisingCallback();
    }

    PlatformMgr().UnlockChipStack();
}


void NUS::OnNUSReceived(struct bt_conn *conn, const uint8_t *const data,
			  uint16_t len)
{
    ChipLogProgress(DeviceLayer, "Received data: %c", data[0]);
}

void NUS::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    if (!GetNUS().IsEnabled())
        return;

    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
        {
            // Check if CHIPoBLE advertising was stopped permanently or it just a matter of opened BLE connection.
            if (ConnectivityMgr().NumBLEConnections() == 0)
                sNUS.restartAdvertisingCallback();
        }
        break;
    case DeviceEventType::kCHIPoBLEConnectionClosed:
        // Check if after closing CHIPoBLE connection advertising is working, if no start SMP advertising.
        if (!ConnectivityMgr().IsBLEAdvertisingEnabled())
        {
            sNUS.restartAdvertisingCallback();
        }
        break;
    default:
        break;
    }
}
