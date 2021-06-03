/*
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *                         Daniel Caujolle-Bert, F1RMB
 *
 * Using information from the MMDVM_HS source code by Andy CA6JAU
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

#include "functions/calibration.h"
#include "functions/hotspot.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"
#include "hardware/HR-C6000.h"
#include "functions/settings.h"
#include "functions/sound.h"
#include "functions/ticks.h"
#include "functions/trx.h"
#include "usb/usb_com.h"
#include "functions/rxPowerSaving.h"
#include "user_interface/uiHotspot.h"

// Uncomment the following to enable demo screen, access it with function events
//#define DEMO_SCREEN

/*
                                Problems with MD-390 on the same frequency
                                ------------------------------------------

 Using a Dual Hat:
M: 2020-01-07 08:46:23.337 DMR Slot 2, received RF voice header from F1RMB to 5000
M: 2020-01-07 08:46:24.143 DMR Slot 2, received RF end of voice transmission from F1RMB to 5000, 0.7 seconds, BER: 0.4%
M: 2020-01-07 08:46:24.644 DMR Slot 2, RF user 12935424 rejected

 Using OpenHD77 HS
 PC 5000 sent from a GD-77
M: 2020-01-07 09:51:55.467 DMR Slot 2, received RF voice header from F1RMB to 5000
M: 2020-01-07 09:51:56.727 DMR Slot 2, received RF end of voice transmission from F1RMB to 5000, 1.1 seconds, BER: 0.3%
M: 2020-01-07 09:52:00.068 DMR Slot 2, received network voice header from 5000 to TG 9
M: 2020-01-07 09:52:03.428 DMR Slot 2, received network end of voice transmission from 5000 to TG 9, 3.5 seconds, 0% packet loss, BER: 0.0%

 Its echo (data sent from the MD-390, by itself):
M: 2020-01-07 09:52:07.300 DMR Slot 2, received RF voice header from F1RMB to 5000
M: 2020-01-07 09:52:09.312 DMR Slot 2, RF voice transmission lost from F1RMB to 5000, 0.0 seconds, BER: 0.0%
M: 2020-01-07 09:52:11.856 DMR Slot 2, received network voice header from 5000 to TG 9
M: 2020-01-07 09:52:15.246 DMR Slot 2, received network end of voice transmission from 5000 to TG 9, 3.5 seconds, 0% packet loss, BER: 0.0%


	 There is a problem if you have a MD-390, GPS enabled, listening on the same frequency (even without different DMRId, I checked !).
	 It send invalid data from time to time (that's not critical, it's simply rejected), but once it heard a PC or TG call (from other
	 transceiver), it will keep repeating that, instead of invalid data.

	 After investigations, it's turns out if you enable 'GPS System' (even with correct configuration, like
	 GC xxx999 for BM), it will send such weird or echo data each GPS interval.

 ** Long story short: turn off GPS stuff on your MD-390 if it's listening local transmissions from other transceivers. **

 */


// Uncomment this to enable all sendDebug*() functions. You will see the results in the MMDVMHost log file.
//#define MMDVM_SEND_DEBUG



static uint8_t savedDMRDestinationFilter = 0xFF; // 0xFF value means unset
static uint8_t savedDMRCcTsFilter = 0xFF; // 0xFF value means unset
static bool displayFWVersion;
static uint32_t savedTGorPC;
static uint8_t savedLibreDMR_Power;

static bool handleEvent(uiEvent_t *ev);

#if defined(DEMO_SCREEN)
bool demoScreen = false;
#endif

menuStatus_t menuHotspotMode(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		rxPowerSavingSetLevel(0);// disable power saving

		// DMR filter level isn't saved yet (cycling power OFF/ON quickly can corrupt
		// this value otherwise, as menuHotspotMode(true) could be called twice.
		if (savedDMRDestinationFilter == 0xFF)
		{
			// Override DMR filtering
			savedDMRDestinationFilter = nonVolatileSettings.dmrDestinationFilter;
			savedDMRCcTsFilter = nonVolatileSettings.dmrCcTsFilter;
			settingsSet(nonVolatileSettings.dmrDestinationFilter, DMR_DESTINATION_FILTER_NONE);
			settingsSet(nonVolatileSettings.dmrCcTsFilter, DMR_CCTS_FILTER_CC_TS);
		}

		// Do not user per channel power settings
		savedLibreDMR_Power = currentChannelData->libreDMR_Power;
		currentChannelData->libreDMR_Power = 0;

		savedTGorPC = trxTalkGroupOrPcId;// Save the current TG or PC

		displayFWVersion = false;

		hotspotInit();

		ucClearBuf();
		ucPrintCentered(0, "Hotspot", FONT_SIZE_3);
		ucPrintCentered(32, "Waiting for", FONT_SIZE_3);
		ucPrintCentered(48, "Pi-Star", FONT_SIZE_3);
		ucRender();

		displayLightTrigger(false);
	}
	else
	{

#if defined(PLATFORM_GD77S)
		uiChannelModeHeartBeatActivityForGD77S(ev);
#endif

		if (ev->hasEvent)
		{
			if (handleEvent(ev) == false)
			{
				return MENU_STATUS_SUCCESS;
			}
		}
	}

