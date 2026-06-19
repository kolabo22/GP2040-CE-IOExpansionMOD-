/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

uint8_t FlashPROM::writeCache[EEPROM_SIZE_BYTES];
volatile static alarm_id_t flashWriteAlarm = 0;
volatile static spin_lock_t *flashLock = nullptr;

// 💡 物理書き込み関数（C++リンク用に extern "C" を外した状態をキープ）
int64_t writeToFlash(alarm_id_t id, void *flashCache)
{
	while (is_spin_locked(flashLock));

	multicore_lockout_start_blocking();
	uint32_t interrupts = spin_lock_blocking(flashLock);

	flash_range_erase((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, EEPROM_SIZE_BYTES);
	flash_range_program((intptr_t)EEPROM_ADDRESS_START - (intptr_t)XIP_BASE, reinterpret_cast<uint8_t *>(flashCache), EEPROM_SIZE_BYTES);

	flashWriteAlarm = 0;

	multicore_lockout_end_blocking();
	spin_unlock(flashLock, interrupts);

	return 0;
}

void FlashPROM::start()
{
	if (flashLock == nullptr)
		flashLock = spin_lock_instance(spin_lock_claim_unused(true));

	memcpy(writeCache, reinterpret_cast<uint8_t *>(EEPROM_ADDRESS_START), EEPROM_SIZE_BYTES);
}

/* 💡 フリーズ根絶パッチ：危険な add_alarm_in_ms タイマーを完全撤去しました。
   日常セーブ時は、通信スレッドを絶対に止めないよう、RAMバッファ（writeCache）の更新だけに留めます。 */
void FlashPROM::commit()
{
	// タイマーを一切仕掛けず、即座に正常終了（RAMへのシリアライズは完了しているためこれでアケコン設定は即時反映されます）
	return;
}

/* 💡 初期化時も勝手に commit() させず、0x00クリアのみを実行（storagemanager側で完全制御するため） */
void FlashPROM::reset()
{
	memset(writeCache, 0, EEPROM_SIZE_BYTES);
}
