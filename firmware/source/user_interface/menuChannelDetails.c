/*
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *                         Colin Durbridge, G4EML
 *                         Daniel Caujolle-Bert, F1RMB
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

#include "functions/codeplug.h"
#include "functions/settings.h"
#include "functions/trx.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"

static void updateScreen(bool isFirstRun, bool allowedToSpeakUpdate);
static void updateCursor(bool moved);
static void handleEvent(uiEvent_t *ev);
static void cssDecrementFromEvent(uiEvent_t *ev, uint16_t *tone, uint8_t *index, CodeplugCSSTypes_t *type);
static void cssIncrementFromEvent(uiEvent_t *ev, uint16_t *tone, uint8_t *index, CodeplugCSSTypes_t *type);
static void saveChanges(uiEvent_t *ev);
static void resetChannelData(void);
static uint8_t RxCSSIndex = 0;
static uint8_t TxCSSIndex = 0;
static CodeplugCSSTypes_t RxCSSType = CSS_TYPE_NONE;
static CodeplugCSSTypes_t TxCSSType = CSS_TYPE_NONE;
static const char CHANNEL_UNSET[5] = { 0xFF, 0xDE, 0xAD, 0xBE, 0xEF };
static struct_codeplugChannel_t tmpChannel =  // update a temporary copy of the channel and only write back if green menu is pressed
{
		.name = { 0xFF, 0xDE, 0xAD, 0xBE, 0xEF }
};
static char channelName[SCREEN_LINE_BUFFER_SIZE];
static int namePos;
static bool nameInError = false;

static menuStatus_t menuChannelDetailsExitCode = MENU_STATUS_SUCCESS;

enum CHANNEL_DETAILS_DISPLAY_LIST { CH_DETAILS_NAME = 0,
									CH_DETAILS_RXFREQ, CH_DETAILS_TXFREQ,
									CH_DETAILS_MODE, CH_DETAILS_DMRID,
									CH_DETAILS_DMR_CC, CH_DETAILS_DMR_TS, CH_DETAILS_RXGROUP,
									CH_DETAILS_RXCSS, CH_DETAILS_TXCSS, CH_DETAILS_BANDWIDTH,
									CH_DETAILS_FREQ_STEP, CH_DETAILS_TOT, CH_DETAILS_RXONLY,
									CH_DETAILS_ZONE_SKIP,CH_DETAILS_ALL_SKIP,
									CH_DETAILS_VOX,
									CH_DETAILS_POWER,
									CH_DETAILS_SQUELCH,
									NUM_CH_DETAILS_ITEMS};// The last item in the list is used so that we automatically get a total number of items in the list

menuStatus_t menuChannelDetails(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		menuDataGlobal.menuOptionsSetQuickkey = 0;
		menuDataGlobal.menuOptionsTimeout = 0;
		menuDataGlobal.endIndex = NUM_CH_DETAILS_ITEMS;
		uiDataGlobal.FreqEnter.index = 0;
		nameInError = false;

		if (memcmp(tmpChannel.name, CHANNEL_UNSET, 5) == 0) // Check if the channel was already loaded (TX Screen was triggered within this menu)
		{
			memcpy(&tmpChannel, currentChannelData, CHANNEL_DATA_STRUCT_SIZE);
		}

		freqEnterReset();

		RxCSSType = codeplugGetCSSType(tmpChannel.rxTone);
		RxCSSIndex = cssGetToneIndex(tmpChannel.rxTone, RxCSSType);

		TxCSSType = codeplugGetCSSType(tmpChannel.txTone);
		TxCSSIndex = cssGetToneIndex(tmpChannel.txTone, TxCSSType);

		codeplugUtilConvertBufToString(tmpChannel.name, channelName, 16);
		namePos = strlen(channelName);

		if ((uiDataGlobal.currentSelectedChannelNumber == CH_DETAILS_VFO_CHANNEL) && (namePos == 0)) // In VFO, and VFO has no name in the codeplug
		{
			snprintf(channelName, SCREEN_LINE_BUFFER_SIZE, "VFO %s", (nonVolatileSettings.currentVFONumber == 0 ? "A" : "B"));
			namePos = 5;
		}

		voicePromptsInit();
		voicePromptsAppendPrompt(PROMPT_SILENCE);
		voicePromptsAppendLanguageString(&currentLanguage->channel_details);
		voicePromptsAppendLanguageString(&currentLanguage->menu);
		voicePromptsAppendPrompt(PROMPT_SILENCE);

		updateScreen(true, true);
		updateCursor(true);

		return (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuChannelDetailsExitCode = MENU_STATUS_SUCCESS;

		updateCursor(false);

		if (ev->hasEvent || (menuDataGlobal.menuOptionsTimeout > 0))
		{
			handleEvent(ev);
		}
	}
	return menuChannelDetailsExitCode;
}

static void updateCursor(bool moved)
{
	if (uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL)
	{
		switch (menuDataGlobal.currentItemIndex)
		{
		case CH_DETAILS_NAME:
			menuUpdateCursor(namePos, moved, true);
			break;
		}
	}
}

static void updateScreen(bool isFirstRun, bool allowedToSpeakUpdate)
{
	int mNum = 0;
	char buf[SCREEN_LINE_BUFFER_SIZE];
	int tmpVal;
	int val_before_dp;
	int val_after_dp;
	struct_codeplugRxGroup_t rxGroupBuf;
	char rxNameBuf[SCREEN_LINE_BUFFER_SIZE];
	char * const *leftSide = NULL;// initialise to please the compiler
	char * const *rightSideConst = NULL;// initialise to please the compiler
	char rightSideVar[SCREEN_LINE_BUFFER_SIZE];
	voicePrompt_t rightSideUnitsPrompt;
	const char * rightSideUnitsStr;

	ucClearBuf();

	bool settingOption = uiShowQuickKeysChoices(buf, SCREEN_LINE_BUFFER_SIZE, currentLanguage->channel_details);

	if (uiDataGlobal.FreqEnter.index != 0)
	{
		snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%c%c%c%s%c%c%c%c%c%s", uiDataGlobal.FreqEnter.digits[0], uiDataGlobal.FreqEnter.digits[1], uiDataGlobal.FreqEnter.digits[2], (menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID) ? "" : ".",
				uiDataGlobal.FreqEnter.digits[3], uiDataGlobal.FreqEnter.digits[4], uiDataGlobal.FreqEnter.digits[5], uiDataGlobal.FreqEnter.digits[6], uiDataGlobal.FreqEnter.digits[7], (menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID) ? "" : " MHz");
		ucPrintCentered(32, buf, FONT_SIZE_3);
	}
	else
	{
		keypadAlphaEnable = (menuDataGlobal.currentItemIndex == CH_DETAILS_NAME);

		// Can only display 3 of the options at a time menu at -1, 0 and +1
		for(int i = -1; i <= 1; i++)
		{
			if ((settingOption == false) || (i == 0))
			{
				mNum = menuGetMenuOffset(NUM_CH_DETAILS_ITEMS, i);
				buf[0] = 0;
				leftSide = NULL;
				rightSideConst = NULL;
				rightSideVar[0] = 0;
				rightSideUnitsPrompt = PROMPT_SILENCE;// use PROMPT_SILENCE as flag that the unit has not been set
				rightSideUnitsStr = NULL;

				switch(mNum)
				{
					case CH_DETAILS_NAME:
						strncpy(rightSideVar, channelName, SCREEN_LINE_BUFFER_SIZE);
					break;
					case CH_DETAILS_MODE:
						leftSide = (char * const *)&currentLanguage->mode;
						strcpy(rightSideVar, (tmpChannel.chMode == RADIO_MODE_ANALOG) ? "FM" : "DMR");
						break;
					break;
					case CH_DETAILS_DMRID:
						leftSide = (char * const *)&currentLanguage->dmr_id;
						if (tmpChannel.chMode == RADIO_MODE_ANALOG)
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						else
						{
							uint32_t dmrID = codeplugChannelGetOptionalDMRID(&tmpChannel);
							if (dmrID == 0)
							{
								rightSideConst = (char * const *)&currentLanguage->none;
							}
							else
							{
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u", dmrID);
							}
						}
						break;
					case CH_DETAILS_DMR_CC:
						leftSide = (char * const *)&currentLanguage->colour_code;
						rightSideConst = (char * const *)&currentLanguage->n_a;
						if (tmpChannel.chMode == RADIO_MODE_ANALOG)
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						else
						{
							snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u", tmpChannel.txColor);
						}
						break;
					case CH_DETAILS_DMR_TS:
						leftSide = (char * const *)&currentLanguage->timeSlot;
						if (tmpChannel.chMode == RADIO_MODE_ANALOG)
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						else
						{
							snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u", ((tmpChannel.flag2 & 0x40) >> 6) + 1);
						}
						break;
					case CH_DETAILS_RXGROUP:
						leftSide = (char * const *)&currentLanguage->rx_group;
						if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
						{
							if (tmpChannel.rxGroupList == 0)
							{
								rightSideConst = (char * const *)&currentLanguage->none;
							}
							else
							{
								codeplugRxGroupGetDataForIndex(tmpChannel.rxGroupList, &rxGroupBuf);
								codeplugUtilConvertBufToString(rxGroupBuf.name, rxNameBuf, 16);
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%s", rxNameBuf);
							}
						}
						else
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						break;
					case CH_DETAILS_RXCSS:
						if (tmpChannel.chMode == RADIO_MODE_ANALOG)
						{
							if (RxCSSType == CSS_TYPE_CTCSS)
							{
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Rx CTCSS:%u.%uHz", tmpChannel.rxTone / 10 , tmpChannel.rxTone % 10);
							}
							else if (RxCSSType & CSS_TYPE_DCS)
							{
								dcsPrintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Rx DCS:", tmpChannel.rxTone);
							}
							else
							{
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Rx CSS:%s", currentLanguage->none);
							}
						}
						else
						{
							snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Rx CSS:%s", currentLanguage->n_a);
						}
						break;
					case CH_DETAILS_TXCSS:
						if (tmpChannel.chMode == RADIO_MODE_ANALOG)
						{
							if  (TxCSSType == CSS_TYPE_CTCSS)
							{
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Tx CTCSS:%u.%uHz", tmpChannel.txTone / 10 , tmpChannel.txTone % 10);
							}
							else if (TxCSSType & CSS_TYPE_DCS)
							{
								dcsPrintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Tx DCS:", tmpChannel.txTone);
							}
							else
							{
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Tx CSS:%s", currentLanguage->none);
							}
						}
						else
						{
							snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Tx CSS:%s", currentLanguage->n_a);
						}
						break;
					case CH_DETAILS_RXFREQ:
						rightSideUnitsPrompt = PROMPT_MEGAHERTZ;
						rightSideUnitsStr = "MHz";
						val_before_dp = tmpChannel.rxFreq / 100000;
						val_after_dp = tmpChannel.rxFreq - val_before_dp * 100000;
						snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Rx:%d.%05d", val_before_dp, val_after_dp);
						break;
					case CH_DETAILS_TXFREQ:
						rightSideUnitsPrompt = PROMPT_MEGAHERTZ;
						rightSideUnitsStr = "MHz";
						val_before_dp = tmpChannel.txFreq / 100000;
						val_after_dp = tmpChannel.txFreq - val_before_dp * 100000;
						snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "Tx:%d.%05d", val_before_dp, val_after_dp);
						break;
					case CH_DETAILS_BANDWIDTH:
						// Bandwidth
						leftSide = (char * const *)&currentLanguage->bandwidth;
						if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						else
						{
							rightSideUnitsPrompt = PROMPT_KILOHERTZ;
							rightSideUnitsStr = "kHz";
							snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%s", ((tmpChannel.flag4 & 0x02) == 0x02) ? "25" : "12.5");
						}
						break;
					case CH_DETAILS_FREQ_STEP:
						rightSideUnitsPrompt = PROMPT_KILOHERTZ;
						rightSideUnitsStr = "kHz";
						leftSide = (char * const *)&currentLanguage->stepFreq;
						tmpVal = VFO_FREQ_STEP_TABLE[(tmpChannel.VFOflag5 >> 4)] / 100;
						snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u.%02u", tmpVal, VFO_FREQ_STEP_TABLE[(tmpChannel.VFOflag5 >> 4)] - (tmpVal * 100));
						break;
					case CH_DETAILS_TOT:// TOT
						leftSide = (char * const *)&currentLanguage->tot;
						if (tmpChannel.tot != 0)
						{
							rightSideUnitsPrompt = PROMPT_SECONDS;
							rightSideUnitsStr = "s";

							snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u", tmpChannel.tot * 15);
						}
						else
						{
							rightSideConst = (char * const *)&currentLanguage->off;
						}
						break;
					case CH_DETAILS_RXONLY:
						leftSide = (char * const *)&currentLanguage->rx_only;
						rightSideConst = (char * const *)(((tmpChannel.flag4 & 0x04) == 0x04) ? &currentLanguage->yes : &currentLanguage->no);
						break;
					case CH_DETAILS_ZONE_SKIP:						// Zone Scan Skip Channel (Using CPS Auto Scan flag)
						leftSide = (char * const *)&currentLanguage->zone_skip;
						rightSideConst = (char * const *)(((tmpChannel.flag4 & CODEPLUG_CHANNEL_FLAG_ZONE_SKIP) == CODEPLUG_CHANNEL_FLAG_ZONE_SKIP) ? &currentLanguage->yes : &currentLanguage->no);
						break;
					case CH_DETAILS_ALL_SKIP:					// All Scan Skip Channel (Using CPS Lone Worker flag)
						leftSide = (char * const *)&currentLanguage->all_skip;
						rightSideConst = (char * const *)(((tmpChannel.flag4 & CODEPLUG_CHANNEL_FLAG_ALL_SKIP) == CODEPLUG_CHANNEL_FLAG_ALL_SKIP) ? &currentLanguage->yes : &currentLanguage->no);
						break;
					case CH_DETAILS_VOX:
						rightSideConst = (char * const *)(((tmpChannel.flag4 & 0x40) == 0x40) ? &currentLanguage->on : &currentLanguage->off);
						snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "VOX:%s", *rightSideConst);
						break;
					case CH_DETAILS_POWER:
						leftSide = (char * const *)&currentLanguage->channel_power;
						if (uiDataGlobal.currentSelectedChannelNumber == CH_DETAILS_VFO_CHANNEL)
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						else
						{
							if (tmpChannel.libreDMR_Power == 0)
							{
								rightSideConst = (char * const *)&currentLanguage->from_master;
							}
							else
							{
								int powerIndex = tmpChannel.libreDMR_Power - 1;
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%s%s", POWER_LEVELS[powerIndex], POWER_LEVEL_UNITS[powerIndex]);
							}
						}
						break;
					case CH_DETAILS_SQUELCH:
						leftSide = (char * const *)&currentLanguage->squelch;
						if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
						{
							rightSideConst = (char * const *)&currentLanguage->n_a;
						}
						else
						{
							if (tmpChannel.sql == 0)
							{
								rightSideConst = (char * const *)&currentLanguage->from_master;
							}
							else
							{
								snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u%%", (5 * (tmpChannel.sql - 1)));
							}
						}
						break;
				}

				if (leftSide != NULL)
				{
					snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%s:%s", *leftSide, (rightSideVar[0] ? rightSideVar : (rightSideConst ? *rightSideConst : "")));
				}
				else
				{
					strcpy(buf, rightSideVar);
				}

				if ((i == 0) && allowedToSpeakUpdate && (uiDataGlobal.FreqEnter.index == 0))
				{
					if (!isFirstRun && (menuDataGlobal.menuOptionsSetQuickkey == 0))
					{
						voicePromptsInit();
					}

					if ((mNum == CH_DETAILS_NAME) && (rightSideVar[0] == 0))
					{
						if (nameInError)
						{
							voicePromptsAppendLanguageString(&currentLanguage->error);
							voicePromptsAppendPrompt(PROMPT_SILENCE);
						}
						voicePromptsAppendLanguageString(&currentLanguage->name);
						voicePromptsAppendPrompt(PROMPT_SILENCE);
						voicePromptsAppendLanguageString(&currentLanguage->none);
					}
					else if ((mNum == CH_DETAILS_RXCSS) || (mNum == CH_DETAILS_TXCSS))
					{
						if (tmpChannel.chMode == RADIO_MODE_ANALOG)
						{
							switch (mNum)
							{
								case CH_DETAILS_RXCSS:
									announceCSSCode(tmpChannel.rxTone, RxCSSType, DIRECTION_RECEIVE, true, AUDIO_PROMPT_MODE_VOICE_LEVEL_3);
									break;
								case CH_DETAILS_TXCSS:
									announceCSSCode(tmpChannel.txTone, TxCSSType, DIRECTION_TRANSMIT, true, AUDIO_PROMPT_MODE_VOICE_LEVEL_3);
									break;
								default:
									break;
							}
						}
						else
						{
							voicePromptsAppendString(((mNum == CH_DETAILS_RXCSS) ? "Rx CSS" : "Tx CSS"));
							voicePromptsAppendLanguageString(&currentLanguage->n_a);
						}
					}
					else if (mNum == CH_DETAILS_VOX)
					{
						voicePromptsAppendPrompt(PROMPT_VOX);
						voicePromptsAppendLanguageString((const char * const *)rightSideConst);
					}
					else if ((mNum == CH_DETAILS_POWER) &&
							((tmpChannel.libreDMR_Power != 0) && (uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL)))
					{
						char buf2[17];
						char *p;

						voicePromptsAppendLanguageString((const char * const *)leftSide);
						memcpy(buf2, rightSideVar, 17);
						if ((p = strstr(buf2, "mW")))
						{
							*p = 0;
							voicePromptsAppendString(buf2);
							voicePromptsAppendPrompt(PROMPT_MILLIWATTS);
						}
						else if (strstr(buf2, "+W-"))
						{
							voicePromptsAppendLanguageString(&currentLanguage->user_power);
						}
						else if ((p = strstr(buf2, "W")))
						{
							*p = 0;
							voicePromptsAppendString(buf2);
							voicePromptsAppendPrompt((((tmpChannel.libreDMR_Power - 1) == 4) ? PROMPT_WATT : PROMPT_WATTS));
						}
					}
					else
					{
						if (leftSide != NULL)
						{
							voicePromptsAppendLanguageString((const char * const *)leftSide);
						}

						if ((rightSideVar[0] != 0) || ((rightSideVar[0] == 0) && (rightSideConst == NULL)))
						{
							voicePromptsAppendString(rightSideVar);
						}
						else
						{
							voicePromptsAppendLanguageString((const char * const *)rightSideConst);
						}

						if (rightSideUnitsPrompt != PROMPT_SILENCE)
						{
							voicePromptsAppendPrompt(rightSideUnitsPrompt);
						}

						if (rightSideUnitsStr != NULL)
						{
							strncat(rightSideVar, rightSideUnitsStr, SCREEN_LINE_BUFFER_SIZE);
						}
					}

					if (menuDataGlobal.menuOptionsTimeout != -1)
					{
						promptsPlayNotAfterTx();
					}
					else
					{
						menuDataGlobal.menuOptionsTimeout = 0;// clear flag indicating that a QuickKey has just been set
					}
				}

				// QuickKeys
				if (menuDataGlobal.menuOptionsTimeout > 0)
				{
					menuDisplaySettingOption(*leftSide, (rightSideVar[0] ? rightSideVar : *rightSideConst));
				}
				else
				{
					if (rightSideUnitsStr != NULL)
					{
						strncat(buf, rightSideUnitsStr, SCREEN_LINE_BUFFER_SIZE);
					}

					menuDisplayEntry(i, mNum, buf);
				}
			}
		}
	}
	ucRender();
}

static void updateFrequency(int frequency)
{
	switch (menuDataGlobal.currentItemIndex)
	{
		case CH_DETAILS_RXFREQ:
			tmpChannel.rxFreq = frequency;
			break;
		case CH_DETAILS_TXFREQ:
			tmpChannel.txFreq = frequency;
			break;
	}

	freqEnterReset();
}

static void handleEvent(uiEvent_t *ev)
{
	int tmpVal;
	struct_codeplugRxGroup_t rxGroupBuf;
	//bool isDirty = false;

	if ((menuDataGlobal.menuOptionsTimeout > 0) && (!BUTTONCHECK_DOWN(ev, BUTTON_SK2)))
	{
		menuDataGlobal.menuOptionsTimeout--;
		if (menuDataGlobal.menuOptionsTimeout == 0)
		{
			resetChannelData();
			menuSystemPopPreviousMenu();
			return;
		}
	}

	if (ev->events & FUNCTION_EVENT)
	{
		if ((QUICKKEY_TYPE(ev->function) == QUICKKEY_MENU) && (QUICKKEY_ENTRYID(ev->function) < NUM_CH_DETAILS_ITEMS))
		{
			menuDataGlobal.currentItemIndex = QUICKKEY_ENTRYID(ev->function);
		}

		if ((QUICKKEY_FUNCTIONID(ev->function) != 0) &&
			((menuDataGlobal.currentItemIndex != CH_DETAILS_NAME) && (menuDataGlobal.currentItemIndex != CH_DETAILS_RXFREQ) && (menuDataGlobal.currentItemIndex != CH_DETAILS_TXFREQ)))
		{
			menuDataGlobal.menuOptionsTimeout = 1000;
		}
		updateScreen(false, true);
	}

	if (ev->events & BUTTON_EVENT)
	{
		if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK1))
		{
			if (menuDataGlobal.currentItemIndex == CH_DETAILS_RXCSS)
			{
				announceCSSCode(tmpChannel.rxTone, RxCSSType, DIRECTION_RECEIVE, true, PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY);
			}
			else if (menuDataGlobal.currentItemIndex == CH_DETAILS_TXCSS)
			{
				announceCSSCode(tmpChannel.txTone, TxCSSType, DIRECTION_TRANSMIT, true, PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY);
			}
		}

		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}
	}

	if ((menuDataGlobal.currentItemIndex == CH_DETAILS_RXFREQ) || (menuDataGlobal.currentItemIndex == CH_DETAILS_TXFREQ) || (menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID))
	{
		if (uiDataGlobal.FreqEnter.index != 0)
		{
			if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
			{
				int number = freqEnterRead(0, 8, (menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID));

				if (((menuDataGlobal.currentItemIndex == CH_DETAILS_RXFREQ) || (menuDataGlobal.currentItemIndex == CH_DETAILS_TXFREQ))
						&& (trxGetBandFromFrequency(number) != -1))
				{
					updateFrequency(number);
				}
				else if ((menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID)
						&& (((number >= MIN_TG_OR_PC_VALUE) && (number <= MAX_TG_OR_PC_VALUE)) || (number == 0)))
				{
					codeplugChannelSetOptionalDMRID(number, &tmpChannel);
					freqEnterReset();
				}
				else
				{
					menuChannelDetailsExitCode |= MENU_STATUS_ERROR;
				}
				updateScreen(false, true);
				return;
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				freqEnterReset();
				updateScreen(false, true);
				return;
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_LEFT))
			{
				char buf[17];

				uiDataGlobal.FreqEnter.index--;
				uiDataGlobal.FreqEnter.digits[uiDataGlobal.FreqEnter.index] = '-';

				voicePromptsInit();
				snprintf(buf, 17, "%c%c%c%s%c%c%c%c%c", uiDataGlobal.FreqEnter.digits[0], uiDataGlobal.FreqEnter.digits[1], uiDataGlobal.FreqEnter.digits[2], (menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID) ? "" : ".",
						uiDataGlobal.FreqEnter.digits[3], uiDataGlobal.FreqEnter.digits[4], uiDataGlobal.FreqEnter.digits[5], uiDataGlobal.FreqEnter.digits[6], uiDataGlobal.FreqEnter.digits[7]);
				buf[(menuDataGlobal.currentItemIndex != CH_DETAILS_DMRID) ? ((uiDataGlobal.FreqEnter.index > 2) ? (uiDataGlobal.FreqEnter.index + 1) : uiDataGlobal.FreqEnter.index) : uiDataGlobal.FreqEnter.index] = 0;
				voicePromptsAppendString(buf);
				if (menuDataGlobal.currentItemIndex != CH_DETAILS_DMRID)
				{
					voicePromptsAppendPrompt(PROMPT_MEGAHERTZ);
				}
				voicePromptsPlay();

				updateScreen(false, true);

				return;
			}
		}

		if ((uiDataGlobal.FreqEnter.index < 8))
		{
			if (!BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				int keyval = menuGetKeypadKeyValue(ev, true);

				if ((keyval != 99) &&
						((menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID) || (((uiDataGlobal.FreqEnter.index == 0) && (keyval == 0)) == false)))
				{
					voicePromptsInit();
					voicePromptsAppendInteger(keyval);
					voicePromptsPlay();

					uiDataGlobal.FreqEnter.digits[uiDataGlobal.FreqEnter.index] = (char) keyval + '0';
					uiDataGlobal.FreqEnter.index++;

					if (uiDataGlobal.FreqEnter.index == 8)
					{
						int number = freqEnterRead(0, 8, (menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID));

						if (((menuDataGlobal.currentItemIndex == CH_DETAILS_RXFREQ) || (menuDataGlobal.currentItemIndex == CH_DETAILS_TXFREQ))
								&& (trxGetBandFromFrequency(number) != -1))
						{
							updateFrequency(number);
						}
						else if ((menuDataGlobal.currentItemIndex == CH_DETAILS_DMRID)
								&& (((number >= MIN_TG_OR_PC_VALUE) && (number <= MAX_TG_OR_PC_VALUE)) || (number == 0)))
						{
							codeplugChannelSetOptionalDMRID(number, &tmpChannel);
							freqEnterReset();
						}
						else
						{
							uiDataGlobal.FreqEnter.index--;
							uiDataGlobal.FreqEnter.digits[uiDataGlobal.FreqEnter.index] = '-';
							soundSetMelody(MELODY_ERROR_BEEP);
						}
					}
					updateScreen(false, true);
					return;
				}
			}
			else
			{
				if (KEYCHECK_PRESS_NUMBER(ev->keys))
				{
					soundSetMelody(MELODY_ERROR_BEEP);
				}
			}
		}
	}

	// Not entering a frequency numeric digit
	bool allowedToSpeakUpdate = true;

	if ((ev->events & KEY_EVENT) && (menuDataGlobal.menuOptionsSetQuickkey == 0) && (menuDataGlobal.menuOptionsTimeout == 0))
	{
		if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
		{
			menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, NUM_CH_DETAILS_ITEMS);
			updateScreen(false, true);
			menuChannelDetailsExitCode |= MENU_STATUS_LIST_TYPE;
		}
		else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
		{
			menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, NUM_CH_DETAILS_ITEMS);
			updateScreen(false, true);
			menuChannelDetailsExitCode |= MENU_STATUS_LIST_TYPE;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
		{
			if (strlen(channelName) == 0) // Do not permit empty names, like CPS does
			{
				menuDataGlobal.currentItemIndex = CH_DETAILS_NAME;
				nameInError = true;
				updateScreen(false, true);
				nameInError = false;
				menuChannelDetailsExitCode |= MENU_STATUS_ERROR;
			}
			else
			{
				saveChanges(ev);

				resetChannelData();
				menuSystemPopAllAndDisplayRootMenu();
			}
			return;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
		{
			resetChannelData();
			menuSystemPopPreviousMenu();
			return;
		}
		else if (KEYCHECK_SHORTUP_NUMBER(ev->keys) && BUTTONCHECK_DOWN(ev, BUTTON_SK2))
		{
			menuDataGlobal.menuOptionsSetQuickkey = ev->keys.key;
			updateScreen(false, false);
		}
	}

	if ((ev->events & (KEY_EVENT | FUNCTION_EVENT)) && (menuDataGlobal.menuOptionsSetQuickkey == 0))
	{
		if (KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_RIGHT))
		{
			if (menuDataGlobal.currentItemIndex == CH_DETAILS_NAME)
			{
				namePos = strlen(channelName);
				updateScreen(false, !voicePromptsIsPlaying());
			}
			else if ((menuDataGlobal.currentItemIndex == CH_DETAILS_RXCSS) || (menuDataGlobal.currentItemIndex == CH_DETAILS_TXCSS))
			{
				goto handlesRightKey;
			}
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT) || (QUICKKEY_FUNCTIONID(ev->function) == FUNC_RIGHT))
		{
			handlesRightKey:
			if (menuDataGlobal.menuOptionsTimeout > 0)
			{
				menuDataGlobal.menuOptionsTimeout = 1000;
			}

			switch(menuDataGlobal.currentItemIndex)
			{
				case CH_DETAILS_NAME:
					if (uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL)
					{
						moveCursorRightInString(channelName, &namePos, 16, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
						updateCursor(true);
						allowedToSpeakUpdate = (strlen(channelName) == 0);
					}
					break;
				case CH_DETAILS_MODE:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						tmpChannel.chMode = RADIO_MODE_ANALOG;
					}
					break;
				case CH_DETAILS_DMR_CC:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						if (tmpChannel.txColor < 15)
						{
							tmpChannel.txColor++;
							trxSetDMRColourCode(tmpChannel.txColor);
						}
					}
					break;
				case CH_DETAILS_DMR_TS:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						tmpChannel.flag2 |= 0x40;// set TS 2 bit
					}
					break;
				case CH_DETAILS_RXCSS:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						bool voicePromptWasPlaying = voicePromptsIsPlaying();

						cssIncrementFromEvent(ev, &tmpChannel.rxTone, &RxCSSIndex, &RxCSSType);
						trxSetRxCSS(tmpChannel.rxTone);
						announceCSSCode(tmpChannel.rxTone, RxCSSType,
								(((voicePromptWasPlaying == false) && (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_2)) ? DIRECTION_RECEIVE : DIRECTION_NONE),
								(voicePromptWasPlaying == false), AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
						allowedToSpeakUpdate = false;
					}
					break;
				case CH_DETAILS_TXCSS:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						bool voicePromptWasPlaying = voicePromptsIsPlaying();

						cssIncrementFromEvent(ev, &tmpChannel.txTone, &TxCSSIndex, &TxCSSType);
						announceCSSCode(tmpChannel.txTone, TxCSSType,
								(((voicePromptWasPlaying == false) && (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_2)) ? DIRECTION_TRANSMIT : DIRECTION_NONE),
								(voicePromptWasPlaying == false), AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
						allowedToSpeakUpdate = false;
					}
					break;
				case CH_DETAILS_BANDWIDTH:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						tmpChannel.flag4 |= 0x02;// set 25kHz bit
					}
					break;
				case CH_DETAILS_FREQ_STEP:
					tmpVal = (tmpChannel.VFOflag5 >> 4) + 1;
					if (tmpVal > 7)
					{
						tmpVal = 7;
					}
					tmpChannel.VFOflag5 &= 0x0F;
					tmpChannel.VFOflag5 |= tmpVal << 4;
					break;
				case CH_DETAILS_TOT:
					if (tmpChannel.tot < 255)
					{
						tmpChannel.tot++;
					}
					break;
				case CH_DETAILS_RXONLY:
					tmpChannel.flag4 |= 0x04;// set Channel RX-Only Bit
					break;
				case CH_DETAILS_ZONE_SKIP:
					tmpChannel.flag4 |= CODEPLUG_CHANNEL_FLAG_ZONE_SKIP;// set Channel Zone Skip bit (was Auto Scan)
					break;
				case CH_DETAILS_ALL_SKIP:
					tmpChannel.flag4 |= CODEPLUG_CHANNEL_FLAG_ALL_SKIP;// set Channel All Skip bit (was Lone Worker)
					break;
				case CH_DETAILS_RXGROUP:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						tmpVal = SAFE_MIN((tmpChannel.rxGroupList + 1), CODEPLUG_RX_GROUPLIST_MAX);

						while (tmpVal <= CODEPLUG_RX_GROUPLIST_MAX) // 1 .. CODEPLUG_RX_GROUPLIST_MAX, codeplugRxGroupGetDataForIndex() is using (index - 1)
						{
							if (codeplugRxGroupGetDataForIndex(tmpVal, &rxGroupBuf))
							{
								tmpChannel.rxGroupList = tmpVal;
								break;
							}
							tmpVal++;
						}
					}
					break;
				case CH_DETAILS_VOX:
					tmpChannel.flag4 |= 0x40;
					break;
				case CH_DETAILS_POWER:
					if ((uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL) &&
						(tmpChannel.libreDMR_Power < (MAX_POWER_SETTING_NUM + CODEPLUG_MIN_PER_CHANNEL_POWER)))
					{

							tmpChannel.libreDMR_Power++;
					}
					break;
				case CH_DETAILS_SQUELCH:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						if (tmpChannel.sql < CODEPLUG_MAX_VARIABLE_SQUELCH)
						{
							tmpChannel.sql++;
						}
					}
					break;

			}

			if (ev->events & FUNCTION_EVENT)
			{
				saveChanges(ev);
			}

			updateScreen(false, allowedToSpeakUpdate);
		}
		else if (KEYCHECK_LONGDOWN(ev->keys, KEY_LEFT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_LEFT))
		{
			if (menuDataGlobal.currentItemIndex == CH_DETAILS_NAME)
			{
				namePos = 0;
				updateScreen(false, !voicePromptsIsPlaying());
			}
			else if ((menuDataGlobal.currentItemIndex == CH_DETAILS_RXCSS) || (menuDataGlobal.currentItemIndex == CH_DETAILS_TXCSS))
			{
				goto handlesLeftKey;
			}
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_LEFT) || (QUICKKEY_FUNCTIONID(ev->function) == FUNC_LEFT))
		{
			handlesLeftKey:
			if (menuDataGlobal.menuOptionsTimeout > 0)
			{
				menuDataGlobal.menuOptionsTimeout = 1000;
			}

			switch(menuDataGlobal.currentItemIndex)
			{
				case CH_DETAILS_NAME:
					if (uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL)
					{
						moveCursorLeftInString(channelName, &namePos, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
						updateCursor(true);
						allowedToSpeakUpdate = (strlen(channelName) == 0);
					}
					break;
				case CH_DETAILS_MODE:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						tmpChannel.chMode = RADIO_MODE_DIGITAL;
						tmpChannel.flag4 &= ~0x02;// clear 25kHz bit
					}
					break;
				case CH_DETAILS_DMR_CC:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						if (tmpChannel.txColor > 0)
						{
							tmpChannel.txColor--;
							trxSetDMRColourCode(tmpChannel.txColor);
						}
					}
					break;
				case CH_DETAILS_DMR_TS:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						tmpChannel.flag2 &= 0xBF;// Clear TS 2 bit
					}
					break;
				case CH_DETAILS_RXCSS:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						bool voicePromptWasPlaying = voicePromptsIsPlaying();

						cssDecrementFromEvent(ev, &tmpChannel.rxTone, &RxCSSIndex, &RxCSSType);
						trxSetRxCSS(tmpChannel.rxTone);
						announceCSSCode(tmpChannel.rxTone, RxCSSType,
								(((voicePromptWasPlaying == false) && (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_2)) ? DIRECTION_RECEIVE : DIRECTION_NONE),
								(voicePromptWasPlaying == false), AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
						allowedToSpeakUpdate = false;
					}
					break;
				case CH_DETAILS_TXCSS:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						bool voicePromptWasPlaying = voicePromptsIsPlaying();

						cssDecrementFromEvent(ev, &tmpChannel.txTone, &TxCSSIndex, &TxCSSType);
						announceCSSCode(tmpChannel.txTone, TxCSSType,
								(((voicePromptWasPlaying == false) && (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_2)) ? DIRECTION_TRANSMIT : DIRECTION_NONE),
								(voicePromptWasPlaying == false), AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
						allowedToSpeakUpdate = false;
					}
					break;
				case CH_DETAILS_BANDWIDTH:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						tmpChannel.flag4 &= ~0x02;// clear 25kHz bit
					}
					break;
				case CH_DETAILS_FREQ_STEP:
					tmpVal = (tmpChannel.VFOflag5 >> 4) - 1;
					if (tmpVal < 0)
					{
						tmpVal = 0;
					}
					tmpChannel.VFOflag5 &= 0x0F;
					tmpChannel.VFOflag5 |= tmpVal << 4;
					break;
				case CH_DETAILS_TOT:
					if (tmpChannel.tot > 0)
					{
						tmpChannel.tot--;
					}
					break;
				case CH_DETAILS_RXONLY:
					tmpChannel.flag4 &= ~0x04;// clear Channel RX-Only Bit
					break;
				case CH_DETAILS_ZONE_SKIP:
					tmpChannel.flag4 &= ~CODEPLUG_CHANNEL_FLAG_ZONE_SKIP;// clear Channel Zone Skip Bit (was Auto Scan bit)
					break;
				case CH_DETAILS_ALL_SKIP:
					tmpChannel.flag4 &= ~CODEPLUG_CHANNEL_FLAG_ALL_SKIP;// clear Channel All Skip Bit (was Lone Worker bit)
					break;
				case CH_DETAILS_RXGROUP:
					if (tmpChannel.chMode == RADIO_MODE_DIGITAL)
					{
						tmpVal = tmpChannel.rxGroupList;

						tmpVal = SAFE_MAX((tmpVal - 1), 0);

						if (tmpVal == 0)
						{
							tmpChannel.rxGroupList = tmpVal;
						}
						else
						{
							while (tmpVal > 0)
							{
								if (codeplugRxGroupGetDataForIndex(tmpVal, &rxGroupBuf))
								{
									tmpChannel.rxGroupList = tmpVal;
									break;
								}
								tmpVal--;
							}
						}
					}
					break;
				case CH_DETAILS_VOX:
					tmpChannel.flag4 &= ~0x40;
					break;
				case CH_DETAILS_POWER:
					if ((uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL) &&
						(tmpChannel.libreDMR_Power > 0))
					{
						tmpChannel.libreDMR_Power--;
					}
					break;
				case CH_DETAILS_SQUELCH:
					if (tmpChannel.chMode == RADIO_MODE_ANALOG)
					{
						if (tmpChannel.sql >= CODEPLUG_MIN_VARIABLE_SQUELCH) // Here, we permit to reach 0 value, aka global squelch setting.
						{
							tmpChannel.sql--;
						}
					}
					break;
			}

			if (ev->events & FUNCTION_EVENT)
			{
				saveChanges(ev);
			}

			updateScreen(false, allowedToSpeakUpdate);
		}
		else if (!BUTTONCHECK_DOWN(ev, BUTTON_SK2) && (menuDataGlobal.currentItemIndex == CH_DETAILS_NAME) && (uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL))
		{
			// vk3kyy - Commented this out, as BUTTON_SK2 is now changed in the enclosing if, as this was needed so that QuicKeys worked (see also next comment below)
			//if (!BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				if ((ev->keys.event == KEY_MOD_PREVIEW) && (namePos < 16))
				{
					channelName[namePos] = ev->keys.key;
					updateCursor(true);
					announceChar(ev->keys.key);
					updateScreen(false, false);
				}
				else if ((ev->keys.event == KEY_MOD_PRESS) && (namePos < 16))
				{
					channelName[namePos] = ev->keys.key;
					if ((namePos < strlen(channelName)) && (namePos < 15))
					{
						namePos++;
					}
					updateCursor(true);
					announceChar(ev->keys.key);
					updateScreen(false, false);
				}
			}
			/*
			 * vk3kyy - needed to stop this code because it interferes with the QuickKeys and saveing of the channel SK2 + Green
			 * And I don't know what it was intended to do.
			else
			{
				keyboardReset();
				keypadAlphaEnable = true;
				soundSetMelody(MELODY_ERROR_BEEP);
			}*/
		}
		else if ((ev->keys.event & KEY_MOD_PRESS) && (menuDataGlobal.menuOptionsTimeout > 0))
		{
			menuDataGlobal.menuOptionsTimeout = 0;
			resetChannelData();
			menuSystemPopPreviousMenu();
			return;
		}
	}

	if ((ev->events & KEY_EVENT) && (menuDataGlobal.menuOptionsSetQuickkey != 0) && (menuDataGlobal.menuOptionsTimeout == 0))
	{
		if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
		{
			menuDataGlobal.menuOptionsSetQuickkey = 0;
			menuDataGlobal.menuOptionsTimeout = 0;
			menuChannelDetailsExitCode |= MENU_STATUS_ERROR;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
		{
			saveQuickkeyMenuIndex(menuDataGlobal.menuOptionsSetQuickkey, menuSystemGetCurrentMenuNumber(), menuDataGlobal.currentItemIndex, 0);
			menuDataGlobal.menuOptionsSetQuickkey = 0;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_LEFT))
		{
			saveQuickkeyMenuIndex(menuDataGlobal.menuOptionsSetQuickkey, menuSystemGetCurrentMenuNumber(), menuDataGlobal.currentItemIndex, FUNC_LEFT);
			menuDataGlobal.menuOptionsSetQuickkey = 0;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT))
		{
			saveQuickkeyMenuIndex(menuDataGlobal.menuOptionsSetQuickkey, menuSystemGetCurrentMenuNumber(), menuDataGlobal.currentItemIndex, FUNC_RIGHT);
			menuDataGlobal.menuOptionsSetQuickkey = 0;
		}
		updateScreen(false, true);
	}
}

