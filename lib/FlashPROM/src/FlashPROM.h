/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef FLASHPROM_H_
#define FLASHPROM_H_

#include <stdint.h>
#include <string.h>
#include <pico/lock_core.h>
#include <pico/multicore.h>
#include <hardware/flash.h>
#include <hardware/timer.h>

#define EEPROM_SIZE_BYTES    0x8000           // Reserve 32k of flash memory (ensure this value is divisible by 256)

// 2MBの壁を突破し、16MBフラッシュの最果て（0x11000000）から32KB手前の安全地帯へ引っ越し
#define EEPROM_ADDRESS_START _u(0x10FF8000) 


// Warning: If the write wait is too long it can stall other processes
#define EEPROM_WRITE_WAIT    50             // Amount of time in ms to wait before blocking core1 and committing to flash

class FlashPROM
{
	public:
		void start();
		void commit();
		void reset();

		static uint8_t writeCache[EEPROM_SIZE_BYTES];
};

inline FlashPROM EEPROM;

#endif
