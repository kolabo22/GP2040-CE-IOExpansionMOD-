#include "addons/gamepad_usb_host.h"
#include "addons/gamepad_usb_host_listener.h"
#include "storagemanager.h"
#include "drivermanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"
#include "class/hid/hid_host.h"

bool GamepadUSBHostAddon::available()
{
    // 1. 参照元を非constにして取得（WebConfigデータ構造体）
    GamepadUSBHostOptions& gamepadUSBHostOptions = Storage::getInstance().getAddonOptions().gamepadUSBHostOptions;

    // 2. WebConfigに保存データがない（未設定リセット）場合のみ初期値をダイレクト上書き
    if (!gamepadUSBHostOptions.isConfigured) {
        gamepadUSBHostOptions.enabled = true;
        gamepadUSBHostOptions.pinDp = 28; // USB_DP (D+) を「GP28」に強制固定（D-は自動的にGP29へ）
    }

    return gamepadUSBHostOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void GamepadUSBHostAddon::setup()
{
    // setupでも未設定時の値を完全に保証する
    GamepadUSBHostOptions& gamepadUSBHostOptions = Storage::getInstance().getAddonOptions().gamepadUSBHostOptions;
    if (!gamepadUSBHostOptions.isConfigured) {
        gamepadUSBHostOptions.enabled = true;
        gamepadUSBHostOptions.pinDp = 28;
    }

    //stdio_init_all();
    //printf("setup function\n");
    listener = new GamepadUSBHostListener();
    ((GamepadUSBHostListener*)listener)->setup();
}

void GamepadUSBHostAddon::preprocess() {
    ((GamepadUSBHostListener*)listener)->process();
}
