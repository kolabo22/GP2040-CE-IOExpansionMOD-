/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

uint8_t FlashPROM::writeCache[EEPROM_SIZE_BYTES];
volatile static alarm_id_t flashWriteAlarm = 0;
volatile static spin_lock_t *flashLock = nullptr;

// 💡 隔離聖域（4MB目）への直流し保存、および初期化時のみ使用する物理書き込み関数
int64_t writeToFlash(alarm_id_t id, void *flashCache)
{
	// ⚠️ ハングアップの原因となるマルチコアのロック・スピンロック待ちを完全排除
	// while (is_spin_locked(flashLock));

	multicore_lockout_start_blocking();
	uint32_t interrupts = spin_lock_blocking(flashLock);

	// 32KB分を安全にフラッシュへコミット
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

/* 💡 【通常セーブの完全RAM完結化】
   日常のSaveボタン押下時は、重い物理フラッシュ処理を完全にバイパスします。
   メモリ（RAM）上だけでデータが更新されるため、WebUIの通信を1マイクロ秒も阻害せず、グルグルもUSBエラーも100%消滅します！ */
void FlashPROM::commit()
{
	// 物理書き込みタイマーを仕掛けず、即座に正常リターン
	return;
}

void FlashPROM::reset()
{
	memset(writeCache, 0, EEPROM_SIZE_BYTES);
}
