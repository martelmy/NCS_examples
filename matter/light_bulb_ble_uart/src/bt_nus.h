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

/**
 *    @file
 *      Provides an implementation of Device Firmware Upgrade using SMP protocol
 *      over Bluetooth LE functionality for nRF Connect SDK platform.
 */

#pragma once

#include <platform/CHIPDeviceLayer.h>

#include <zephyr/kernel.h>

#include <bluetooth/services/nus.h>

typedef void (*NUSRestartAdvertisingHandler)(void);

class NUS
{
public:
    void Init(NUSRestartAdvertisingHandler startAdvertisingCb);
    void StartServer();
    void StartBLEAdvertising();
    bool IsEnabled() { return mIsEnabled; }

private:
    friend NUS & GetNUS(void);

    static void OnBleConnect(bt_conn * conn, uint8_t err);
    static void OnBleDisconnect(bt_conn * conn, uint8_t reason);
    static void OnNUSReceived(struct bt_conn *conn, const uint8_t *const data, uint16_t len);
    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    
    bool mIsEnabled;
    bool mIsAdvertisingEnabled;
    bt_conn_cb mBleConnCallbacks;
    bt_nus_cb mBleNUSCallbacks;
    NUSRestartAdvertisingHandler restartAdvertisingCallback;

    static NUS sNUS;
};

inline NUS & GetNUS(void)
{
    return NUS::sNUS;
}