static void cssIncrementFromEvent(uiEvent_t *ev, uint16_t *tone, uint8_t *index, CodeplugCSSTypes_t *type)
{
	if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
	{
		switch (*type)
		{
			case CSS_TYPE_CTCSS:
				if (*index < (TRX_NUM_CTCSS - 1))
				{
					*index = (TRX_NUM_CTCSS - 1);
					*tone = TRX_CTCSSTones[*index];
				}
				else
				{
					*type = CSS_TYPE_DCS;
					*index = 0;
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS;
				}
				break;
			case CSS_TYPE_DCS:
				if (*index < (TRX_NUM_DCS - 1))
				{
					*index = (TRX_NUM_DCS - 1);
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS;
				}
				else
				{
					*type = (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED);
					*index = 0;
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED;
				}
				break;
			case (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED):
				if (*index < (TRX_NUM_DCS - 1))
				{
					*index = (TRX_NUM_DCS - 1);
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED;
				}
				break;
			case CSS_TYPE_NONE:
				*type = CSS_TYPE_CTCSS;
				*index = 0;
				*tone = TRX_CTCSSTones[*index];
				break;
			default:
				break;
		}
	}
	else
	{
		// Step +5, cssIncrement() handles index overflow
		cssIncrement(tone, index, ((ev->keys.event & KEY_MOD_LONG) ? 5 : 1), type, false, false);
	}
}

