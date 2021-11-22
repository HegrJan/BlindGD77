/*
 * Copyright (C) 2021 Joseph Stephen vk7js
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
#include <ctype.h>
#include "functions/codeplug.h"
#include "functions/settings.h"
#include "functions/trx.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"
#include "user_interface/editHandler.h"

static void updateScreen(bool isFirstRun, bool updateScreen);
static void handleEvent(uiEvent_t *ev);

static menuStatus_t menuRadioDetailsExitCode = MENU_STATUS_SUCCESS;

enum DETAILS_DISPLAY_LIST { DETAILS_CALLSIGN=0, DETAILS_DMRID, DETAILS_LINE1, DETAILS_LINE2, NUM_DETAILS_ITEMS };// The last item in the list is used so that we automatically get a total number of items in the list
// four lines, callsign, DMRID, boot line 1, boot line 2.
static char userInfo[NUM_DETAILS_ITEMS][SCREEN_LINE_BUFFER_SIZE+1];
static int xOffsetForEditableMenuItems[NUM_DETAILS_ITEMS]; // for cursor 
static int userInfoCursorPositions[NUM_DETAILS_ITEMS];

static void SetEditParamsForMenuIndex()
{
		bool curFieldIsNumeric=menuDataGlobal.currentItemIndex == DETAILS_DMRID;
		keypadAlphaEnable = !curFieldIsNumeric;

	editParams.editFieldType=curFieldIsNumeric ? EDIT_TYPE_NUMERIC : EDIT_TYPE_ALPHANUMERIC;
	editParams.editBuffer= userInfo[menuDataGlobal.currentItemIndex];
	editParams.cursorPos=&userInfoCursorPositions[menuDataGlobal.currentItemIndex];
	editParams.maxLen= ((menuDataGlobal.currentItemIndex == DETAILS_CALLSIGN) || curFieldIsNumeric) ? 9 : SCREEN_LINE_BUFFER_SIZE+1;
	editParams.xPixelOffset=xOffsetForEditableMenuItems[menuDataGlobal.currentItemIndex]*8; // font size 3.
	editParams.yPixelOffset=0; // use default menu calculation.
	*editParams.cursorPos = SAFE_MIN(strlen(editParams.editBuffer), editParams.maxLen-1); // place cursor at end.
}

menuStatus_t menuRadioDetails(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		memset(userInfo, 0, sizeof(userInfo));
		memset(userInfoCursorPositions, 0, sizeof(userInfoCursorPositions));
		memset(xOffsetForEditableMenuItems, 0,sizeof(xOffsetForEditableMenuItems));
		
		codeplugGetRadioName(userInfo[DETAILS_CALLSIGN]);
		if (codeplugGetUserDMRID())
		{
			snprintf(userInfo[DETAILS_DMRID], SCREEN_LINE_BUFFER_SIZE, "%u", codeplugGetUserDMRID());
		}
		else
		{
			userInfo[DETAILS_DMRID][0]='\0';
		}
		uint8_t bootScreenType;
		codeplugGetBootScreenData(userInfo[DETAILS_LINE1], userInfo[DETAILS_LINE2], &bootScreenType);
		
		voicePromptsInit();
		menuDataGlobal.currentItemIndex = DETAILS_CALLSIGN;
		menuDataGlobal.endIndex = NUM_DETAILS_ITEMS;
		SetEditParamsForMenuIndex();

		voicePromptsAppendLanguageString(&currentLanguage->user_info);
		voicePromptsAppendLanguageString(&currentLanguage->menu);
	
		updateScreen(isFirstRun, true);
		editUpdateCursor(&editParams, true, true);
		
		return (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuRadioDetailsExitCode = MENU_STATUS_SUCCESS;
		
		editUpdateCursor(&editParams, false, true);

		if (ev->hasEvent)
		{
			handleEvent(ev);
		}
	}

	return menuRadioDetailsExitCode;
}

static void updateScreen(bool isFirstRun, bool allowedToSpeakUpdate)
{
	int mNum = 0;
	char buf[SCREEN_LINE_BUFFER_SIZE];
	char * const *leftSide = NULL;// initialise to please the compiler
	char * const *leftSideConst = NULL;// initialise to please the compiler
	voicePrompt_t leftSidePrompt;
	char * const *rightSideConst = NULL;// initialise to please the compiler
	char rightSideVar[SCREEN_LINE_BUFFER_SIZE];
	voicePrompt_t rightSidePrompt;

	ucClearBuf();

	menuDisplayTitle(currentLanguage->user_info);

	for(int i = -1; i <= 1; i++)
	{
		mNum = menuGetMenuOffset(menuDataGlobal.endIndex, i);

		buf[0] = 0;
		leftSide = NULL;
		leftSideConst = NULL;
		leftSidePrompt = PROMPT_SILENCE;
		rightSideConst = NULL;
		rightSideVar[0] = 0;
		rightSidePrompt = PROMPT_SILENCE;// use PROMPT_SILENCE as flag that the unit has not been set

		switch (mNum)
		{
			case DETAILS_CALLSIGN:
				leftSide = (char * const *)&currentLanguage->callsign;
				strncpy(rightSideVar, userInfo[DETAILS_CALLSIGN], SCREEN_LINE_BUFFER_SIZE);
				break;
			case DETAILS_DMRID:
				leftSide = (char * const *)&currentLanguage->dmr_id;
				strncpy(rightSideVar, userInfo[DETAILS_DMRID], SCREEN_LINE_BUFFER_SIZE);
				break;
			default: // line 1 and line 2 of boot text.
				snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%d:%s", mNum-1, userInfo[mNum]);
				break;
		}

		if (leftSide != NULL)
		{
			snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%s:%s", *leftSide, (rightSideVar[0] ? rightSideVar : (rightSideConst ? *rightSideConst : "")));
			xOffsetForEditableMenuItems[mNum]=strlen(*leftSide)+1;
		}
		else
		{
			strcpy(buf, rightSideVar);
			xOffsetForEditableMenuItems[mNum]=2;
		}
		if ((i == 0) && allowedToSpeakUpdate)
		{
			if (!isFirstRun)
			{
				voicePromptsInit();
			}
			if (leftSidePrompt != PROMPT_SILENCE)
			{
				voicePromptsAppendPrompt(leftSidePrompt);
				voicePromptsAppendPrompt(PROMPT_SILENCE);
			}
			else if ((leftSideConst != NULL) || (leftSide != NULL))
			{
				voicePromptsAppendLanguageString((const char * const *) (leftSideConst ? leftSideConst : leftSide));
			}
			if (rightSidePrompt != PROMPT_SILENCE)
			{
				voicePromptsAppendPrompt(rightSidePrompt);
			}
			else if (rightSideConst != NULL)
			{
				voicePromptsAppendLanguageString((const char * const *)rightSideConst);
			}
			else if (rightSideVar[0] != 0)
			{
				voicePromptsAppendString(rightSideVar);
			}

			promptsPlayNotAfterTx();
		}
		if (!*buf)
			strcpy(buf, " ");
		menuDisplayEntry(i, mNum, buf);
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
	if (ev->events & KEY_EVENT)
	{
		if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
		{
			menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, NUM_DETAILS_ITEMS);
			SetEditParamsForMenuIndex();
			updateScreen(false, true);
			menuRadioDetailsExitCode |= MENU_STATUS_LIST_TYPE;
		}
		else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
		{
			menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, NUM_DETAILS_ITEMS);
			SetEditParamsForMenuIndex();
			updateScreen(false, true);
			menuRadioDetailsExitCode |= MENU_STATUS_LIST_TYPE;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
		{
			// save buffers.
			uint32_t newDmrID=atol(userInfo[DETAILS_DMRID]);
			if (newDmrID!=codeplugGetUserDMRID())
			{
				trxDMRID=newDmrID;
				codeplugSetUserDMRID(trxDMRID);
				uiDataGlobal.userDMRId = trxDMRID;
			}
			//joe
			codeplugSetRadioName(userInfo[DETAILS_CALLSIGN]);
			codeplugSetBootScreenData(userInfo[DETAILS_LINE1], userInfo[DETAILS_LINE2]);
			menuSystemPopAllAndDisplayRootMenu();
			return;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
		{
			menuSystemPopPreviousMenu();
			return;
		}		
		else if (HandleEditEvent(ev, &editParams))
		{
			updateScreen(false, editParams.allowedToSpeakUpdate);
		return; 
		}	
	}
}