#if defined(DEMO_SCREEN)
	if (! demoScreen)
	{
#endif
		processUSBDataQueue();
		if (com_request == 1)
		{
			handleHotspotRequest();
			com_request = 0;
		}
		hotspotStateMachine();

		// CW beaconing
		if (hotspotCwKeying)
		{
			if (hotspotCwpoLen > 0)
			{
				cwProcess();
			}
		}
#if defined(DEMO_SCREEN)
	}
#endif

	return MENU_STATUS_SUCCESS;
}

void menuHotspotRestoreSettings(void)
{
	if (savedDMRDestinationFilter != 0xFF)
	{
		settingsSet(nonVolatileSettings.dmrDestinationFilter, savedDMRDestinationFilter);
		savedDMRDestinationFilter = 0xFF; // Unset saved DMR destination filter level

		settingsSet(nonVolatileSettings.dmrCcTsFilter, savedDMRCcTsFilter);
		savedDMRCcTsFilter = 0xFF; // Unset saved CC TS filter level
	}
}

static void displayContactInfo(uint8_t y, char *text, size_t maxLen)
{
	// Max for TalkerAlias is 37: TA 27 (in 7bit format) + ' [' + 6 (Maidenhead)  + ']' + NULL
	// Max for DMRID Database is MAX_DMR_ID_CONTACT_TEXT_LENGTH (50 + NULL)
	char buffer[MAX_DMR_ID_CONTACT_TEXT_LENGTH];

	if (strlen(text) >= 5)
	{
		int32_t  cpos;

		if ((cpos = getFirstSpacePos(text)) != -1)
		{
			// Callsign found
			memcpy(buffer, text, cpos);
			buffer[cpos] = 0;
			ucPrintCentered(y, chomp(buffer), FONT_SIZE_3);
		}
		else
		{
			// No space found, use a chainsaw
			memcpy(buffer, text, 16);
			buffer[16] = 0;

			ucPrintCentered(y, chomp(buffer), FONT_SIZE_3);
		}
	}
	else
	{
		memcpy(buffer, text, strlen(text));
		buffer[strlen(text)] = 0;
		ucPrintCentered(y, chomp(buffer), FONT_SIZE_3);
	}
}

static void updateContactLine(uint8_t y)
{
	if ((LinkHead->talkGroupOrPcId >> 24) == PC_CALL_FLAG) // Its a Private call
	{
		ucPrintCentered(y, LinkHead->contact, FONT_SIZE_3);
	}
	else // Group call
	{
		switch (nonVolatileSettings.contactDisplayPriority)
		{
			case CONTACT_DISPLAY_PRIO_CC_DB_TA:
			case CONTACT_DISPLAY_PRIO_DB_CC_TA:
				// No contact found is codeplug and DMRIDs, use TA as fallback, if any.
				if ((strncmp(LinkHead->contact, "ID:", 3) == 0) && (LinkHead->talkerAlias[0] != 0x00))
				{
					displayContactInfo(y, LinkHead->talkerAlias, sizeof(LinkHead->talkerAlias));
				}
				else
				{
					displayContactInfo(y, LinkHead->contact, sizeof(LinkHead->contact));
				}
				break;

			case CONTACT_DISPLAY_PRIO_TA_CC_DB:
			case CONTACT_DISPLAY_PRIO_TA_DB_CC:
				// Talker Alias have the priority here
				if (LinkHead->talkerAlias[0] != 0x00)
				{
					displayContactInfo(y, LinkHead->talkerAlias, sizeof(LinkHead->talkerAlias));
				}
				else // No TA, then use the one extracted from Codeplug or DMRIdDB
				{
					displayContactInfo(y, LinkHead->contact, sizeof(LinkHead->contact));
				}
				break;
		}
	}
}

