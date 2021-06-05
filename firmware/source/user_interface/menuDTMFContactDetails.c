/*
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *
 * Joseph Stephen VK7JS
 * Jan Hegr OK1TE
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

#define DTMF_CODE_MAX_LEN 16 // should match the max size of the struct_codeplugDTMFContact_t .code field in codeplug.h.
#define DTMF_NAME_MAX_LEN 16 // should match the max size of the struct_codeplugDTMFContact_t .name field in codeplug.h.
#define MAX_CHAR_BUF_LEN 33 // include null.

static void updateScreen(bool isFirstRun, bool updateScreen);
static void updateCursor(bool moved);
static void handleEvent(uiEvent_t *ev);

static menuStatus_t menuDTMFContactDetailsExitCode = MENU_STATUS_SUCCESS;

static struct_codeplugDTMFContact_t tmpDTMFContact;

static int dtmfContactDetailsIndex;
static int dtmfContactCount;

static char dtmfCodeChars[DTMF_CODE_MAX_LEN+1]; // DTMF may have 16 chars plus a null.
static char contactName[DTMF_NAME_MAX_LEN+1];

static int namePos;
static int codePos;

static bool createNew;

enum DTMF_CONTACT_DETAILS_DISPLAY_LIST { CONTACT_DETAILS_NAME=0, CONTACT_DETAILS_DTMF_CODE, NUM_DTMF_CONTACT_DETAILS_ITEMS };// The last item in the list is used so that we automatically get a total number of items in the list

static int menuDTMFContactDetailsState;
static int menuDTMFContactDetailsTimeout;

enum MENU_DTMF_CONTACT_DETAILS_STATE { MENU_DTMF_CONTACT_DETAILS_DISPLAY = 0, MENU_DTMF_CONTACT_DETAILS_SAVED, MENU_DTMF_CONTACT_DETAILS_EXISTS };

static bool DTMFContactExists(char* name)
{
	if (!name || !*name) return false;

	int count=codeplugDTMFContactsGetCount();

	for (int index= 1; index <= count; ++index)
	{
		char buf[DTMF_NAME_MAX_LEN];
		struct_codeplugDTMFContact_t record;

		if (codeplugDTMFContactGetDataForIndex(index, &record))
		{
			buf[0]='\0';
			codeplugUtilConvertBufToString(record.name,buf, DTMF_NAME_MAX_LEN);
			if (strcmp(name, buf)==0)
			{
				return true;
			}
		}
	}

	return false;
	}

static const char *DTMF_AllowedChars = "0123456789ABCD*#"; // The order is mandatory

static bool ConvertDTMFToChars(uint8_t *code, char *text, int maxSize)
{
	if (!text || !code)
	{
		return false;
	}
	int j=0;

	for (int i = 0; i < maxSize; i++)
	{
		if (code[i] < 16)
			text[j++] = DTMF_AllowedChars[code[i]];
	}
	text[j] = 0;

	return true;
}

extern int toupper(int __c);

static bool ConvertCharsToDTMFCode(char *text, uint8_t *code, int maxSize)
{// initialize to empty.
	if (!text || !*text || !code)
	{
		return false;
	}

	memset(code, 0xFFU, DTMF_CODE_MAX_LEN);
	for (int i = 0; (i < maxSize) && text[i]; i++)
	{
		char *symbol = strchr(DTMF_AllowedChars, toupper(text[i]));
		if (!symbol)
		{
			return false;
		}
		code[i] = (symbol - DTMF_AllowedChars);
	}

	return true;
}

menuStatus_t menuDTMFContactDetails(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		voicePromptsInit();

		if (uiDataGlobal.currentSelectedContactIndex == 0)
		{// new contact
			dtmfContactCount = codeplugDTMFContactsGetCount();
			dtmfContactDetailsIndex = dtmfContactCount ? codeplugDTMFContactGetFreeIndex() : dtmfContactCount + 1;

			memset(tmpDTMFContact.name, 0xFFU, DTMF_NAME_MAX_LEN);
			memset(tmpDTMFContact.code, 0xFFU, DTMF_CODE_MAX_LEN);
			// Must clear out these buffers as they are reused between new and edit and artifacts can cause a new contact to inherit content from the last edited data.
			memset(contactName, 0, DTMF_NAME_MAX_LEN);
			memset(dtmfCodeChars, 0, DTMF_CODE_MAX_LEN);

			namePos = 0;
			codePos = 0;

			createNew=true;

			voicePromptsAppendLanguageString(&currentLanguage->new_contact);
		}
		else
		{// editing existing.
			dtmfContactDetailsIndex = uiDataGlobal.currentSelectedContactIndex;

			memcpy(&tmpDTMFContact, &contactListDTMFContactData, CODEPLUG_DTMF_CONTACT_DATA_STRUCT_SIZE);

			ConvertDTMFToChars(tmpDTMFContact.code, dtmfCodeChars, DTMF_CODE_MAX_LEN); // convert the dtmfCode to numbers and letters.
			codeplugUtilConvertBufToString(tmpDTMFContact.name, contactName, DTMF_NAME_MAX_LEN);

			namePos = strlen(contactName);
			codePos = strlen(dtmfCodeChars);

			createNew=false;
		}

		menuDTMFContactDetailsState = MENU_DTMF_CONTACT_DETAILS_DISPLAY;
		menuDataGlobal.currentItemIndex = CONTACT_DETAILS_NAME;
		menuDataGlobal.endIndex = NUM_DTMF_CONTACT_DETAILS_ITEMS;

		updateScreen(isFirstRun, true);
		updateCursor(true);

		return (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuDTMFContactDetailsExitCode = MENU_STATUS_SUCCESS;

		updateCursor(false);
		if (ev->hasEvent || (menuDTMFContactDetailsTimeout > 0))
		{
			handleEvent(ev);
		}
	}
	return menuDTMFContactDetailsExitCode;
}

static void updateCursor(bool moved)
{
	switch (menuDataGlobal.currentItemIndex)
	{
		case CONTACT_DETAILS_NAME:
			menuUpdateCursor(namePos, moved, true);
			break;
		case CONTACT_DETAILS_DTMF_CODE:
			menuUpdateCursor(codePos, moved, true);
			break;
	}
}

static void updateScreen(bool isFirstRun, bool allowedToSpeakUpdate)
{
	int mNum = 0;
	char buf[MAX_CHAR_BUF_LEN];
	char * const *leftSide = NULL;// initialise to please the compiler
	char * const *leftSideConst = NULL;// initialise to please the compiler
	voicePrompt_t leftSidePrompt;
	char * const *rightSideConst = NULL;// initialise to please the compiler
	char rightSideVar[DTMF_NAME_MAX_LEN];
	voicePrompt_t rightSidePrompt;

	ucClearBuf();

	if (tmpDTMFContact.name[0] == 0xFFU || tmpDTMFContact.name[0]==0) // Could be buff format or string format.
	{
		snprintf(buf, MAX_CHAR_BUF_LEN, "%s", currentLanguage->new_contact);
	}
	else
	{
		codeplugUtilConvertBufToString(tmpDTMFContact.name, buf, DTMF_NAME_MAX_LEN);
	}

	menuDisplayTitle(buf);

	if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_NAME)
	{
		keypadAlphaEnable = true;
	}
	else
	{
		keypadAlphaEnable = false;
	}

	switch (menuDTMFContactDetailsState)
	{
		case MENU_DTMF_CONTACT_DETAILS_DISPLAY:
			// Can only display 3 of the options at a time menu at -1, 0 and +1
			for(int i = -1; i <= 1; i++)
			{
				if (menuDataGlobal.endIndex <= (i + 1))
				{
					break;
				}

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
					case CONTACT_DETAILS_NAME:
						leftSide = (char * const *)&currentLanguage->name;
						strncpy(rightSideVar, contactName, DTMF_NAME_MAX_LEN);
						break;
					case CONTACT_DETAILS_DTMF_CODE:
						leftSide = (char * const *)&currentLanguage->dtmfCode;
						strncpy(rightSideVar, dtmfCodeChars, DTMF_CODE_MAX_LEN);
						break;
				}

				if (leftSide != NULL)
				{
					snprintf(buf, MAX_CHAR_BUF_LEN, "%s:%s", *leftSide, (rightSideVar[0] ? rightSideVar : (rightSideConst ? *rightSideConst : "")));
				}
				else
				{
					strcpy(buf, rightSideVar);
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
			break;
		case MENU_DTMF_CONTACT_DETAILS_SAVED:
			ucPrintCentered(16, currentLanguage->contact_saved, FONT_SIZE_3);
			ucDrawChoice(CHOICE_OK, false);
			break;
		case MENU_DTMF_CONTACT_DETAILS_EXISTS:
			ucPrintCentered(16, currentLanguage->duplicate, FONT_SIZE_3);
			ucPrintCentered(32, currentLanguage->contact, FONT_SIZE_3);
			ucDrawChoice(CHOICE_OK, false);
			break;
	}
	ucRender();
}

static void handleEvent(uiEvent_t *ev)
{
	int sLen = strlen(dtmfCodeChars);

	if (ev->events & BUTTON_EVENT)
	{
		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}
	}

	// Not entering a frequency numeric digit
	bool allowedToSpeakUpdate = true;

	switch (menuDTMFContactDetailsState)
	{
		case MENU_DTMF_CONTACT_DETAILS_DISPLAY:
			if (ev->events & KEY_EVENT)
			{
				if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
				{
					menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, NUM_DTMF_CONTACT_DETAILS_ITEMS);
					updateScreen(false, true);
					menuDTMFContactDetailsExitCode |= MENU_STATUS_LIST_TYPE;
				}
				else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
				{
					menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, NUM_DTMF_CONTACT_DETAILS_ITEMS);
					updateScreen(false, true);
					menuDTMFContactDetailsExitCode |= MENU_STATUS_LIST_TYPE;
				}
				else if (KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_RIGHT))
				{
					if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_NAME)
					{
						namePos = strlen(contactName);
						updateScreen(false, !voicePromptsIsPlaying());
					}
					else if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_DTMF_CODE)
					{
						codePos = strlen(dtmfCodeChars);
						updateScreen(false, !voicePromptsIsPlaying());
					}
				}
				else if (KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT))
				{
					switch(menuDataGlobal.currentItemIndex)
					{
						case CONTACT_DETAILS_NAME:
							moveCursorRightInString(contactName, &namePos, DTMF_NAME_MAX_LEN, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
							updateCursor(true);
							allowedToSpeakUpdate = false;
							break;
						case CONTACT_DETAILS_DTMF_CODE:
							moveCursorRightInString(dtmfCodeChars, &codePos, DTMF_CODE_MAX_LEN, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
							updateCursor(true);
							allowedToSpeakUpdate = false;
							break;
					}
					updateScreen(false, allowedToSpeakUpdate);
				}
				else if (KEYCHECK_LONGDOWN(ev->keys, KEY_LEFT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_LEFT))
				{
					if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_NAME)
					{
						namePos = 0;
						updateScreen(false, !voicePromptsIsPlaying());
					}
					else if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_DTMF_CODE)
					{
						codePos = 0;
						updateScreen(false, !voicePromptsIsPlaying());
					}
				}
				else if (KEYCHECK_SHORTUP(ev->keys, KEY_LEFT))
				{
					switch(menuDataGlobal.currentItemIndex)
					{
						case CONTACT_DETAILS_NAME:
							moveCursorLeftInString(contactName, &namePos, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
							updateCursor(true);
							allowedToSpeakUpdate = false;
							break;
						case CONTACT_DETAILS_DTMF_CODE:
							moveCursorLeftInString(dtmfCodeChars, &codePos, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
							updateCursor(true);
							allowedToSpeakUpdate = false;
							break;
					}
					updateScreen(false, allowedToSpeakUpdate);
				}
				else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
				{
					// Check existance.
					if (createNew && (dtmfContactCount > 0) && DTMFContactExists(contactName))
					{
						menuDTMFContactDetailsState=MENU_DTMF_CONTACT_DETAILS_EXISTS;
						menuDTMFContactDetailsTimeout = 2000;
						voicePromptsInit();
						voicePromptsAppendLanguageString(&currentLanguage->duplicate);
						voicePromptsPlay();

						updateScreen(false, allowedToSpeakUpdate);
						break;
					}

					memset(tmpDTMFContact.name, 0xFFU, DTMF_NAME_MAX_LEN);
					memset(tmpDTMFContact.code, 0xFFU, DTMF_CODE_MAX_LEN);

					codeplugUtilConvertStringToBuf(contactName, tmpDTMFContact.name, DTMF_NAME_MAX_LEN);
					ConvertCharsToDTMFCode(dtmfCodeChars, tmpDTMFContact.code, DTMF_CODE_MAX_LEN);

					if (((dtmfContactDetailsIndex >= CODEPLUG_DTMF_CONTACTS_MIN) && (dtmfContactDetailsIndex <= CODEPLUG_DTMF_CONTACTS_MAX))
						&& (tmpDTMFContact.name[0]!=0xFFU) && (tmpDTMFContact.code[0] != 0xFFU))
					{
						codeplugContactSaveDTMFDataForIndex(dtmfContactDetailsIndex, &tmpDTMFContact);
						menuDTMFContactDetailsTimeout = 2000;
						menuDTMFContactDetailsState = MENU_DTMF_CONTACT_DETAILS_SAVED;
						voicePromptsInit();
						voicePromptsAppendLanguageString(&currentLanguage->contact_saved);
						voicePromptsPlay();
					}

					updateScreen(false, allowedToSpeakUpdate);
				}
				else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
				{
					menuSystemPopPreviousMenu();
					return;
				}
				else
				{
					if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_DTMF_CODE)
					{
						// Add a digit
						if (sLen < DTMF_CODE_MAX_LEN)
						{
							int keyval = menuGetKeypadKeyValue(ev, false);

							char c[2] = {0, 0};

							if (keyval != 99)
							{
								if (keyval <=9)
									c[0] = keyval + '0';
								else if (keyval==14)
									c[0]='*';
								else if (keyval==15)
									c[0]='#';
								else
									break;
								dtmfCodeChars[codePos]=*c;
								if (codePos < strlen(dtmfCodeChars) && codePos < DTMF_CODE_MAX_LEN-1)
								{
									codePos++;
								}

								updateCursor(true);
								announceChar(*c);
							}
							updateScreen(false, false);
						}
					}
					else if (menuDataGlobal.currentItemIndex == CONTACT_DETAILS_NAME)
					{
						if ((ev->keys.event == KEY_MOD_PREVIEW) && (namePos < DTMF_NAME_MAX_LEN))
						{
							contactName[namePos] = ev->keys.key;
							updateCursor(true);
							announceChar(ev->keys.key);
							updateScreen(false, false);
						}
						else if ((ev->keys.event == KEY_MOD_PRESS) && (namePos < DTMF_NAME_MAX_LEN))
						{
							contactName[namePos] = ev->keys.key;
							if (namePos < strlen(contactName) && namePos < DTMF_NAME_MAX_LEN-1)
							{
								namePos++;
							}
							updateCursor(true);
							announceChar(ev->keys.key);
							updateScreen(false, false);
						}
					}
				}
			}
			break;

		case MENU_DTMF_CONTACT_DETAILS_SAVED:
			menuDTMFContactDetailsTimeout--;
			if ((menuDTMFContactDetailsTimeout == 0) || KEYCHECK_SHORTUP(ev->keys, KEY_GREEN) || KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				menuSystemPopPreviousMenu();
				return;
			}
			updateScreen(false, allowedToSpeakUpdate);
			break;

		case MENU_DTMF_CONTACT_DETAILS_EXISTS:
			menuDTMFContactDetailsTimeout--;
			if ((menuDTMFContactDetailsTimeout == 0) || KEYCHECK_SHORTUP(ev->keys, KEY_GREEN) || KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				menuDTMFContactDetailsState = MENU_DTMF_CONTACT_DETAILS_DISPLAY;
			}
			updateScreen(false, allowedToSpeakUpdate);
			break;
	}
}
