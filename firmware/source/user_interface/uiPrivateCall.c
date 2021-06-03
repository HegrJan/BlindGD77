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
#include "user_interface/menuSystem.h"
#include "user_interface/uiLocalisation.h"
#include "user_interface/uiUtilities.h"
#include "functions/settings.h"
#include "functions/ticks.h"

static bool privateCallCallback(void);
static bool handled = false;

menuStatus_t menuPrivateCall(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		static const int bufferLen = 17;
		char buffer[bufferLen];
		dmrIdDataStruct_t currentRec;

		// privateCallCallback() has been called, so this screen is
		// reloading, but we need to leave it ASAP
		if (handled)
		{
			handled = false;
			keyboardReset();
			menuSystemPopPreviousMenu();
			return MENU_STATUS_SUCCESS;
		}

		soundSetMelody(MELODY_PRIVATE_CALL);
		uiDataGlobal.PrivateCall.state = PRIVATE_CALL_ACCEPT;
		uiDataGlobal.receivedPcId = LinkHead->id;
		uiDataGlobal.receivedPcTS = (dmrMonitorCapturedTS != -1) ? dmrMonitorCapturedTS : trxGetDMRTimeSlot();

		if (!contactIDLookup(uiDataGlobal.receivedPcId, CONTACT_CALLTYPE_PC, buffer))
		{
			dmrIDLookup(uiDataGlobal.receivedPcId, &currentRec);
			strncpy(buffer, currentRec.text, 16);
			buffer[16] = 0;
		}

		snprintf(uiDataGlobal.MessageBox.message, MESSAGEBOX_MESSAGE_LEN_MAX, "%s\n%s\n%s",
		currentLanguage->private_call, currentLanguage->accept_call, buffer);
		uiDataGlobal.MessageBox.type = MESSAGEBOX_TYPE_INFO;
		uiDataGlobal.MessageBox.buttons = MESSAGEBOX_BUTTONS_YESNO;
		uiDataGlobal.MessageBox.decoration = MESSAGEBOX_DECORATION_NONE;
		uiDataGlobal.MessageBox.validatorCallback = privateCallCallback;

		menuSystemPushNewMenu(UI_MESSAGE_BOX);
	}

	return MENU_STATUS_SUCCESS;
}

void menuPrivateCallClear(void)
{
	uiDataGlobal.PrivateCall.state = PRIVATE_CALL_NOT_IN_CALL;
	uiDataGlobal.PrivateCall.lastID = 0;
	uiDataGlobal.tgBeforePcMode = 0;
	uiDataGlobal.receivedPcId = 0x00;
}

void menuPrivateCallDismiss(void)
{
	handled = true;
	uiDataGlobal.MessageBox.validatorCallback = NULL;
	menuSystemPopPreviousMenu();
}

static bool privateCallCallback(void)
{
	if (uiDataGlobal.MessageBox.keyPressed == KEY_RED)
	{
		uiDataGlobal.PrivateCall.state = PRIVATE_CALL_DECLINED;
		uiDataGlobal.PrivateCall.lastID = 0;
	}
	else if (uiDataGlobal.MessageBox.keyPressed == KEY_GREEN)
	{
		acceptPrivateCall(uiDataGlobal.receivedPcId, uiDataGlobal.receivedPcTS);
	}

	handled = true;

	return true;
}