void uiHotspotUpdateScreen(uint8_t rxCommandState)
{
	int val_before_dp;
	int val_after_dp;
	static const int bufferLen = 17;
	char buffer[22]; // set to 22 due to FW info

	hotspotCurrentRxCommandState = rxCommandState;

	ucClearBuf();
	ucPrintAt(4, 4, "DMR", FONT_SIZE_1);
	ucPrintCentered(0, "Hotspot", FONT_SIZE_3);

	snprintf(buffer, bufferLen, "%d%%", getBatteryPercentage());

	ucPrintAt(DISPLAY_SIZE_X - (strlen(buffer) * 6) - 4, 4, buffer, FONT_SIZE_1);

	if (trxTransmissionEnabled)
	{
		if (displayFWVersion)
		{
			snprintf(buffer, 22, "%s", &HOTSPOT_VERSION_STRING[12]);
			ucPrintCentered(16 + 4, buffer, FONT_SIZE_1);
		}
		else
		{
			if (hotspotCwKeying)
			{
				sprintf(buffer, "%s", "<Tx CW ID>");
				ucPrintCentered(16, buffer, FONT_SIZE_3);
			}
			else
			{
				updateContactLine(16);
			}
		}

		if (hotspotCwKeying)
		{
			buffer[0] = 0;
		}
		else
		{
			if ((trxTalkGroupOrPcId & 0xFF000000) == 0)
			{
				snprintf(buffer, bufferLen, "%s %u", currentLanguage->tg, trxTalkGroupOrPcId & 0x00FFFFFF);
			}
			else
			{
				snprintf(buffer, bufferLen, "%s %u", currentLanguage->pc, trxTalkGroupOrPcId & 0x00FFFFFF);
			}
		}

		ucPrintCentered(32, buffer, FONT_SIZE_3);

		val_before_dp = hotspotFreqTx / 100000;
		val_after_dp = hotspotFreqTx - val_before_dp * 100000;
		sprintf(buffer, "T %d.%05d MHz", val_before_dp, val_after_dp);
	}
	else
	{
		if (rxCommandState == HOTSPOT_RX_START  || rxCommandState == HOTSPOT_RX_START_LATE)
		{
			dmrIdDataStruct_t currentRec;

			if (displayFWVersion)
			{
				snprintf(buffer, 22, "%s", &HOTSPOT_VERSION_STRING[12]);
			}
			else
			{
				if (dmrIDLookup(hotspotRxedDMR_LC.srcId, &currentRec) == true)
				{
					snprintf(buffer, bufferLen, "%s", currentRec.text);
				}
				else
				{
					snprintf(buffer, bufferLen, "ID: %u", hotspotRxedDMR_LC.srcId);
				}
			}

			ucPrintCentered(16 + (displayFWVersion ? 4 : 0), buffer, (displayFWVersion ? FONT_SIZE_1 : FONT_SIZE_3));

			if (hotspotRxedDMR_LC.FLCO == 0)
			{
				snprintf(buffer, bufferLen, "%s %u", currentLanguage->tg, hotspotRxedDMR_LC.dstId);
			}
			else
			{
				snprintf(buffer, bufferLen, "%s %u", currentLanguage->pc, hotspotRxedDMR_LC.dstId);
			}

			ucPrintCentered(32, buffer, FONT_SIZE_3);
		}
		else
		{

			if (displayFWVersion)
			{
				snprintf(buffer, 22, "%s", &HOTSPOT_VERSION_STRING[12]);
				ucPrintCentered(16 + 4, buffer, FONT_SIZE_1);
			}
			else
			{
				if (hotspotModemState == STATE_POCSAG)
				{
					ucPrintCentered(16, "<POCSAG>", FONT_SIZE_3);
				}
				else
				{
					if (strlen(hotspotMmdvmQSOInfoIP))
					{
						ucPrintCentered(16 + 4, hotspotMmdvmQSOInfoIP, FONT_SIZE_1);
					}
				}
			}

			snprintf(buffer, bufferLen, "CC:%d", trxGetDMRColourCode());//, trxGetDMRTimeSlot()+1) ;

			ucPrintCore(0, 32, buffer, FONT_SIZE_3, TEXT_ALIGN_LEFT, false);

			sprintf(buffer,"%s%s",POWER_LEVELS[hotspotPowerLevel],POWER_LEVEL_UNITS[hotspotPowerLevel]);
			ucPrintCore(0, 32, buffer, FONT_SIZE_3, TEXT_ALIGN_RIGHT, false);
		}
		val_before_dp = hotspotFreqRx / 100000;
		val_after_dp = hotspotFreqRx - val_before_dp * 100000;
		snprintf(buffer, bufferLen, "R %d.%05d MHz", val_before_dp, val_after_dp);
	}

	ucPrintCentered(48, buffer, FONT_SIZE_3);
	ucRender();

	displayLightTrigger(false);
}

