/*
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
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
#include "functions/settings.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiLocalisation.h"
#include "user_interface/uiUtilities.h"

#define CHAR_CONSTANTS_ONLY // Needed to get FONT_CHAR_* glyph offsets
#if defined(LANGUAGE_BUILD_JAPANESE)
#include "hardware/UC1701_charset_JA.h"
#else
#include "hardware/UC1701_charset.h"
#endif

static void updateScreen(bool isFirstRun);
static void handleEvent(uiEvent_t *ev);
static menuStatus_t menuLanguageExitCode = MENU_STATUS_SUCCESS;


static void clearNonLatinChar(uint8_t *str)
{
#if ! defined(LANGUAGE_BUILD_JAPANESE)
	uint8_t *p = str;

	while (*p != '\0')
	{
		switch (*p)
		{
			case FONT_CHAR_CAPITAL_A_WITH_OGONEK: // Ą
			case FONT_CHAR_SMALL_A_WITH_OGONEK:   // ą
			case 192: // ?�
			case 193: // ?�
			case 194: // ?�
			case 195: // ??
			case 196: // ?�
			case 197: // ?�
			case 224: // ?�
			case 225: // ??
			case 226: // ??
			case 227: // ??
			case 228: // ?�
			case 229: // ??
				*p = 'a';
				break;

			case FONT_CHAR_CAPITAL_C_WITH_CARON: // �?
			case FONT_CHAR_SMALL_C_WITH_CARON:   // �?
			case FONT_CHAR_CAPITAL_C_WITH_ACUTE: // Ć
			case FONT_CHAR_SMALL_C_WITH_ACUTE:   // ć
			case 199: // ?�
			case 231: // ?�
				*p = 'c';
				break;

			case FONT_CHAR_CAPITAL_E_WITH_OGONEK: // �?
			case FONT_CHAR_SMALL_E_WITH_OGONEK:   // ę
			case FONT_CHAR_CAPITAL_E_WITH_CARON:  // Ě
			case FONT_CHAR_SMALL_E_WITH_CARON:    // ě
			case 200: // ??
			case 201: // ?�
			case 202: // ?�
			case 203: // ?�
			case 232: // ?�
			case 233: // ?�
			case 234: // ??
			case 235: // ?�
				*p = 'e';
				break;

			case FONT_CHAR_G_CAPITAL_WITH_BREVE: // Ğ
			case FONT_CHAR_G_SMALL_WITH_BREVE:   // �?
				*p = 'g';
				break;

			case FONT_CHAR_SMALL_I_DOTLESS:    // ı
			case FONT_CHAR_CAPITAL_I_WITH_DOT: // İ
			case 204: // ??
			case 205: // ??
			case 206: // ?�
			case 207: // ??
			case 236: // ?�
			case 237: // ?�
			case 238: // ?�
			case 239: // ??
				*p ='i';
				break;

			case FONT_CHAR_CAPITAL_L_WITH_STROKE: // ?�
			case FONT_CHAR_SMALL_L_WITH_STROKE:   // ?�
				*p = 'l';
				break;

			case FONT_CHAR_CAPITAL_N_WITH_ACUTE: // ??
			case FONT_CHAR_SMALL_N_WITH_ACUTE:   // ?�
			case 209: // ?�
			case 241: // ?�
				*p = 'n';
				break;

			case 210: // ?�
			case 211: // ?�
			case 212: // ?�
			case 213: // ?�
			case 214: // ?�
			case 216: // ??
			case 242: // ??
			case 243: // ??
			case 244: // ?�
			case 245: // ?�
			case 246: // ?�
			case 248: // ?�
			case 240: // ?�
				*p = 'o';
				break;

			case FONT_CHAR_CAPITAL_R_WITH_CARON: // ??
			case FONT_CHAR_SMALL_R_WITH_CARON:   // ?�
			case 208: // ?�
				*p = 'r';
				break;

			case FONT_CHAR_CAPITAL_S_WITH_ACUTE:   // ?�
			case FONT_CHAR_SMALL_S_WITH_ACUTE:     // ?�
			case FONT_CHAR_CAPITAL_S_WITH_CARON:   // ?�
			case FONT_CHAR_SMALL_S_WITH_CARON:     // ??
			case FONT_CHAR_S_CAPITAL_WITH_CEDILLA: // ?�
			case FONT_CHAR_S_SMALL_WITH_CEDILLA:   // ??
			case 223: // ??
				*p = 's';
				break;

			case FONT_CHAR_CAPITAL_U_WITH_RING_ABOVE: // ?�
			case FONT_CHAR_SMALL_U_WITH_RING_ABOVE:   // ??
			case 217: // ?�
			case 218: // ?�
			case 219: // ?�
			case 220: // ??
			case 249: // ??
			case 250: // ??
			case 251: // ?�
			case 252: // ??
				*p = 'u';
				break;

			case FONT_CHAR_CAPITAL_Y_WITH_DIAERESIS: // ?�
			case 221: // ??
			case 253: // ??
			case 255: // ??
				*p = 'y';
				break;

			case FONT_CHAR_SMALL_Z_WITH_CARON:       // ??
			case FONT_CHAR_CAPITAL_Z_WITH_CARON:     // ??
			case FONT_CHAR_CAPITAL_Z_WITH_ACUTE:     // ??
			case FONT_CHAR_SMALL_Z_WITH_ACUTE:       // ??
			case FONT_CHAR_CAPITAL_Z_WITH_DOT_ABOVE: // ?�
			case FONT_CHAR_SMALL_Z_WITH_DOT_ABOVE:   // ??
				*p = 'z';
				break;
		}
		p++;
	}
#endif
}

menuStatus_t menuLanguage(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		menuDataGlobal.endIndex = NUM_LANGUAGES;

		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->language);
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
		menuLanguageExitCode = MENU_STATUS_SUCCESS;

		if (ev->hasEvent)
		{
			handleEvent(ev);
		}
	}
	return menuLanguageExitCode;
}

static void updateScreen(bool isFirstRun)
{
	int mNum = 0;

	ucClearBuf();
	menuDisplayTitle(currentLanguage->language);

	for(int i = -1; i <= 1; i++)
	{
		mNum = menuGetMenuOffset(NUM_LANGUAGES, i);

		if (mNum < NUM_LANGUAGES)
		{
			menuDisplayEntry(i, mNum, (char *)languages[LANGUAGE_DISPLAY_ORDER[mNum]].LANGUAGE_NAME);

			if (i == 0)
			{
				if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
				{
					char buffer[17];

					snprintf(buffer, 17, "%s", (char *)languages[LANGUAGE_DISPLAY_ORDER[mNum]].LANGUAGE_NAME);

					clearNonLatinChar((uint8_t *)&buffer[0]);

					if (isFirstRun == false)
					{
						voicePromptsInit();
					}

					voicePromptsAppendString(buffer);
					promptsPlayNotAfterTx();
				}
			}
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
		if ((QUICKKEY_TYPE(ev->function) == QUICKKEY_MENU) && (QUICKKEY_ENTRYID(ev->function) < NUM_LANGUAGES))
		{
			menuDataGlobal.currentItemIndex = QUICKKEY_ENTRYID(ev->function);
			settingsSet(nonVolatileSettings.languageIndex, menuDataGlobal.currentItemIndex);
			currentLanguage = &languages[menuDataGlobal.currentItemIndex];
			settingsSaveIfNeeded(true);
			menuSystemLanguageHasChanged();
			menuSystemPopAllAndDisplayRootMenu();
		}
		return;
	}

	if (KEYCHECK_PRESS(ev->keys, KEY_DOWN) && (menuDataGlobal.endIndex != 0))
	{
		menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, NUM_LANGUAGES);
		updateScreen(false);
		menuLanguageExitCode |= MENU_STATUS_LIST_TYPE;
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
	{
		menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, NUM_LANGUAGES);
		updateScreen(false);
		menuLanguageExitCode |= MENU_STATUS_LIST_TYPE;
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
	{
		settingsSet(nonVolatileSettings.languageIndex, (uint8_t) LANGUAGE_DISPLAY_ORDER[menuDataGlobal.currentItemIndex]);
		currentLanguage = &languages[LANGUAGE_DISPLAY_ORDER[menuDataGlobal.currentItemIndex]];
		settingsSaveIfNeeded(true);
		menuSystemLanguageHasChanged();
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
		saveQuickkeyMenuIndex(ev->keys.key, menuSystemGetCurrentMenuNumber(), menuDataGlobal.currentItemIndex, 0);
		return;
	}
}
