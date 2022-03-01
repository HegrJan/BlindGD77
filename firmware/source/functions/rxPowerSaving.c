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

#include "functions/rxPowerSaving.h"
#include "functions/trx.h"
#include "functions/ticks.h"
#include <interfaces/clockManager.h>


static ecoPhase_t rxPowerSavingState = ECOPHASE_POWERSAVE_INACTIVE;
static uint32_t ecoPhaseCounter = 0;
volatile static int powerSavingLevel = 1;
static bool hrc6000IsPoweredOff = false;



bool rxPowerSavingIsRxOn(void)
{
	return (rxPowerSavingState != ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF);
}

void rxPowerSavingSetLevel(int newLevel)
{
	if (powerSavingLevel != newLevel)
	{
		if (newLevel == 0)
		{
			rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);
		}

		powerSavingLevel = newLevel;
	}
}

int rxPowerSavingGetLevel(void)
{
	return powerSavingLevel;
}

static void resumeBeepAndC6000Tasks(void)
{
	vTaskResume(hrc6000Task.Handle);
	hrc6000Task.Running = true;
	hrc6000Task.AliveCount = TASK_FLAGGED_ALIVE;
	vTaskResume(beepTask.Handle);
	beepTask.Running = true;
	beepTask.AliveCount = TASK_FLAGGED_ALIVE;
}

static void suspendBeepAndC6000Tasks(void)
{
	vTaskSuspend(beepTask.Handle);
	beepTask.Running = false;
	vTaskSuspend(hrc6000Task.Handle);
	hrc6000Task.Running = false;
}

void rxPowerSavingSetState(ecoPhase_t newState)
{
	if (rxPowerSavingState != newState)
	{
		if (rxPowerSavingState == ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF)
		{
			hrc6000IsPoweredOff = trxPowerUpDownRxAndC6000(true, hrc6000IsPoweredOff);
		}

		rxPowerSavingState = newState;

		// Avoid to instantly jump to power saving mode
		if (rxPowerSavingState == ECOPHASE_POWERSAVE_INACTIVE)
		{
			if (powerSavingLevel > 1)
			{
				clockManagerSetRunMode(kAPP_PowerModeRun, CLOCK_MANAGER_SPEED_RUN);
				resumeBeepAndC6000Tasks();
			}

			ecoPhaseCounter = ticksGetMillis() + ((12 - (MIN(powerSavingLevel, 4) * 2)) * 1000);
		}
	}
}

void rxPowerSavingTick(uiEvent_t *ev, bool hasSignal)
{
	if ((settingsUsbMode != USB_MODE_HOTSPOT) || (rxPowerSavingState != ECOPHASE_POWERSAVE_INACTIVE))
	{
		if (USB_DeviceIsResetting() || isCompressingAMBE || hasSignal || trxTransmissionEnabled || trxIsTransmitting ||
				(menuSystemGetCurrentMenuNumber() == UI_TX_SCREEN) || (menuSystemGetCurrentMenuNumber() == UI_CPS) ||
				(uiDataGlobal.Scan.active && uiDataGlobal.Scan.scanType == SCAN_TYPE_NORMAL_STEP) || ev->hasEvent)
		{
			if (rxPowerSavingState != ECOPHASE_POWERSAVE_INACTIVE)
			{
				if (rxPowerSavingState == ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF)
				{
					hrc6000IsPoweredOff = trxPowerUpDownRxAndC6000(true, hrc6000IsPoweredOff);// Power up AT1846S, C6000 and preamp
				}

				if (powerSavingLevel > 1)
				{
					clockManagerSetRunMode(kAPP_PowerModeRun, CLOCK_MANAGER_SPEED_RUN);
					resumeBeepAndC6000Tasks();
				}

				rxPowerSavingState = ECOPHASE_POWERSAVE_INACTIVE;
			}

			// Postpone entering the ECOPHASE_POWERSAVE_INACTIVE
			ecoPhaseCounter = ticksGetMillis() + ((12 - (MIN(powerSavingLevel, 4) * 2)) * 1000);
		}
		else
		{
			if ((ticksGetMillis() >= ecoPhaseCounter) && (powerSavingLevel > 0) && (melody_play == NULL) && (voicePromptsIsPlaying() == false))
			{
				int rxDuration = (130 - (10 * powerSavingLevel));
				switch(rxPowerSavingState)
				{
					case ECOPHASE_POWERSAVE_INACTIVE:// wait before shutting down
						ecoPhaseCounter = ticksGetMillis() + ((12 - (MIN(powerSavingLevel, 4) * 2)) * 1000);

						if (powerSavingLevel > 1)
						{
							suspendBeepAndC6000Tasks();
							clockManagerSetRunMode(kAPP_PowerModeRun, CLOCK_MANAGER_RUN_ECO_POWER_MODE);
						}
						rxPowerSavingState = ECOPHASE_POWERSAVE_ACTIVE___RX_IS_ON;
						break;

					case ECOPHASE_POWERSAVE_ACTIVE___RX_IS_ON:
						hrc6000IsPoweredOff = trxPowerUpDownRxAndC6000(false, (powerSavingLevel > 1));// Power down AT1846S, C6000 and preamp
						ecoPhaseCounter = ticksGetMillis() + (rxDuration * (1 << (powerSavingLevel - 1)));
						rxPowerSavingState = ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF;
						break;

					case ECOPHASE_POWERSAVE_ACTIVE___RX_IS_OFF:
						hrc6000IsPoweredOff = trxPowerUpDownRxAndC6000(true, hrc6000IsPoweredOff);// Power up AT1846S, C6000 and preamps
						ecoPhaseCounter = ticksGetMillis() + (rxDuration * 1);
						trxPostponeReadRSSIAndNoise(0); // Give it a bit of time, after powering up, before checking the RSSI and Noise values
						rxPowerSavingState = ECOPHASE_POWERSAVE_ACTIVE___RX_IS_ON;
						break;
				}
			}
		}
	}
}
