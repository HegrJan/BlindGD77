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
#include "user_interface/uiLocalisation.h"
#include "user_interface/editHandler.h"
#include "functions/ticks.h"

EditStructParrams_t editParams=
{
	.editFieldType=EDIT_TYPE_ALPHANUMERIC,
	.editBuffer=NULL,
	.maxLen=SCREEN_LINE_BUFFER_SIZE,
	.cursorPos=NULL,
	.xPixelOffset=0,
	.yPixelOffset=0,
	.allowedToSpeakUpdate=true
};

static bool needToRequeueEditBufferForAnnouncementOnSK1=false;

static void AnnounceCharWithRequeue(char ch)
{
	announceChar(ch);
	needToRequeueEditBufferForAnnouncementOnSK1=true;
} 

/*
After announcing a character during editing, we need to requeue the entire buffer so that sk1 will reread the entire buffer.
*/
void RequeueEditBufferForAnnouncementOnSK1IfNeeded()
{
	if (!needToRequeueEditBufferForAnnouncementOnSK1)
		return;
	if (voicePromptsIsPlaying())
		return;
	
	needToRequeueEditBufferForAnnouncementOnSK1=false;
	
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
		return;

	if (!editParams.editBuffer[0])
		return;
	
	voicePromptsInit();
	voicePromptsAppendString(editParams.editBuffer);
	if (keypadAlphaEnable)
	{
		voicePromptsAppendLanguageString(&currentLanguage->alphanumeric);
	}
	else
	{
		voicePromptsAppendLanguageString(&currentLanguage->numeric);
	}
}

static bool InsertChar(char* buffer, char ch, int* cursorPos, int max)
{
	if (!buffer || !cursorPos) return false;
	
	int len=strlen(buffer);
	if (len >=max-1)
		return false;
	
	if (*cursorPos >= len)
	{
		buffer[*cursorPos]=ch;
		if (*cursorPos < max-1)
		{
			(*cursorPos)++;
			buffer[*cursorPos]='\0';
		}
		return true;
	}
	// move chars along to make room.
	for (int i=len+1; i > *cursorPos; --i)
	{
		buffer[i]=buffer[i-1];
	}
	buffer[(*cursorPos)++]=ch;
	return true;
}

 void editUpdateCursor(EditStructParrams_t* editParams, bool moved, bool render)
{
	if (!editParams)
		return;
	
#if defined(PLATFORM_RD5R)
	const int MENU_CURSOR_Y = 32;
#else
	const int MENU_CURSOR_Y = 46;
#endif

int xPixelOffset = editParams->xPixelOffset;
int yPixelOffset=editParams->yPixelOffset ? editParams->yPixelOffset : MENU_CURSOR_Y;
int pos=*editParams->cursorPos;

	static uint32_t lastBlink = 0;
	static bool     blink = false;
	uint32_t        m = fw_millis();

	if (moved)
	{
		blink = true;
	}

	if (moved || (m - lastBlink) > 500)
	{
		ucDrawFastHLine(xPixelOffset+(pos * 8), yPixelOffset, 8, blink);

		blink = !blink;
		lastBlink = m;

		if (render)
		{
			ucRenderRows(yPixelOffset / 8, yPixelOffset / 8 + 1);
		}
	}
}

void moveCursorLeftInString(char *str, int *pos, bool delete)
{
	int nLen = strlen(str);

	if (*pos > 0)
	{
		*pos -=1;
		AnnounceCharWithRequeue(str[*pos]); // speak the new char or the char about to be backspaced out.

		if (delete)
		{
			for (int i = *pos; i <= nLen; i++)
			{
				str[i] = str[i + 1];
			}
		}
	}
}

void moveCursorRightInString(char *str, int *pos, int max)
{
	int safeMax=SAFE_MIN(strlen(str),(max-1));
	if (*pos < safeMax)
	{
		(*pos) ++;
		AnnounceCharWithRequeue(str[*pos]); // speak the new char
	}
}

