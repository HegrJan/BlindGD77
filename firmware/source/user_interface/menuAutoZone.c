/*
 * Copyright (C) 2021 Joseph Stephen, VK7JS
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
#include "main.h"
#include "functions/settings.h"
#include "functions/autozone.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"

static void updateScreen(bool isFirstRun);
static void handleEvent(uiEvent_t *ev);
static uint8_t autoZonesEnabled=0;
static menuStatus_t menuAutoZoneExitCode = MENU_STATUS_SUCCESS;

menuStatus_t menuAutoZone(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		menuDataGlobal.endIndex = AutoZone_TYPE_MAX-1;
		autoZonesEnabled=nonVolatileSettings.autoZonesEnabled;
		
		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->autoZone);
		if (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_2)
		{
			voicePromptsAppendLanguageString(&currentLanguage->menu);
		}
		voicePromptsAppendPrompt(PROMPT_SILENCE);

		updateScreen(true);
		return (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuAutoZoneExitCode = MENU_STATUS_SUCCESS;

		if (ev->hasEvent)
		{
			handleEvent(ev);
		}
	}
	return menuAutoZoneExitCode;
}

static void updateScreen(bool isFirstRun)
{
	char buf[SCREEN_LINE_BUFFER_SIZE];

	int mNum;
	struct_AutoZoneParams_t autoZone;
	
	ucClearBuf();
	menuDisplayTitle(currentLanguage->autoZone);

	for(int i = -1; i <= 1; i++)
	{
		mNum = menuGetMenuOffset(menuDataGlobal.endIndex, i);
		
		AutoZoneGetData(mNum+1, &autoZone);
		snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%s:%s", (char*)autoZone.name, IsBitSet(autoZonesEnabled, mNum+1) ? currentLanguage->on : currentLanguage->off);

		menuDisplayEntry(i, mNum, (char *)buf);

		if (i == 0)
		{
			if (!isFirstRun)
			{
				voicePromptsInit();
			}

			voicePromptsAppendString((char*)autoZone.name);
			if (IsBitSet(autoZonesEnabled, mNum+1))
				voicePromptsAppendLanguageString(&currentLanguage->on);
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
				
			promptsPlayNotAfterTx();

		}
	}

	ucRender();
}

static void handleEvent(uiEvent_t *ev)
{
	if (ev->events & BUTTON_EVENT)
	{
		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}
	}

	if (ev->events & FUNCTION_EVENT)
	{
		if ((QUICKKEY_TYPE(ev->function) == QUICKKEY_MENU) && (QUICKKEY_LONGENTRYID(ev->function) > 0) && (QUICKKEY_LONGENTRYID(ev->function) <= menuDataGlobal.endIndex))
		{
			menuDataGlobal.currentItemIndex = QUICKKEY_LONGENTRYID(ev->function)-1;
		}
		return;
	}

	if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
	{
		menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, menuDataGlobal.endIndex);
		updateScreen(false);
		menuAutoZoneExitCode |= MENU_STATUS_LIST_TYPE;
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
	{
		menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, menuDataGlobal.endIndex);
		updateScreen(false);
		menuAutoZoneExitCode |= MENU_STATUS_LIST_TYPE;
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_LEFT))
	{
		if (IsBitSet(autoZonesEnabled, menuDataGlobal.currentItemIndex+1)) // menu index is 0-based
			SetBit(&autoZonesEnabled, menuDataGlobal.currentItemIndex+1, false);
		updateScreen(false);
		menuAutoZoneExitCode |= MENU_STATUS_LIST_TYPE;
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_RIGHT))
	{
		if (!IsBitSet(autoZonesEnabled, menuDataGlobal.currentItemIndex+1))
			SetBit(&autoZonesEnabled, menuDataGlobal.currentItemIndex+1, true);
		updateScreen(false);
		menuAutoZoneExitCode |= MENU_STATUS_LIST_TYPE;
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
	{
		nonVolatileSettings.autoZonesEnabled=autoZonesEnabled;
		if(autoZonesEnabled==0)
			autoZone.flags=0;
		if (AutoZoneIsCurrentZone(currentZone.NOT_IN_CODEPLUGDATA_indexNumber))
		{
			currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
			if (autoZonesEnabled==0)
				nonVolatileSettings.currentZone = 0;
		}
		settingsSetDirty();
		menuSystemPopAllAndDisplayRootMenu();
		return;
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
	{
		menuSystemPopPreviousMenu();
		return;
	}
	else if (KEYCHECK_SHORTUP_NUMBER(ev->keys) && BUTTONCHECK_DOWN(ev, BUTTON_SK2))
	{
		saveQuickkeyMenuLongValue(ev->keys.key, menuSystemGetCurrentMenuNumber(), menuDataGlobal.currentItemIndex + 1);
		return;
	}
}