static void cssDecrementFromEvent(uiEvent_t *ev, uint16_t *tone, uint8_t *index, CodeplugCSSTypes_t *type)
{
	if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
	{
		switch (*type)
		{
			case CSS_TYPE_CTCSS:
				if (*index > 0)
				{
					*index = 0;
					*tone = TRX_CTCSSTones[*index];
				}
				else
				{
					*type = CSS_TYPE_NONE;
					*index = 0;
					*tone = CODEPLUG_CSS_TONE_NONE;
				}
				break;
			case CSS_TYPE_DCS:
				if (*index > 0)
				{
					*index = 0;
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS;
				}
				else
				{
					*type = CSS_TYPE_CTCSS;
					*index = (TRX_NUM_CTCSS - 1);
					*tone = TRX_CTCSSTones[*index];
				}
				break;
			case (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED):
				if (*index > 0)
				{
					*index = 0;
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED;
				}
				else
				{
					*type = CSS_TYPE_DCS;
					*index = (TRX_NUM_DCS - 1);
					*tone = TRX_DCSCodes[*index] | CSS_TYPE_DCS;
				}
				break;
			default:
				break;
		}
	}
	else
	{
		// Step -5 on long press, cssDecrement() handles index < 0
		cssDecrement(tone, index, ((ev->keys.event & KEY_MOD_LONG) ? 5 : 1), type, false, false);
	}
}