static void ToggleKeypadAlphaEnable()
{
	voicePromptsInit();
	keypadAlphaEnable=!keypadAlphaEnable;
	if (keypadAlphaEnable)
	{
		editParams.editFieldType = EDIT_TYPE_ALPHANUMERIC;
		voicePromptsAppendLanguageString(&currentLanguage->alphanumeric);
	}
	else
	{
		editParams.editFieldType = EDIT_TYPE_DTMF_CHARS;
		voicePromptsAppendLanguageString(&currentLanguage->numeric);
	}
	voicePromptsPlay();
}
bool HandleEditEvent(uiEvent_t *ev, EditStructParrams_t* editParams)
{
	if (!editParams || !editParams->editBuffer || !editParams->cursorPos)
		return false;
	if (HandleCustomPrompts(ev, editParams->editBuffer))
	{
		editParams->allowedToSpeakUpdate=false;
		return true;
	}
	editParams->allowedToSpeakUpdate=true; // set to false when we speak a newly inserted character to avoid the whole buffer being repeated on a screen update.
	
	if ((ev->events & KEY_EVENT)==0)
	{
		RequeueEditBufferForAnnouncementOnSK1IfNeeded();
		return false;
	}
	if (KEYCHECK_PRESS(ev->keys, KEY_DOWN) || KEYCHECK_PRESS(ev->keys, KEY_UP))
		return false; // switching fields.
	if (KEYCHECK_PRESS(ev->keys, KEY_GREEN) || KEYCHECK_PRESS(ev->keys, KEY_RED))
		return false; // exiting screen.
	int editBufferLen=strlen(editParams->editBuffer);
	
	bool keyPreview=(ev->keys.event == KEY_MOD_PREVIEW);
	bool keyPressed=(ev->keys.event == KEY_MOD_PRESS);
	int keyval = menuGetKeypadKeyValue(ev, editParams->editFieldType==EDIT_TYPE_NUMERIC);
	if (BUTTONCHECK_DOWN(ev, BUTTON_SK1) && (keyPressed || keyPreview || keyval!=99))
		return false;
	if (KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_RIGHT))
	{
		bool deleteToEnd =BUTTONCHECK_DOWN(ev, BUTTON_SK2);
		if (deleteToEnd)
		{
			while (editBufferLen > (*editParams->cursorPos))
			{
				editParams->editBuffer[*editParams->cursorPos]=editParams->editBuffer[(*editParams->cursorPos)+1];
				editBufferLen--;
			}
			editParams->editBuffer[editBufferLen]='\0';
		}

		*editParams->cursorPos = editBufferLen;
		editUpdateCursor(editParams, true, true);
		editParams->allowedToSpeakUpdate = false;
		return true;
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT))
	{
		if (BUTTONCHECK_DOWN(ev, BUTTON_SK2) && (editParams->editFieldType != EDIT_TYPE_NUMERIC))
		{
			ToggleKeypadAlphaEnable();
			editParams->allowedToSpeakUpdate=false; // set to false when we speak a newly inserted character to avoid the whole buffer being repeated on a screen update.
			return true;
		}
		moveCursorRightInString(editParams->editBuffer, editParams->cursorPos, editParams->maxLen);
		editUpdateCursor(editParams, true, true);
		editParams->allowedToSpeakUpdate = false;
		return true;
	}
	else if (KEYCHECK_LONGDOWN(ev->keys, KEY_LEFT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_LEFT))
	{
		bool deleteToStart =BUTTONCHECK_DOWN(ev, BUTTON_SK2);
		if (deleteToStart)
		{
			while (*editParams->cursorPos > 0)
			{
				moveCursorLeftInString(editParams->editBuffer, editParams->cursorPos, true);
			}
		}
		*editParams->cursorPos = 0;
		editUpdateCursor(editParams, true, true);
		if (editBufferLen > 0)
			AnnounceCharWithRequeue(editParams->editBuffer[0]);
		editParams->allowedToSpeakUpdate = false;
		return true;
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_LEFT))
	{
		moveCursorLeftInString(editParams->editBuffer, editParams->cursorPos, BUTTONCHECK_DOWN(ev, BUTTON_SK2));
		editUpdateCursor(editParams, true, true);
		editParams->allowedToSpeakUpdate = false;
		return true;
	}
	// Add a digit or character 
	int curLen=strlen(editParams->editBuffer);
	
	char ch=0;
	if (editParams->editFieldType!=EDIT_TYPE_ALPHANUMERIC)
	{
		if (keyval == 99)
			return false;
		keyPressed=true;// pretend because when enableAlpha is not set, we do not get the same events.
		if (keyval <=9)
			ch = keyval + '0';
		else if (editParams->editFieldType==EDIT_TYPE_DTMF_CHARS)
		{
			if (keyval==14)
				ch='*';
			else if (keyval==15)
				ch='#';
		}
	}
	else if (keyPreview || keyPressed)
	{
		ch = ev->keys.key;
	}
	if (!ch)
		return false;
	if (keyPressed)
	{
		if (curLen < editParams->maxLen-1)
			InsertChar(editParams->editBuffer, ch, editParams->cursorPos, editParams->maxLen);
		else
		{
			editParams->editBuffer[*editParams->cursorPos]=ch; // just add at the current cursor location, overwriting what was there.
			editParams->editBuffer[editParams->maxLen-1]='\0';// null terminate
			// If we're not on the last char then advance the cursor.
			if (*editParams->cursorPos < editParams->maxLen-1)
				(*editParams->cursorPos)++;
		}
		editUpdateCursor(editParams, true, true);
	}
	AnnounceCharWithRequeue(ch);
	editParams->allowedToSpeakUpdate = false;
	return true;
}
char* GetCurrentEditBuffer()
{
	return editParams.editBuffer;
}
