/*
 * Copyright (C) 2021 Roger Clark, VK3KYY / G4KYF
 *
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. Use of this source code or binary releases for commercial purposes is strictly forbidden. This includes, without limitation,
 *    incorporation in a commercial product or incorporation into a product or project which allows commercial use.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <functions/rxPowerSaving.h>
#include "functions/trx.h"
#include "interfaces/pit.h"


static ecoPhase_t rxPowerSavingState = ECOPHASE_POWERSAVE_INACTIVE;
static uint32_t ecoPhaseCounter = 0;
static int powerSavingLevel = 1;

bool rxPowerSavingIsRxOn(void)
{
	return (rxPowerSavingState != ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF);
}

static void sendC6000BeepFixSequence(void)
{
	const int SIZE_OF_FILL_BUFFER = 128;
	uint8_t spi_values[SIZE_OF_FILL_BUFFER];

	memset(spi_values, 0xAA, SIZE_OF_FILL_BUFFER);
	SPI0SeClearPageRegByteWithMask(0x04, 0x06, 0xFD, 0x02); // SET OpenMusic bit (play Boot sound and Call Prompts)
	SPI0WritePageRegByteArray(0x03, 0x00, spi_values, SIZE_OF_FILL_BUFFER);
	SPI0SeClearPageRegByteWithMask(0x04, 0x06, 0xFD, 0x00); // CLEAR OpenMusic bit (play Boot sound and Call Prompts)
}

void rxPowerSavingSetLevel(int newLevel)
{
	if (powerSavingLevel != newLevel)
	{
		if (newLevel == 0)
		{
			if (rxPowerSavingState == ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF)
			{
				trxPowerUpDownRxAndC6000(true, true);
			}

			sendC6000BeepFixSequence();

			I2SReset();

			rxPowerSavingState = ECOPHASE_POWERSAVE_INACTIVE;
		}
		powerSavingLevel = newLevel;
	}
}

int rxPowerSavingGetLevel(void)
{
	return powerSavingLevel;
}

void rxPowerSavingSetState(ecoPhase_t newState)
{
	if (rxPowerSavingState != newState)
	{
		if (rxPowerSavingState == ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF)
		{
			trxPowerUpDownRxAndC6000(true, true);
			sendC6000BeepFixSequence();
		}

		if (newState == ECOPHASE_POWERSAVE_INACTIVE)
		{
			I2SReset();
		}
		rxPowerSavingState = newState;
	}
}

void rxPowerSavingTick(uiEvent_t *ev, bool hasSignal)
{
	if ((settingsUsbMode != USB_MODE_HOTSPOT) || (rxPowerSavingState != ECOPHASE_POWERSAVE_INACTIVE))
	{
		if (hasSignal || (uiDataGlobal.Scan.active && uiDataGlobal.Scan.scanType == SCAN_TYPE_NORMAL_STEP) || ev->hasEvent)
		{
			if (rxPowerSavingState != ECOPHASE_POWERSAVE_INACTIVE)
			{
				if (rxPowerSavingState == ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF)
				{
					trxPowerUpDownRxAndC6000(true, true);// Power up AT1846S, C6000 and preamp
				}
				sendC6000BeepFixSequence();
				rxPowerSavingState = ECOPHASE_POWERSAVE_INACTIVE;
			}
		}
		else
		{
			if ((PITCounter >= ecoPhaseCounter) && (powerSavingLevel > 0) && (melody_play == NULL) && (voicePromptsIsPlaying() == false))
			{
				int rxDuration = (130 - (10 * powerSavingLevel)) * 10;
				switch(rxPowerSavingState)
				{
					case ECOPHASE_POWERSAVE_INACTIVE:// wait before shutting down
						ecoPhaseCounter = PITCounter + ((12 - (MIN(powerSavingLevel,4) * 2)) * 1000 * 10 );
						rxPowerSavingState = ECHOPHASE_POWERSAVE_ENTRY;
						break;
					case ECHOPHASE_POWERSAVE_ENTRY:
						if (powerSavingLevel > 1)
						{
							SAI_TxEnable(I2S0, false);
							SAI_RxEnable(I2S0, false);
						}
						//rxPowerSavingState = ECOPHASE_POWERSAVE_ACTIVE___RX_IS_ON;
						// drop through

					case ECOPHASE_POWERSAVE_ACTIVE___RX_IS_ON:
						trxPowerUpDownRxAndC6000(false, (powerSavingLevel > 1));// Power down AT1846S, C6000 and preamp
						ecoPhaseCounter = PITCounter + (rxDuration * (1 << (powerSavingLevel - 1))) ;
						rxPowerSavingState = ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF;
						break;
					case ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF:
						trxPowerUpDownRxAndC6000(true, true);// Power up AT1846S, C6000 and preamps
						ecoPhaseCounter = PITCounter + (rxDuration * 1) ;
						rxPowerSavingState = ECOPHASE_POWERSAVE_ACTIVE___RX_IS_ON;
						break;
				}
			}
		}
	}
}