static void saveChanges(uiEvent_t *ev)
{
	if (uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL)
	{
		codeplugUtilConvertStringToBuf(channelName, (char *)&tmpChannel.name, 16);
	}
	memcpy(currentChannelData, &tmpChannel, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE); // Leave channel's NOT_IN_CODEPLUG_flag out of the copy

	// uiDataGlobal.currentSelectedChannelNumber is -1 when in VFO mode
	// But the VFO is stored in the nonVolatile settings, and not saved back to the codeplug
	// Also don't store this back to the codeplug unless the Function key (Blue / SK2 ) is pressed at the same time.
	if (!(ev->events & FUNCTION_EVENT) && ((uiDataGlobal.currentSelectedChannelNumber != CH_DETAILS_VFO_CHANNEL) && BUTTONCHECK_DOWN(ev, BUTTON_SK2)))
	{
		codeplugChannelSaveDataForIndex(uiDataGlobal.currentSelectedChannelNumber, currentChannelData);
	}

	if ((uiDataGlobal.currentSelectedChannelNumber == CH_DETAILS_VFO_CHANNEL) || (currentChannelData->libreDMR_Power == 0))
	{
		trxSetPowerFromLevel(nonVolatileSettings.txPowerLevel);
	}
	else
	{
		trxSetPowerFromLevel(currentChannelData->libreDMR_Power - 1);
	}

	settingsSetVFODirty();
	settingsSaveIfNeeded(true);
}

static void resetChannelData(void)
{
	memcpy(tmpChannel.name, CHANNEL_UNSET, 5);
}
