/*
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *                         Daniel Caujolle-Bert, F1RMB
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
#include "functions/codeplug.h"
#include "main.h"
#include "functions/settings.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"

typedef enum
{
	MENU_CONTACT_LIST_DISPLAY = 0,
	MENU_CONTACT_LIST_CONFIRM,
	MENU_CONTACT_LIST_DELETED,
	MENU_CONTACT_LIST_TG_IN_RXGROUP
} contactListState_t;

typedef enum
{
	MENU_CONTACT_LIST_CONTACT_DIGITAL = 0,
	MENU_CONTACT_LIST_CONTACT_DTMF
} contactListContactType_t;

static void updateScreen(bool isFirstRun);
static void handleEvent(uiEvent_t *ev);

static struct_codeplugContact_t     contact;
static struct_codeplugDTMFContact_t dtmfContact;

static int contactListType;
static contactListState_t contactListDisplayState;
static contactListState_t contactListOverrideState = MENU_CONTACT_LIST_DISPLAY;
static int menuContactListTimeout; // Action result screen autohide timeout (or it will instantly disappear if RED or GREEN is pressed)
static menuStatus_t menuContactListExitCode = MENU_STATUS_SUCCESS;
static menuStatus_t menuContactListSubMenuExitCode = MENU_STATUS_SUCCESS;
static int contactListEntryCount;
static int submenuEntryCount;
static bool sortAfterCreateOrEditContact=false;

static const char * const *calltypeVoices[4] = { NULL, NULL, NULL, NULL };

static void UpdateChannelWithLastReferencedContact()
{
								if (currentChannelData->NOT_IN_CODEPLUG_flag != 0) return; // its not the channel screen.
	if (uiDataGlobal.currentSelectedContactIndex == 0) return; // nothing to add.
							
	uint16_t channelIndex=CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : currentZone.channels[nonVolatileSettings.currentChannelIndexInZone];
	AddLastReferencedContactToChannel(channelIndex, uiDataGlobal.currentSelectedContactIndex);
}

// Apply contact + its TS on selection for TX (contact list of quick list).
static void overrideWithSelectedContact(void)
{
	menuPrivateCallClear();
	setOverrideTGorPC(contactListContactData.tgNumber, (contactListContactData.callType == CONTACT_CALLTYPE_PC));
	// Contact has a TS override set
	if ((contactListContactData.reserve1 & 0x01) == 0x00)
	{
		int ts = ((contactListContactData.reserve1 & 0x02) >> 1);
		trxSetDMRTimeSlot(ts);
		tsSetManualOverride(((menuSystemGetRootMenuNumber() == UI_CHANNEL_MODE) ? CHANNEL_CHANNEL : (CHANNEL_VFO_A + nonVolatileSettings.currentVFONumber)), (ts + 1));
	}
}

static void reloadContactList(contactListContactType_t type)
{
	if (sortAfterCreateOrEditContact)
	{// coming back from creating a new contact.
		sortAfterCreateOrEditContact=false;
		nonVolatileSettings.sortFlags|=sortContactsByName;
		codeplugInitContactsCache(); // force it to be reloaded unsorted!
	}

	menuDataGlobal.endIndex = (type == MENU_CONTACT_LIST_CONTACT_DIGITAL) ? codeplugContactsGetCount(nonVolatileSettings.dmrListFilterType) : codeplugDTMFContactsGetCount();
	if (menuDataGlobal.endIndex > 0)
	{
		if (menuDataGlobal.currentItemIndex >= menuDataGlobal.endIndex)
		{
			menuDataGlobal.currentItemIndex = 0;
		}
		uiDataGlobal.currentSelectedContactIndex = (type == MENU_CONTACT_LIST_CONTACT_DIGITAL)
				? codeplugContactGetDataForNumberInType(menuDataGlobal.currentItemIndex + 1, nonVolatileSettings.dmrListFilterType, &contactListContactData)
				: codeplugDTMFContactGetDataForNumber(menuDataGlobal.currentItemIndex + 1, &contactListDTMFContactData);
	}
	else
	{
		uiDataGlobal.currentSelectedContactIndex = 0;
	}
}

menuStatus_t menuContactList(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		calltypeVoices[0] = &currentLanguage->group_call;
		calltypeVoices[1] = &currentLanguage->private_call;
		calltypeVoices[2] = &currentLanguage->all_call;
		calltypeVoices[3] = &currentLanguage->all;

		menuContactListTimeout = 0;

		if (contactListOverrideState == MENU_CONTACT_LIST_DISPLAY)
		{
			if (uiDataGlobal.currentSelectedContactIndex == 0)
			{
				int currentMenu = menuSystemGetCurrentMenuNumber();

				// Shows digital contact list if called from "contact list" menu entry, or from <SK2>+# in digital.
				// Otherwise displays DTMF contact list
				bool contactListTypeLocal = ((currentMenu == MENU_CONTACT_LIST) || ((currentMenu == MENU_CONTACT_QUICKLIST) && (trxGetMode() != RADIO_MODE_ANALOG))) ? MENU_CONTACT_LIST_CONTACT_DIGITAL : MENU_CONTACT_LIST_CONTACT_DTMF;
				if (contactListType!=contactListTypeLocal)
				contactListType=contactListTypeLocal;

				dtmfSequenceReset();
			}
			else
			{
				if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
				{
					codeplugContactGetDataForIndex(uiDataGlobal.currentSelectedContactIndex, &contactListContactData);
				}
				else
				{
					codeplugDTMFContactGetDataForIndex(uiDataGlobal.currentSelectedContactIndex, &contactListDTMFContactData);
				}
			}

			reloadContactList(contactListType);
			contactListDisplayState = MENU_CONTACT_LIST_DISPLAY;

			voicePromptsInit();
			if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
			{
				voicePromptsAppendLanguageString(&currentLanguage->dmr_contacts);
				voicePromptsAppendLanguageString(calltypeVoices[nonVolatileSettings.dmrListFilterType]);
			}
			else
			{
				voicePromptsAppendLanguageString(&currentLanguage->dtmf_contact_list);
			}
			if (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_2)
			{
				voicePromptsAppendLanguageString(&currentLanguage->menu);
			}
			voicePromptsAppendPrompt(PROMPT_SILENCE);
		}
		else
		{
			if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
			{
				codeplugContactGetDataForIndex(uiDataGlobal.currentSelectedContactIndex, &contactListContactData);
			}
			else
			{
				codeplugDTMFContactGetDataForIndex(uiDataGlobal.currentSelectedContactIndex, &contactListDTMFContactData);
			}
			contactListDisplayState = contactListOverrideState;
			contactListOverrideState = MENU_CONTACT_LIST_DISPLAY;
		}

		updateScreen(true);
		menuContactListExitCode = (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuContactListExitCode = MENU_STATUS_SUCCESS;

		if (ev->hasEvent || (menuContactListTimeout > 0))
		{
			handleEvent(ev);
		}
	}

	// Play the DTMF contact sequence from Quicklist
	dtmfSequenceTick(false);

	return menuContactListExitCode;
}

static void PlayDMRVoiceTag(bool init)
{
	if (contactListType != MENU_CONTACT_LIST_CONTACT_DIGITAL) return;
	if (contactListContactData.ringStyle==0) return;
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_2) return;
	if (init)
		voicePromptsInit();
	voicePromptsAppendPrompt(VOICE_PROMPT_CUSTOM+contactListContactData.ringStyle);
	if (init)
		voicePromptsPlay();
}

static void updateScreen(bool isFirstRun)
{
	contactListEntryCount=menuDataGlobal.endIndex;// store this off as it will be used in the submenu because the above changes to reflect that menu.

	char nameBuf[33];
	int mNum;
	int idx;
	const char *calltypeName[] = { currentLanguage->group_call, currentLanguage->private_call, currentLanguage->all_call, "DTMF" };

	ucClearBuf();

	switch (contactListDisplayState)
	{
		case MENU_CONTACT_LIST_DISPLAY:
			menuDisplayTitle((char *) calltypeName[((contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL) ? nonVolatileSettings.dmrListFilterType : 4)]);

			if (menuDataGlobal.endIndex == 0)
			{
				ucPrintCentered((DISPLAY_SIZE_Y / 2), currentLanguage->empty_list, FONT_SIZE_3);

				voicePromptsAppendLanguageString(&currentLanguage->empty_list);
				if (!uiDataGlobal.VoicePrompts.inhibitInitial)
					promptsPlayNotAfterTx();
			}
			else
			{
				for (int i = -1; i <= 1; i++)
				{
					mNum = menuGetMenuOffset(menuDataGlobal.endIndex, i);
					idx = (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
							? codeplugContactGetDataForNumberInType(mNum + 1, nonVolatileSettings.dmrListFilterType, &contact)
							: codeplugDTMFContactGetDataForNumber(mNum + 1, &dtmfContact);

					if (idx > 0)
					{
						codeplugUtilConvertBufToString(((contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL) ? contact.name : dtmfContact.name), nameBuf, 16); // need to convert to zero terminated string
					}
					else
						strcpy(nameBuf, " ");
					menuDisplayEntry(i, mNum, (char*) nameBuf);

					if (i == 0)
					{
						if (strlen(nameBuf))
						{
							voicePromptsAppendString(nameBuf);
						}
						else
						{
							voicePromptsAppendLanguageString(&currentLanguage->name);
							voicePromptsAppendPrompt(PROMPT_SILENCE);
							voicePromptsAppendLanguageString(&currentLanguage->none);
						}
						PlayDMRVoiceTag(false);
					}
				}

			}
			if (!uiDataGlobal.VoicePrompts.inhibitInitial)
				promptsPlayNotAfterTx();
			break;
		case MENU_CONTACT_LIST_CONFIRM:
			codeplugUtilConvertBufToString(contactListContactData.name, nameBuf, 16);
			menuDisplayTitle(nameBuf);
			ucPrintCentered(16, currentLanguage->delete_contact_qm, FONT_SIZE_3);
			ucDrawChoice(CHOICE_YESNO, false);
			break;
		case MENU_CONTACT_LIST_DELETED:
			codeplugUtilConvertBufToString(contactListContactData.name, nameBuf, 16);
			ucPrintCentered(16, currentLanguage->contact_deleted, FONT_SIZE_3);
			ucDrawChoice(CHOICE_DISMISS, false);
			break;
		case MENU_CONTACT_LIST_TG_IN_RXGROUP:
			codeplugUtilConvertBufToString(contactListContactData.name, nameBuf, 16);
			menuDisplayTitle(nameBuf);
			ucPrintCentered(16, currentLanguage->contact_used, FONT_SIZE_3);
			ucPrintCentered((DISPLAY_SIZE_Y/2), currentLanguage->in_rx_group, FONT_SIZE_3);
			ucDrawChoice(CHOICE_DISMISS, false);
			break;
	}
	if (uiDataGlobal.VoicePrompts.inhibitInitial)
		uiDataGlobal.VoicePrompts.inhibitInitial=false;

	ucRender();
}

static void handleEvent(uiEvent_t *ev)
{
	if (ev->events & FUNCTION_EVENT)
	{
		if (ev->function == FUNC_REDRAW)
		{
			updateScreen(false);
			return;
		}
	}

	if (ev->events & BUTTON_EVENT)
	{
		if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK1) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0))
		{
			bool announceChannelName=currentChannelData->name[0]!=0;
			AnnounceChannelSummary((nonVolatileSettings.audioPromptMode <= AUDIO_PROMPT_MODE_VOICE_LEVEL_2), announceChannelName);
			return;
		}
		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}
	}

	// DTMF sequence is playing, stop it.
	if (dtmfSequenceIsKeying() && ((ev->keys.key != 0) || BUTTONCHECK_DOWN(ev, BUTTON_PTT)
#if ! defined(PLATFORM_RD5R)
													|| BUTTONCHECK_DOWN(ev, BUTTON_ORANGE)
#endif
	))
	{
		dtmfSequenceStop();
		keyboardReset();
		return;
	}

	switch (contactListDisplayState)
	{
		case MENU_CONTACT_LIST_DISPLAY:
			if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
			{
				menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, menuDataGlobal.endIndex);
				uiDataGlobal.currentSelectedContactIndex = (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
						? codeplugContactGetDataForNumberInType(menuDataGlobal.currentItemIndex + 1, nonVolatileSettings.dmrListFilterType, &contactListContactData)
						: codeplugDTMFContactGetDataForNumber(menuDataGlobal.currentItemIndex + 1, &contactListDTMFContactData);
				voicePromptsInit();
				updateScreen(false);
				menuContactListExitCode |= MENU_STATUS_LIST_TYPE;
			}
			else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
			{
				menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, menuDataGlobal.endIndex);
				uiDataGlobal.currentSelectedContactIndex = (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
						? codeplugContactGetDataForNumberInType(menuDataGlobal.currentItemIndex + 1, nonVolatileSettings.dmrListFilterType, &contactListContactData)
						: codeplugDTMFContactGetDataForNumber(menuDataGlobal.currentItemIndex + 1, &contactListDTMFContactData);
				voicePromptsInit();
				updateScreen(false);
				menuContactListExitCode |= MENU_STATUS_LIST_TYPE;
			}
			else if (KEYCHECK_LONGDOWN(ev->keys, KEY_STAR))
			{// set the sort contacts flag.
				voicePromptsInit();
				voicePromptsAppendLanguageString(&currentLanguage->sortBy);

				if ((nonVolatileSettings.sortFlags & sortContactsByName)==0)
				{
					nonVolatileSettings.sortFlags |= sortContactsByName;
					voicePromptsAppendLanguageString(&currentLanguage->name);
				}
				else
				{
					nonVolatileSettings.sortFlags &= ~sortContactsByName;
					voicePromptsAppendLanguageString(&currentLanguage->none);
				}
				codeplugInitContactsCache();
				codeplugRxGroupInitCache();

				lastLoadedRxGroup=-1; // force reload as it has been changed.
				uiDataGlobal.VoicePrompts.inhibitInitial=true;
				reloadContactList(contactListType);
				contactListDisplayState = MENU_CONTACT_LIST_DISPLAY;
				updateScreen(false);
				voicePromptsPlay();

				return; 
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_HASH))
			{
				if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
				{
					nonVolatileSettings.dmrListFilterType = (nonVolatileSettings.dmrListFilterType + 1) % (CONTACT_CALLTYPE_ANY + 1);
					settingsSetDirty();
					reloadContactList(contactListType);

					voicePromptsInit();
					voicePromptsAppendLanguageString(calltypeVoices[nonVolatileSettings.dmrListFilterType]);
					voicePromptsAppendPrompt(PROMPT_SILENCE);

					updateScreen(false);
				}
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
			{
				int currentMenu = menuSystemGetCurrentMenuNumber();
				int currentMode = trxGetMode();

				if (currentMenu == MENU_CONTACT_QUICKLIST)
				{
					if (contactListEntryCount==0)
					{
						if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
						{
							menuSystemSetCurrentMenu(MENU_CONTACT_DETAILS);
						}
						else
						{
							menuSystemSetCurrentMenu(MENU_CONTACT_NEW_DTMF);
						}
						return;
					}
					if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
					{
						if (menuDataGlobal.endIndex > 0)
						{
							if (currentMode == RADIO_MODE_DIGITAL)
							{
								overrideWithSelectedContact();
								UpdateChannelWithLastReferencedContact();
								uiDataGlobal.currentSelectedContactIndex = 0;
								announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC, PROMPT_THRESHOLD_3);
								menuSystemPopAllAndDisplayRootMenu();
							}
							else
							{
								menuContactListExitCode |= MENU_STATUS_ERROR;
							}
						}
						return;
					}
					else if (menuDataGlobal.endIndex > 0)// MENU_CONTACT_LIST_CONTACT_DTMF
					{
						if (currentMode == RADIO_MODE_ANALOG)
						{
							dtmfSequencePrepare(contactListDTMFContactData.code, true);
							UpdateChannelWithLastReferencedContact();
						}
						else
						{
							menuContactListExitCode |= MENU_STATUS_ERROR;
						}
						return;
					}
				}

				if ((currentMenu == MENU_CONTACT_LIST) ||
						((currentMenu == MENU_DTMF_CONTACT_LIST)))
				{
					menuSystemPushNewMenu(MENU_CONTACT_LIST_SUBMENU);
				}

				return;
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				uiDataGlobal.currentSelectedContactIndex = 0;
				menuSystemPopPreviousMenu();
				return;
			}

			if ((contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL) &&
					(KEYCHECK_SHORTUP_NUMBER(ev->keys) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2))))
			{
				saveQuickkeyContactIndex(ev->keys.key, (uint16_t)contactListContactData.NOT_IN_CODEPLUGDATA_indexNumber);
				return;
			}

			break;

		case MENU_CONTACT_LIST_CONFIRM:
			if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
			{
				if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
				{// delete voice tag for this contact.
					if (contact.ringStyle)
					{
						DeleteDMRVoiceTag(contact.ringStyle);
						contact.ringStyle=0;
					}
					memset(contact.name, 0xff, 16);
					contact.tgNumber = 0;
					contact.callType = 0xFF;
					codeplugContactSaveDataForIndex(uiDataGlobal.currentSelectedContactIndex, &contact);
				}
				else // DTMF
				{
					memset(&dtmfContact, 0xFFu, CODEPLUG_DTMF_CONTACT_DATA_STRUCT_SIZE);
					codeplugContactSaveDTMFDataForIndex(uiDataGlobal.currentSelectedContactIndex, &dtmfContact);
				}
				uiDataGlobal.currentSelectedContactIndex = 0;
				menuContactListTimeout = 500;
				contactListDisplayState = MENU_CONTACT_LIST_DELETED;
				reloadContactList(contactListType);
				updateScreen(false);
				voicePromptsInit();
				voicePromptsAppendLanguageString(&currentLanguage->contact_deleted);
				voicePromptsPlay();
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				contactListDisplayState = MENU_CONTACT_LIST_DISPLAY;
				reloadContactList(contactListType);
				updateScreen(false);
			}
			break;

		case MENU_CONTACT_LIST_DELETED:
		case MENU_CONTACT_LIST_TG_IN_RXGROUP:
			menuContactListTimeout--;
			if ((menuContactListTimeout == 0) || KEYCHECK_SHORTUP(ev->keys, KEY_GREEN) || KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				menuContactListTimeout = 0;
				contactListDisplayState = MENU_CONTACT_LIST_DISPLAY;
				reloadContactList(contactListType);
				voicePromptsInit();

				updateScreen(false);
			}
			break;
	}
}

enum CONTACT_LIST_QUICK_MENU_ITEMS
{
	CONTACT_LIST_QUICK_MENU_SELECT = 0,
	CONTACT_LIST_QUICK_MENU_NEW,
	CONTACT_LIST_QUICK_MENU_EDIT,
	CONTACT_LIST_QUICK_MENU_DELETE,
	CONTACT_LIST_QUICK_MENU_DTMF_ENTRY, // should be the last so we can choose not to show it.
	NUM_CONTACT_LIST_QUICK_MENU_ITEMS    // The last item in the list is used so that we automatically get a total number of items in the list
};

static void updateSubMenuScreen(bool isFirstRun)
{
	int mNum = 0;
	char buf[SCREEN_LINE_BUFFER_SIZE];
	char * const *langTextConst = NULL;// initialise to please the compiler
	if (!isFirstRun)
	{
		voicePromptsInit();
	}
	ucClearBuf();

	codeplugUtilConvertBufToString((contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL) ? contactListContactData.name : contactListDTMFContactData.name, buf, 16);
	menuDisplayTitle(buf);
	
	submenuEntryCount = contactListEntryCount > 0 ? NUM_CONTACT_LIST_QUICK_MENU_ITEMS : 2; // only render DTMF Entry and New Contact if list is empty.
	if ((contactListType ==MENU_CONTACT_LIST_CONTACT_DIGITAL || trxGetMode() == RADIO_MODE_DIGITAL) && (submenuEntryCount > 1))
		submenuEntryCount--; // Do not include DTMF_ENTRY.
	bool replaceSelectWithDTMFEntry=(submenuEntryCount==2);

	for(int i = -1; i <= 1; i++)
	{
		mNum = menuGetMenuOffset(submenuEntryCount, i);

		buf[0] = 0;
		
		switch(mNum)
		{
			case CONTACT_LIST_QUICK_MENU_SELECT:
				if (replaceSelectWithDTMFEntry)
					langTextConst = (char * const *)&currentLanguage->dtmf_entry;
				else
					langTextConst = (char * const *)&currentLanguage->select_tx;
				break;
			case CONTACT_LIST_QUICK_MENU_NEW:
				langTextConst = (char * const *)&currentLanguage->new_contact;
				break;
			case CONTACT_LIST_QUICK_MENU_EDIT:
				langTextConst = (char * const *)&currentLanguage->edit_contact;
				break;
			case CONTACT_LIST_QUICK_MENU_DELETE:
				langTextConst = (char * const *)&currentLanguage->delete_contact;
				break;
			case CONTACT_LIST_QUICK_MENU_DTMF_ENTRY:
				langTextConst = (char * const *)&currentLanguage->dtmf_entry;
				break;
		}

		if (langTextConst != NULL)
		{
			strncpy(buf, *langTextConst, SCREEN_LINE_BUFFER_SIZE);
		}
		else
		{
			strncpy(buf, " ", SCREEN_LINE_BUFFER_SIZE);
		}

		if ((i == 0) && (langTextConst != NULL))
		{
			voicePromptsAppendLanguageString((const char * const *)langTextConst);
			promptsPlayNotAfterTx();
		}

		menuDisplayEntry(i, mNum, buf);
	}

	ucRender();
}

static void handleSubMenuEvent(uiEvent_t *ev)
{
	if (ev->events & BUTTON_EVENT)
	{
		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}
	}

	// DTMF sequence is playing, stop it.
	if (uiDataGlobal.DTMFContactList.isKeying && ((ev->keys.key != 0) || BUTTONCHECK_DOWN(ev, BUTTON_PTT)
#if ! defined(PLATFORM_RD5R)
													|| BUTTONCHECK_DOWN(ev, BUTTON_ORANGE)
#endif
	))
	{
		uiDataGlobal.DTMFContactList.poLen = 0U;
		return;
	}

	if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
	{
		menuSystemPopPreviousMenu();
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
	{
		contactListOverrideState = MENU_CONTACT_LIST_DISPLAY;
		switch (menuDataGlobal.currentItemIndex)
		{
			case CONTACT_LIST_QUICK_MENU_SELECT:
				if (contactListEntryCount==0)
				{
					uiDataGlobal.currentSelectedContactIndex =0;
					if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
					{
						menuSystemSetCurrentMenu(MENU_CONTACT_DETAILS);
					}
					else
					{// Activate the DTMF Entry screen.
						menuSystemSetCurrentMenu(MENU_NUMERICAL_ENTRY);
					}
					return;
				}
				UpdateChannelWithLastReferencedContact();
				if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
				{
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						overrideWithSelectedContact();
						uiDataGlobal.currentSelectedContactIndex = 0;
						announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC, PROMPT_THRESHOLD_3);
						uiDataGlobal.VoicePrompts.inhibitInitial = true;
						menuSystemPopAllAndDisplayRootMenu();
					}
					else
					{
						menuContactListSubMenuExitCode |= MENU_STATUS_ERROR;
					}
					return;
				}
				else
				{
					uiDataGlobal.VoicePrompts.inhibitInitial = true;
					if (trxGetMode() == RADIO_MODE_ANALOG)
					{
						dtmfSequencePrepare(contactListDTMFContactData.code, true);
					}
					else
					{
						menuContactListSubMenuExitCode |= MENU_STATUS_ERROR;

						return;
					}
				}
				break;
			case CONTACT_LIST_QUICK_MENU_NEW:
				// protect against adding new if already full
				if (((contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL) && (contactListEntryCount==CODEPLUG_CONTACTS_MAX)) ||
					((contactListType == MENU_CONTACT_LIST_CONTACT_DTMF) && (contactListEntryCount==CODEPLUG_DTMF_CONTACTS_MAX)))
				{
					if (nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
					{
						uiDataGlobal.VoicePrompts.inhibitInitial = true;
						voicePromptsInit();
						voicePromptsAppendLanguageString(&currentLanguage->list_full);
						voicePromptsPlay();
					}
					else
						menuContactListSubMenuExitCode |= MENU_STATUS_ERROR;
					return;
				}
				// ok, deliberate fall through.
				
				uiDataGlobal.currentSelectedContactIndex =0; // Deliberate fall through
			case CONTACT_LIST_QUICK_MENU_EDIT:
				sortAfterCreateOrEditContact=(nonVolatileSettings.sortFlags&sortContactsByName) ? true : false;
				if (sortAfterCreateOrEditContact)
				{// if we don't unsort, the functions which find free indices and insert/update the internal cache break!
					nonVolatileSettings.sortFlags&=~sortContactsByName;
					codeplugInitContactsCache(); // force it to be reloaded unsorted!
				}

				if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
				{
					menuSystemSetCurrentMenu(MENU_CONTACT_DETAILS);
				}
				else
				{
					menuSystemSetCurrentMenu(MENU_CONTACT_NEW_DTMF);
				}
				break;
			case CONTACT_LIST_QUICK_MENU_DELETE:
				if (uiDataGlobal.currentSelectedContactIndex > 0)
				{
					if (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL)
					{
						voicePromptsInit();
						if ((contactListContactData.callType == CONTACT_CALLTYPE_TG) &&
								codeplugContactGetRXGroup(contactListContactData.NOT_IN_CODEPLUGDATA_indexNumber))
						{
							menuContactListTimeout = 500;
							contactListOverrideState = MENU_CONTACT_LIST_TG_IN_RXGROUP;
							voicePromptsAppendLanguageString(&currentLanguage->contact_used);
							voicePromptsAppendLanguageString(&currentLanguage->in_rx_group);
						}
						else
						{
							contactListOverrideState = MENU_CONTACT_LIST_CONFIRM;
							voicePromptsAppendLanguageString(&currentLanguage->delete_contact_qm);
						}
						voicePromptsPlay();
					}
					else
					{//DTMF
						contactListOverrideState = MENU_CONTACT_LIST_CONFIRM;
						voicePromptsAppendLanguageString(&currentLanguage->delete_contact_qm);
						voicePromptsPlay();
					}
					menuSystemPopPreviousMenu();
				}
				break;
			case CONTACT_LIST_QUICK_MENU_DTMF_ENTRY:
				menuSystemSetCurrentMenu(MENU_NUMERICAL_ENTRY);
				break;
		}
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
	{
		menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, submenuEntryCount);
		updateSubMenuScreen(false);
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
	{
		menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, submenuEntryCount);
		updateSubMenuScreen(false);
	}

	if ((menuDataGlobal.currentItemIndex == CONTACT_LIST_QUICK_MENU_SELECT) && (contactListType == MENU_CONTACT_LIST_CONTACT_DIGITAL) &&
			(KEYCHECK_SHORTUP_NUMBER(ev->keys) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2))))
	{
		saveQuickkeyContactIndex(ev->keys.key, (uint16_t)contactListContactData.NOT_IN_CODEPLUGDATA_indexNumber);
		return;
	}

}

menuStatus_t menuContactListSubMenu(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		menuDataGlobal.currentItemIndex=0;
		
		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->quick_menu);
		voicePromptsAppendPrompt(PROMPT_SILENCE);

		updateSubMenuScreen(isFirstRun);
		keyboardInit();
		menuContactListSubMenuExitCode = (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuContactListSubMenuExitCode = MENU_STATUS_SUCCESS;

		if (ev->hasEvent)
		{
			handleSubMenuEvent(ev);
		}
	}

	dtmfSequenceTick(true);

	return menuContactListSubMenuExitCode;
}