static bool handleEvent(uiEvent_t *ev)
{
	displayLightTrigger(ev->hasEvent);

#if defined(DEMO_SCREEN)
	if (ev->events & FUNCTION_EVENT)
	{
		demoScreen = ((ev->function >= 90) && (ev->function < 100));

		switch (ev->function)
		{
			case 90:
				sprintf(hotspotMmdvmQSOInfoIP, "%s", "192.168.100.10");
				settingsUsbMode = USB_MODE_CPS;
				break;
			case 91:
				trxTalkGroupOrPcId = 98977;
				LinkHead->talkGroupOrPcId = 0;
				sprintf(LinkHead->contact, "%s", "VK3KYY");
				trxTransmissionEnabled = true;
				uiHotspotUpdateScreen(HOTSPOT_STATE_TX_START_BUFFERING);
				break;
			case 92:
				hotspotRxedDMR_LC.FLCO = 0;
				hotspotRxedDMR_LC.srcId = 5053238;
				hotspotRxedDMR_LC.dstId = 91;
				trxTransmissionEnabled = false;
				uiHotspotUpdateScreen(HOTSPOT_RX_START);
				break;
			case 93:
				hotspotModemState = STATE_POCSAG;
				trxTransmissionEnabled = false;
				uiHotspotUpdateScreen(HOTSPOT_RX_IDLE);
				break;
		}
	}
#endif

	if (KEYCHECK_SHORTUP(ev->keys, KEY_RED)
#if defined(PLATFORM_GD77S)
			// There is no RED key on the GD-77S, use Orange button to exit the hotspot mode
			|| ((ev->events & BUTTON_EVENT) && (ev->buttons & BUTTON_ORANGE))
#endif
	)
	{
		// Do not permit to leave HS in MMDVMHost mode, otherwise that will mess up the communication
		// and MMDVMHost won't recover from that, sometimes.
		// Anyway, in MMDVMHost mode, there is a timeout after MMDVMHost stop responding (or went to shutdown).
		if (nonVolatileSettings.hotspotType == HOTSPOT_TYPE_BLUEDV)
		{
			hotspotExit();
			return false;
		}
	}

	if (ev->events & BUTTON_EVENT)
	{
		// Display HS FW version
		if ((displayFWVersion == false) && (ev->buttons == BUTTON_SK1))
		{
			uint8_t prevRxCmd = hotspotCurrentRxCommandState;

			displayFWVersion = true;
			uiHotspotUpdateScreen(hotspotCurrentRxCommandState);
			hotspotCurrentRxCommandState = prevRxCmd;
			return true;
		}
		else if (displayFWVersion && ((ev->buttons & BUTTON_SK1) == 0))
		{
			displayFWVersion = false;
			uiHotspotUpdateScreen(hotspotCurrentRxCommandState);
			return true;
		}
	}

	return true;
}

void hotspotExit(void)
{
	trxDisableTransmission();
	if (trxTransmissionEnabled)
	{
		trxTransmissionEnabled = false;
		//trxSetRX();

		LEDs_PinWrite(GPIO_LEDgreen, Pin_LEDgreen, 0);

		if (hotspotCwKeying)
		{
			cwReset();
			hotspotCwKeying = false;
		}
	}

	currentChannelData->libreDMR_Power = savedLibreDMR_Power;

	trxTalkGroupOrPcId = savedTGorPC;// restore the current TG or PC
	if (hotspotSavedPowerLevel != -1)
	{
		trxSetPowerFromLevel(hotspotSavedPowerLevel);
	}

	trxDMRID = codeplugGetUserDMRID();
	settingsUsbMode = USB_MODE_CPS;
	hotspotMmdvmHostIsConnected = false;

	menuHotspotRestoreSettings();
	menuSystemPopAllAndDisplayRootMenu();
}

