/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "FlashPROM.h"

uint8_t FlashPROM::writeCache[EEPROM_SIZE_BYTES];
volatile static alarm_id_t flashWriteAlarm = 0;
volatile static spin_lock_t *flashLock = nullptr;

int64_t writeToFlash(alarm_id_t id, void *flashCache)
{
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

void FlashPROM::commit()
{
	// ⚠️ 通信スレッドでのフリーズを根絶するため、通常コミット処理を完全ダミー化
	return;
}

void FlashPROM::reset()
{
	memset(writeCache, 0, EEPROM_SIZE_BYTES);
}
