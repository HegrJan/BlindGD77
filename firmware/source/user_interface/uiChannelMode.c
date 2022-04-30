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
#include <ctype.h>
#include "functions/codeplug.h"
#include "functions/settings.h"
#include "functions/trx.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"
#include "functions/voicePrompts.h"
#include "functions/ticks.h"
#include "functions/rxPowerSaving.h"


#define NAME_BUFFER_LEN   23
#define NO_ZONE 255
#define NO_CHANNEL -1

#if defined(PLATFORM_GD77S)
typedef enum
{
	GD77S_UIMODE_ZONE,
	GD77S_UIMODE_TG_OR_SQUELCH,
	GD77S_UIMODE_SCAN,
	GD77S_UIMODE_KEYPAD, // virtual keypad
	GD77S_UIMODE_DTMF_CONTACTS,
	GD77S_UIMODE_TS,
	GD77S_UIMODE_CC,
	GD77S_UIMODE_FILTER,
	GD77S_UIMODE_VOX,
	GD77S_UIMODE_OPTIONS1,
	GD77S_UIMODE_OPTIONS2,
	GD77S_UIMODE_MAX
} GD77S_UIMODES_t;

typedef enum
{
	GD77S_OPTION_POWER,
	GD77S_OPTION_BAND_LIMITS,
	GD77S_OPTION_HOTSPOT_MODE,
	GD77S_OPTION_FM_MIC_GAIN,
	GD77S_OPTION_DMR_MIC_GAIN,
	GD77S_OPTION_VHF_SQUELCH,
	GD77S_OPTION_UHF_SQUELCH,
	GD77S_OPTION_CSS_TAIL_ELIM,
	GD77S_OPTION_FM_BEEP,
	GD77S_OPTION_DMR_BEEP,
	GD77S_OPTION_RX_END_BEEP,
	GD77S_OPTION_DTMF_VOL,
	GD77S_OPTION_TOT_MASTER,
	GD77S_OPTION_PTT_LATCH,
	GD77S_OPTION_ECO,
	GD77S_OPTION_AUTOZONE,
	GD77S_OPTION_VOX_THRESHOLD,
	GD77S_OPTION_VOX_TAIL,
	GD77S_OPTION_KEYPAD_SOURCE, //  VFO_A, VFO_B, cur_channel
	GD77S_OPTION_VOLUME,
	GD77S_OPTION_RATE,
	GD77S_OPTION_PHONETIC_SPELL,
	GD77S_OPTION_DMR_ID_ANNOUNCE,
	GD77S_OPTION_CUSTOM_PROMPT_RECORD, // for recording a custom prompt.
	GD77S_OPTION_CUSTOM_PROMPT_REVIEW,
	GD77S_OPTION_EDIT_START,
	GD77S_OPTION_EDIT_END,
	GD77S_OPTION_FIRMWARE_INFO,
	GD77S_OPTION_MAX
} 	GD77S_OPTIONS_t;

typedef struct
{
	bool             firstRun;
	GD77S_UIMODES_t  uiMode;
	bool             channelOutOfBounds;
	uint16_t         dtmfListSelected;
	int32_t          dtmfListCount;
	bool virtualVFOMode;
	uint8_t autoZoneTypeIndex;// when setting up autozones
	uint8_t channelbankOffset; // GD77S only has 16 physical channels thus this is useed to access banks of   16 channels in an autozone with more than 16.
	bool cycleFunctionsInReverse;
	uint8_t option; // used in Options modes.
	// the next two fields are used for autodialer feature.
	uint16_t dialedChannels; // bit 0 ch1, bit 1 ch2, ... bit 15 ch 16.
	uint16_t dialedZones; // bit 0 zone 1, bit 1 zone 2, ... bit 15 zone 16.
	uint8_t customPromptIndex;
} GD77SParameters_t;

static GD77SParameters_t GD77SParameters =
{
		.firstRun = true,
		.uiMode = GD77S_UIMODE_TG_OR_SQUELCH,
		.channelOutOfBounds = false,
		.dtmfListSelected = 0,
		.dtmfListCount = 0,
		.virtualVFOMode=false,
		.autoZoneTypeIndex=1,
		.channelbankOffset=0,
		.cycleFunctionsInReverse=false,
		.option=0,
		.dialedChannels=0,
		.dialedZones=0,
		.customPromptIndex=1
};

static void buildSpeechUiModeForGD77S(GD77S_UIMODES_t uiMode);

static void checkAndUpdateSelectedChannelForGD77S(uint16_t chanNum, bool forceSpeech);
static void handleEventForGD77S(uiEvent_t *ev);
static uint16_t getCurrentChannelInCurrentZoneForGD77S(void);

#else // ! PLATFORM_GD77S

static void handleUpKey(uiEvent_t *ev);
#endif // PLATFORM_GD77S
static void selectPrevNextZone(bool nextZone);

static void handleEvent(uiEvent_t *ev);
static void loadChannelData(bool useChannelDataInMemory, bool loadVoicePromptAnnouncement);

static void updateQuickMenuScreen(bool isFirstRun);
static void handleQuickMenuEvent(uiEvent_t *ev);

static void scanning(void);
static void scanStart(bool longPressBeep);
static void scanSearchForNextChannel(void);
static void scanApplyNextChannel(void);

static void updateTrxID(void);

static char currentZoneName[SCREEN_LINE_BUFFER_SIZE];
static int directChannelNumber = 0;
static int scanStartZone=NO_ZONE; //unset, must use 255 since autozones and all channels zone are negative.
static int scanStartChannel=NO_CHANNEL; // unset

static struct_codeplugChannel_t channelNextChannelData = { .rxFreq = 0 };
static bool nextChannelReady = false;
static int nextChannelIndex = 0;
static int scanZoneIndexApplied=0;
static bool scobAlreadyTriggered = false;
static bool quickmenuChannelFromVFOHandled = false; // Quickmenu new channel confirmation window
static bool quickmenuDeleteChannelHandled=false;
static bool quickmenuDeleteFromAllZones=false;
static bool reorderingChannels=false;

static menuStatus_t menuChannelExitStatus = MENU_STATUS_SUCCESS;
static menuStatus_t menuQuickChannelExitStatus = MENU_STATUS_SUCCESS;
static uint16_t repeaterOffsetDisplayTimeout=0;

struct DualWatchChannelData_t
{
	bool dualWatchActive;
	int16_t watchChannelIndex; // When Dual Watch is activated, this is the index of the channel on which it is activated, i.e. the channel to watch (relative to allChannels zone).
	char watchChannelName[17]; // saved off when initially loaded.
	int16_t currentChannelIndex; // the index of the channel the user has switched to after activating Dual Watch relative to the current zone.
	int16_t dualWatchChannelIndex; // When Dual Watch is active, this index toggles between the above two and holds the index of the channel currently being monitored.
	char currentChannelName[17]; // saved off when loaded.
	bool dualWatchChannelIndexIsRelativeToAllChannelsZone;
	uint16_t dualWatchPauseCountdownTimer; // this timer is used to give voice prompts time to announce the channel that the user selects after dual watch is activated.
	bool allowedToAnnounceChannelDetails; // To avoid white noise while scanning, only changes to the channel made by the user are allowed to be announced.
	bool restartDualWatch; // When the user releases PTT this flag indicates that the Dual Watch should be restarted.
} dualWatchChannelData;

static int GetChannelName(uint16_t index, bool indexIsRelativeToAllChannelsZone, char* buffer, int bufLen)
{
	if (!buffer) return 0;
	*buffer='\0';
	struct_codeplugChannel_t tmpChannelData = { .rxFreq = 0 };
	
	uint16_t indexRelativeToAllChannelsZone=indexIsRelativeToAllChannelsZone ? index : currentZone.channels[index];
	codeplugChannelGetDataForIndex(indexRelativeToAllChannelsZone, &tmpChannelData);
	codeplugUtilConvertBufToString(tmpChannelData.name, buffer, bufLen); // need to convert to zero terminated string
	return strlen(buffer);
	}

static bool ToggleDualWatchChannelIndex()
{
	uint16_t currentChannelIndexRelativeToAllChannelsZone=CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ?dualWatchChannelData.currentChannelIndex : currentZone.channels[dualWatchChannelData.currentChannelIndex];
	// The watch channel is always relative to all Channels zone.
	if (dualWatchChannelData.watchChannelIndex == currentChannelIndexRelativeToAllChannelsZone)
		return false; // User hasn't moved away from the initial channel yet.

	if (dualWatchChannelData.dualWatchChannelIndex == dualWatchChannelData.watchChannelIndex)
	{
		dualWatchChannelData.dualWatchChannelIndex=dualWatchChannelData.currentChannelIndex;
		dualWatchChannelData.dualWatchChannelIndexIsRelativeToAllChannelsZone=CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone);
	}
	else
	{
		dualWatchChannelData.dualWatchChannelIndex=dualWatchChannelData.watchChannelIndex;
		dualWatchChannelData.dualWatchChannelIndexIsRelativeToAllChannelsZone=true;
	}

	return true;
}

static void SetNextChannelIndexAndData(uint16_t channelIndex, bool indexIsRelativeToAllChannelsZone)
{
	// nextChannelIndex is expected to be relative to the current zone.
	// ScanApplyNextChannel saves this back to either nonVolatileSettings.currentChannelIndexInAllZone or nonVolatileSettings.currentChannelIndexInZone
	// Thus if this is the dual watch or priority channel index, it is relative to the allChannelsZone and may not exist in the current zone.
	if (indexIsRelativeToAllChannelsZone && !CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
	{
		uint16_t indexRelativeToCurrentZone;
		if (codeplugFindAllChannelsIndexInCurrentZone(channelIndex, &indexRelativeToCurrentZone))
			nextChannelIndex=indexRelativeToCurrentZone;
		else
			nextChannelIndex=nonVolatileSettings.currentChannelIndexInZone; // leave it unchanged for the purposes of saving it.
	}
	else
		nextChannelIndex =channelIndex;

	if (indexIsRelativeToAllChannelsZone)
		codeplugChannelGetDataForIndex(channelIndex, &channelNextChannelData);
	else
		codeplugChannelGetDataForIndex(currentZone.channels[channelIndex], &channelNextChannelData);

	nextChannelReady = true;
}

static bool DoDualWatchScan()
{
	if (!dualWatchChannelData.dualWatchActive)
		return false; // It's not active, perhaps another scan mode is active which will then be handled.
	// If the voice is playing, wait until it is finished before continuing.
	if (voicePromptsIsPlaying())
		return true;
	if (dualWatchChannelData.dualWatchPauseCountdownTimer > 0)
	{
		dualWatchChannelData.dualWatchPauseCountdownTimer--;
		return true;
	}

	if (!ToggleDualWatchChannelIndex())
	return true; // User hasn't yet moved away from the initial channel, but no other scan mode should be handled.
	// At this point we have set the dualWatchChannelData.dualWatchChannelIndex to the other channel to watch.
	// Now load it's data.
	SetNextChannelIndexAndData(dualWatchChannelData.dualWatchChannelIndex, dualWatchChannelData.dualWatchChannelIndexIsRelativeToAllChannelsZone);

	return true;
}

#if ! defined(PLATFORM_GD77S)
static void EnsureDualWatchReturnsToCurrent()
{
	// return to the last channel selected by the user.
	dualWatchChannelData.dualWatchChannelIndexIsRelativeToAllChannelsZone = CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone);
	SetNextChannelIndexAndData(dualWatchChannelData.currentChannelIndex, dualWatchChannelData.dualWatchChannelIndexIsRelativeToAllChannelsZone);
	scanApplyNextChannel();
}
#endif // ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own

static void InitDualWatchData()
{
	memset(&dualWatchChannelData, 0, sizeof(dualWatchChannelData));
}

#if ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own
static void AnnounceDualWatchChannels(bool immediately)
{
	voicePromptsInit();

	if (dualWatchChannelData.watchChannelIndex==uiDataGlobal.priorityChannelIndex)
		voicePromptsAppendLanguageString(&currentLanguage->priorityScan);
	else
		voicePromptsAppendLanguageString(&currentLanguage->dual_watch);
	voicePromptsAppendString(dualWatchChannelData.currentChannelName);
	voicePromptsAppendPrompt(PROMPT_SILENCE);
	voicePromptsAppendString(dualWatchChannelData.watchChannelName);

	if (immediately)
		voicePromptsPlay();
}
#endif // ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own

static void StartDualWatch(uint16_t watchChannelIndex, uint16_t currentChannelIndex, uint16_t startDelay)
{
	dualWatchChannelData.dualWatchActive=true;
	dualWatchChannelData.allowedToAnnounceChannelDetails=false; // set to true when arrowing to a new channel during a dual watch.
	//When the user chooses a new current channel, the dual watch will begin automatically.
	dualWatchChannelData.watchChannelIndex = watchChannelIndex;
	dualWatchChannelData.currentChannelIndex = currentChannelIndex;
	dualWatchChannelData.dualWatchChannelIndex = watchChannelIndex;
	dualWatchChannelData.dualWatchChannelIndexIsRelativeToAllChannelsZone=true;
	GetChannelName(watchChannelIndex, true, dualWatchChannelData.watchChannelName, 17);
	GetChannelName(currentChannelIndex, CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone), dualWatchChannelData.currentChannelName, 17);
	
	scanStart(false);

	uiDataGlobal.Scan.scanType = SCAN_TYPE_DUAL_WATCH;

	nextChannelIndex = dualWatchChannelData.watchChannelIndex;
	nextChannelReady = false;

	int  currentPowerSavingLevel = rxPowerSavingGetLevel();
	if (currentPowerSavingLevel > 1)
	{
		rxPowerSavingSetLevel(currentPowerSavingLevel -1);
	}
	dualWatchChannelData.dualWatchPauseCountdownTimer=startDelay; // give time for initial voice prompt to speak.
}

#if ! defined(PLATFORM_GD77S)
static void StopDualWatch(bool returnToCurrentChannel)
{
	if (dualWatchChannelData.dualWatchActive==false)
		return;

	if (returnToCurrentChannel)
	{
		scanStartZone = NO_ZONE;
		scanStartChannel = NO_CHANNEL;
		EnsureDualWatchReturnsToCurrent();
	}
	uiChannelModeStopScanning();
	InitDualWatchData();
	rxPowerSavingSetLevel(nonVolatileSettings.ecoLevel);
}

static void SetDualWatchCurrentChannelIndex(uint16_t currentChannelIndex)
{// We have to do this because when in Dual Watch, voice prompts from Load Channel are suppressed.
	dualWatchChannelData.dualWatchPauseCountdownTimer=2000;
	dualWatchChannelData.allowedToAnnounceChannelDetails=true;
	dualWatchChannelData.currentChannelIndex=currentChannelIndex;
		
	GetChannelName(currentChannelIndex, CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone), dualWatchChannelData.currentChannelName, 17);
	uiDataGlobal.priorityChannelActive=false; // in case it was set by long press red.
	
	uiDataGlobal.Scan.timer =500; // force scan to continue;
	uiDataGlobal.repeaterOffsetDirection=0; // reset this as the current channel just changed.
}
#else
static bool ModeUsesChannelKnob()
{
	switch (GD77SParameters.uiMode)
	{
	case GD77S_UIMODE_KEYPAD:
	case GD77S_UIMODE_OPTIONS1:
	case GD77S_UIMODE_OPTIONS2:
	return true;
	default:
	break;
	}
	return false;
}
#endif // ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own

static void EnsurePriorityChannelIsSet()
{
		if (AutoZoneIsCurrentZone(currentZone.NOT_IN_CODEPLUGDATA_indexNumber))
	{
		if (autoZone.priorityChannelIndex > 0)
			uiDataGlobal.priorityChannelIndex=autoZone.priorityChannelIndex;
		else
			uiDataGlobal.priorityChannelIndex=NO_PRIORITY_CHANNEL;
	}
	else
	{
	uiDataGlobal.priorityChannelIndex=nonVolatileSettings.priorityChannelIndex;
	}
}

menuStatus_t uiChannelMode(uiEvent_t *ev, bool isFirstRun)
{
	static uint32_t m = 0, sqm = 0;

	if (isFirstRun)
	{
#if ! defined(PLATFORM_GD77S) // GD77S speech can be triggered in main(), so let it ends.
		voicePromptsTerminate();
#else
		GD77SParameters.channelbankOffset = (nonVolatileSettings.currentChannelIndexInZone/16)*16;
#endif

		settingsSet(nonVolatileSettings.initialMenuNumber, (uint8_t) UI_CHANNEL_MODE);// This menu.
		uiDataGlobal.displayChannelSettings = false;
		uiDataGlobal.reverseRepeater = false;
		nextChannelReady = false;
		uiDataGlobal.displaySquelch = false;
		uiDataGlobal.Scan.refreshOnEveryStep = false;
		// We're in digital mode, RXing, and current talker is already at the top of last heard list,
		// hence immediately display complete contact/TG info on screen
		// This mostly happens when getting out of a menu.
		uiDataGlobal.displayQSOState = (isQSODataAvailableForCurrentTalker() ? QSO_DISPLAY_CALLER_DATA : QSO_DISPLAY_DEFAULT_SCREEN);

		lastHeardClearLastID();
		uiDataGlobal.displayQSOStatePrev = QSO_DISPLAY_IDLE;
		currentChannelData = &channelScreenChannelData;// Need to set this as currentChannelData is used by functions called by loadChannelData()

		if (currentChannelData->rxFreq != 0)
		{
			loadChannelData(true, false);
		}
		else
		{
			uiChannelInitializeCurrentZone();
			loadChannelData(false, false);
		}

		if ((uiDataGlobal.displayQSOState == QSO_DISPLAY_CALLER_DATA) && (trxGetMode() == RADIO_MODE_ANALOG))
		{
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
		}

#if defined(PLATFORM_GD77S)
		//Reset to first UiMode if the radio is in Analog mode and the current UiMode only applies to DMR
		if ((trxGetMode() == RADIO_MODE_ANALOG) &&
			((GD77SParameters.uiMode == GD77S_UIMODE_TS ) ||
			(GD77SParameters.uiMode == GD77S_UIMODE_CC ) ||
			(GD77SParameters.uiMode == GD77S_UIMODE_FILTER )))
		{
			GD77SParameters.uiMode = GD77S_UIMODE_TG_OR_SQUELCH;
		}

		// Ensure the correct channel is loaded, on the very first run
		if (GD77SParameters.firstRun)
		{
			if (voicePromptsIsPlaying() == false)
			{
				GD77SParameters.firstRun = false;
				checkAndUpdateSelectedChannelForGD77S(rotarySwitchGetPosition()+GD77SParameters.channelbankOffset, true);
			}

			GD77SParameters.dtmfListCount = codeplugDTMFContactsGetCount();
		}
#endif
		uiChannelModeUpdateScreen(0);

		if (uiDataGlobal.Scan.active == false)
		{
			uiDataGlobal.Scan.state = SCAN_SCANNING;
		}

		// Need to do this last, as other things in the screen init, need to know whether the main screen has just changed
		if (uiDataGlobal.VoicePrompts.inhibitInitial)
		{
			uiDataGlobal.VoicePrompts.inhibitInitial = false;
		}

		// Scan On Boot is enabled, but has to be run only once.
		if ((settingsIsOptionBitSet(BIT_SCAN_ON_BOOT_ENABLED) || settingsIsOptionBitSet(BIT_PRI_SCAN_ON_BOOT_ENABLED))&& (scobAlreadyTriggered == false))
		{
			if (settingsIsOptionBitSet(BIT_PRI_SCAN_ON_BOOT_ENABLED))
			{
				uint16_t priorityChannelIndex= uiDataGlobal.priorityChannelIndex;
				uint16_t currentChannelIndex= CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : nonVolatileSettings.currentChannelIndexInZone;

				if (priorityChannelIndex != NO_PRIORITY_CHANNEL)
				{
					StartDualWatch(priorityChannelIndex, currentChannelIndex, 1000);
				}
			}
			else
				scanStart(false);
		}

		// Disable ScOB for this session, and also prevent false triggering (like exiting the Options screen)
		scobAlreadyTriggered = true;

		menuChannelExitStatus = MENU_STATUS_SUCCESS; // Due to Orange Quick Menu
	}
	else
	{
		menuChannelExitStatus = MENU_STATUS_SUCCESS;
		
		if (uiDataGlobal.displayChannelSettings)
		{
			if (repeaterOffsetDisplayTimeout > 0)
			{
				repeaterOffsetDisplayTimeout--;
			}
			else
			{
				uiDataGlobal.displayChannelSettings=false;
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}
		}

#if defined(PLATFORM_GD77S)
		uiChannelModeHeartBeatActivityForGD77S(ev);
#endif

		if (ev->events == NO_EVENT)
		{
#if defined(PLATFORM_GD77S)
			// Just ensure rotary's selected channel is matching the already loaded one
			// as rotary selector could be turned while the GD is OFF, or in hotspot mode.
			if (((uiDataGlobal.Scan.active == false) && !ModeUsesChannelKnob() && (rotarySwitchGetPosition()+GD77SParameters.channelbankOffset != getCurrentChannelInCurrentZoneForGD77S())) || (GD77SParameters.firstRun == true))
			{
				if (voicePromptsIsPlaying() == false)
				{
					checkAndUpdateSelectedChannelForGD77S(rotarySwitchGetPosition()+GD77SParameters.channelbankOffset, GD77SParameters.firstRun);

					// Opening channel number announce has not took place yet, probably because it was telling
					// parameter like new hotspot mode selection.
					if (GD77SParameters.firstRun)
					{
						GD77SParameters.firstRun = false;
					}
				}
			}
#endif

			// is there an incoming DMR signal
			if (uiDataGlobal.displayQSOState != QSO_DISPLAY_IDLE)
			{
				uiChannelModeUpdateScreen(0);
			}
			else
			{
				// Clear squelch region
				if (uiDataGlobal.displaySquelch && ((ev->time - sqm) > 1000))
				{
					uiDataGlobal.displaySquelch = false;
					uiUtilityDisplayInformation(NULL, DISPLAY_INFO_SQUELCH_CLEAR_AREA, -1);
					ucRenderRows(2, 4);
				}

				if ((ev->time - m) > RSSI_UPDATE_COUNTER_RELOAD)
				{
					m = ev->time;

					if (uiDataGlobal.Scan.active && (uiDataGlobal.Scan.state == SCAN_PAUSED))
					{
#if defined(PLATFORM_RD5R)
						ucClearRows(0, 1, false);
#else
						ucClearRows(0, 2, false);
#endif
						uiUtilityRenderHeader(notScanning);
					}
					else
					{
						uiUtilityDrawRSSIBarGraph();
					}

					// Only render the second row which contains the bar graph, if we're not scanning,
					// as there is no need to redraw the rest of the screen
					ucRenderRows(((uiDataGlobal.Scan.active && (uiDataGlobal.Scan.state == SCAN_PAUSED)) ? 0 : 1), 2);
				}
			}

			if (uiDataGlobal.Scan.active == true)
			{
				scanning();
			}
			else if (dualWatchChannelData.dualWatchActive==false && dualWatchChannelData.restartDualWatch && ((ev->buttons & BUTTON_PTT)==0) && (getAudioAmpStatus()==0))
			{
				if (dualWatchChannelData.dualWatchPauseCountdownTimer > 0)
					dualWatchChannelData.dualWatchPauseCountdownTimer--;
				else
				{
					uint16_t currentChannelIndex= CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : nonVolatileSettings.currentChannelIndexInZone;
					StartDualWatch(dualWatchChannelData.watchChannelIndex, currentChannelIndex, 1000);
				}
			}
		}
		else
		{
			if (ev->hasEvent)
			{
				if ((trxGetMode() == RADIO_MODE_ANALOG) &&
						(ev->events & KEY_EVENT) && ((ev->keys.key == KEY_LEFT) || (ev->keys.key == KEY_RIGHT)))
				{
					sqm = ev->time;
				}

				handleEvent(ev);
			}
		}

#if defined(PLATFORM_GD77S)
		dtmfSequenceTick(false);
#endif
	}
	return menuChannelExitStatus;
}

#if 0 // rename: we have an union declared (fw_sound.c) with the same name.
uint16_t byteSwap16(uint16_t in)
{
	return ((in & 0xff << 8) | (in >> 8));
}
#endif

static bool canCurrentZoneBeScanned(int *availableChannels)
{
	int enabledChannels = 0;
	int chanIdx = (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : currentZone.channels[nonVolatileSettings.currentChannelIndexInZone]);

	if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 1)
	{
		int chansInZone = currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone;

		if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
		{
			do
			{
				do
				{
					chanIdx = ((((chanIdx - 1) + 1) % currentZone.NOT_IN_CODEPLUGDATA_highestIndex) + 1);
				} while (!codeplugAllChannelsIndexIsInUse(chanIdx));

				chansInZone--;
				// Get flag4 only
				codeplugChannelGetDataWithOffsetAndLengthForIndex(chanIdx, &channelNextChannelData, CODEPLUG_CHANNEL_FLAG4_OFFSET, 1);

				if (CODEPLUG_CHANNEL_IS_FLAG_SET(&channelNextChannelData, CODEPLUG_CHANNEL_FLAG_ALL_SKIP) == false)
				{
					enabledChannels++;
				}

			} while (chansInZone > 0);
		}
		else
		{
			do
			{
				chanIdx = ((chanIdx + 1) % currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone);

				chansInZone--;
				// Get flag4 only
				codeplugChannelGetDataWithOffsetAndLengthForIndex(currentZone.channels[chanIdx], &channelNextChannelData, CODEPLUG_CHANNEL_FLAG4_OFFSET, 1);

				if (CODEPLUG_CHANNEL_IS_FLAG_SET(&channelNextChannelData, CODEPLUG_CHANNEL_FLAG_ZONE_SKIP) == false)
				{
					enabledChannels++;
				}

			} while (chansInZone > 0);
		}
	}

	*availableChannels = enabledChannels;

	return (enabledChannels > 1);
}

static bool SetNextZoneToScanIfNeeded(int curChannelIndex)
{
	// detect and handle multizone scan.
	if (!uiDataGlobal.Scan.scanAllZones) return false;

	bool loadNextZone=false;

	if (uiDataGlobal.Scan.direction == 1)	
	{
		loadNextZone=nextChannelIndex <= curChannelIndex;
	}
	else
	{
		loadNextZone=nextChannelIndex >= curChannelIndex;
	}
	
	if (!loadNextZone) return false;
	
	// record the current zone in case we cycle back to the same zone.
	int prevZoneIndex=nonVolatileSettings.currentZone;
	do 
	{
		selectPrevNextZone(uiDataGlobal.Scan.direction == 1);
		if (nonVolatileSettings.currentZone==prevZoneIndex)
			return false;
		if (nextChannelIndex > currentZone.NOT_IN_CODEPLUGDATA_highestIndex)
			nextChannelIndex =currentZone.NOT_IN_CODEPLUGDATA_highestIndex;
#if  defined(PLATFORM_GD77S)
		GD77SParameters.channelbankOffset=(nextChannelIndex/16)*16;
#endif
	} while (canCurrentZoneBeScanned(&uiDataGlobal.Scan.availableChannelsCount) == false || CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone));
	return true;
}

static void AddFrequencyToNuisanceList(uint32_t freq)
{
				// There is two channels available in the Zone, just stop scanning
	if (!uiDataGlobal.Scan.scanAllZones && uiDataGlobal.Scan.nuisanceDeleteIndex == (uiDataGlobal.Scan.availableChannelsCount - 2))
	{
		uiDataGlobal.Scan.lastIteration = true;
	}

	uiDataGlobal.Scan.nuisanceDelete[uiDataGlobal.Scan.nuisanceDeleteIndex] = freq;
	uiDataGlobal.Scan.nuisanceDeleteIndex = (uiDataGlobal.Scan.nuisanceDeleteIndex + 1) % MAX_ZONE_SCAN_NUISANCE_CHANNELS;
	uiDataGlobal.Scan.timer = SCAN_SKIP_CHANNEL_INTERVAL;	//force scan to continue;
	uiDataGlobal.Scan.state = SCAN_SCANNING;
	keyboardReset();
}

#if  defined(PLATFORM_GD77S) // GD77S vfo scan
static bool InitGD77SScan()
{
	if (trxGetBandFromFrequency(settingsVFOChannel[CHANNEL_VFO_A].rxFreq)==-1)
	{
		voicePromptsInit();
		voicePromptsAppendPrompt(PROMPT_SCAN_MODE);
		voicePromptsAppendLanguageString(&currentLanguage->error);
		voicePromptsPlay();	
		return false;
	}
	nonVolatileSettings.vfoScanLow[CHANNEL_VFO_A]=settingsVFOChannel[CHANNEL_VFO_A].rxFreq;
	nonVolatileSettings.vfoScanHigh[CHANNEL_VFO_A] = nonVolatileSettings.vfoScanLow[CHANNEL_VFO_A]+100000;// 1MHz.
	while (trxGetBandFromFrequency(nonVolatileSettings.vfoScanHigh[CHANNEL_VFO_A])==-1)
	{
		nonVolatileSettings.vfoScanHigh[CHANNEL_VFO_A]--;
	}
	
	return true;
}

static bool HandleGD77SScanning()
{
	if (!GD77SParameters.virtualVFOMode)
return false;
	uint32_t freq=settingsVFOChannel[CHANNEL_VFO_A].rxFreq;
	bool wide=(settingsVFOChannel[CHANNEL_VFO_A].flag4&0x02) ? true : false;
		
	int fStep=wide ? 2500 : 1250; // 25 KHz or 12.5 KHz depending on bandwidth.
	if (uiDataGlobal.Scan.direction == 1)
	{
		if (freq < (nonVolatileSettings.vfoScanHigh[CHANNEL_VFO_A]-fStep))
			freq +=fStep;
		else
			freq =nonVolatileSettings.vfoScanLow[CHANNEL_VFO_A];
	}
	else
	{
		if (freq > (nonVolatileSettings.vfoScanLow[CHANNEL_VFO_A]+fStep))
			freq -=fStep;
		else
			freq = nonVolatileSettings.vfoScanHigh[CHANNEL_VFO_A];
	}
	uint8_t vfo=nonVolatileSettings.currentVFONumber;
	if (vfo > 1)
		vfo=1;
	settingsVFOChannel[vfo].txFreq=settingsVFOChannel[vfo].rxFreq=freq;
	//check all nuisance delete entries and skip channel if there is a match
	if (ScanShouldSkipFrequency(freq))
		return true;
	memcpy(&channelScreenChannelData, &settingsVFOChannel[vfo], CODEPLUG_CHANNEL_DATA_STRUCT_SIZE);
	trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
	nextChannelReady =true;
	
	return true;
}
#endif

static void scanSearchForNextChannel(void)
{
	if (DoDualWatchScan())
		return;
#if  defined(PLATFORM_GD77S) // GD77S vfo scan
	if (HandleGD77SScanning())
	{
		return;
	}
#endif

	int curChannelIndex=nextChannelIndex;
	// All Channels virtual zone
	if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
	{
		do
		{
			do
			{
				// rollover (up/down) CODEPLUG_CHANNELS_MIN .. currentZone.NOT_IN_CODEPLUGDATA_highestIndex
				nextChannelIndex = ((uiDataGlobal.Scan.direction == 1) ?
						((((nextChannelIndex - 1) + 1) % currentZone.NOT_IN_CODEPLUGDATA_highestIndex) + 1) :
						((((nextChannelIndex - 1) + currentZone.NOT_IN_CODEPLUGDATA_highestIndex - 1) % currentZone.NOT_IN_CODEPLUGDATA_highestIndex) + 1));
				if (SetNextZoneToScanIfNeeded(curChannelIndex))
					curChannelIndex=nextChannelIndex;
			} while (!codeplugAllChannelsIndexIsInUse(nextChannelIndex));

			// Check if the channel is skipped.
			// Get flag4 only
			codeplugChannelGetDataWithOffsetAndLengthForIndex(nextChannelIndex, &channelNextChannelData, CODEPLUG_CHANNEL_FLAG4_OFFSET, 1);
		} while (CODEPLUG_CHANNEL_IS_FLAG_SET(&channelNextChannelData, CODEPLUG_CHANNEL_FLAG_ALL_SKIP));

		codeplugChannelGetDataForIndex(nextChannelIndex, &channelNextChannelData);
	}
	else
	{
		do
		{
			// rollover (up/down) 0 .. (currentZone.NOT_IN_CODEPLUGDATA_highestIndex - 1)
			nextChannelIndex = ((uiDataGlobal.Scan.direction == 1) ?
					((nextChannelIndex + 1) % currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone) :
					((nextChannelIndex + currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone - 1) % currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone));
			if (SetNextZoneToScanIfNeeded(curChannelIndex))
				curChannelIndex=nextChannelIndex;
			
			// Check if the channel is skipped.
			// Get flag4 only
			codeplugChannelGetDataWithOffsetAndLengthForIndex(currentZone.channels[nextChannelIndex], &channelNextChannelData, CODEPLUG_CHANNEL_FLAG4_OFFSET, 1);

		} while (CODEPLUG_CHANNEL_IS_FLAG_SET(&channelNextChannelData, CODEPLUG_CHANNEL_FLAG_ZONE_SKIP));

		codeplugChannelGetDataForIndex(currentZone.channels[nextChannelIndex], &channelNextChannelData);
	}

	//check all nuisance delete entries and skip channel if there is a match
	if (ScanShouldSkipFrequency(channelNextChannelData.rxFreq)) 
		return;
	
	nextChannelReady = true;
}

static void scanApplyNextChannel(void)
{
	#if  defined(PLATFORM_GD77S) // GD77S vfo scan
	if (GD77SParameters.virtualVFOMode==false)
	{
#endif

	if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
	{
		// All Channels virtual zone
		settingsSet(nonVolatileSettings.currentChannelIndexInAllZone, (int16_t) nextChannelIndex);
	}
	else
	{
		settingsSetCurrentChannelIndexForZone((int16_t) nextChannelIndex, nonVolatileSettings.currentZone);
	}
	scanZoneIndexApplied=nonVolatileSettings.currentZone;
	lastHeardClearLastID();

	memcpy(&channelScreenChannelData, &channelNextChannelData, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE);
#if  defined(PLATFORM_GD77S) // GD77S vfo scan
	}
#endif
	loadChannelData(true, false);
	uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
	uiChannelModeUpdateScreen(0);

	if ((trxGetMode() == RADIO_MODE_DIGITAL) && (trxDMRModeRx != DMR_MODE_SFR) && (uiDataGlobal.Scan.stepTimeMilliseconds < SCAN_DMR_SIMPLEX_MIN_INTERVAL))
	{
		uiDataGlobal.Scan.timerReload = SCAN_DMR_SIMPLEX_MIN_INTERVAL;
	}
	else
	{
		uiDataGlobal.Scan.timerReload = uiDataGlobal.Scan.stepTimeMilliseconds;
	}
	uiDataGlobal.Scan.timer = uiDataGlobal.Scan.timerReload;
	uiDataGlobal.Scan.state = SCAN_SCANNING;
	nextChannelReady = false;
}

#if ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own
static bool AnnounceChannelInfoImmediately()
{
	int nextMenu = menuSystemGetPreviouslyPushedMenuNumber();

	if (nextMenu == UI_TX_SCREEN)
	return false;
	if (nextMenu == UI_PRIVATE_CALL)
		return false;
	if (nextMenu == UI_LOCK_SCREEN)
		return false;
	if (uiDataGlobal.Scan.active && (uiDataGlobal.Scan.scanType == SCAN_TYPE_NORMAL_STEP))
	return false;
	return true;
}

static bool AllowedToAnnounceChannelInfo(bool loadVoicePromptAnnouncement)
{
	if (uiDataGlobal.VoicePrompts.inhibitInitial && !loadVoicePromptAnnouncement)
	return false;

	if (!dualWatchChannelData.dualWatchActive)
	{
		if (uiDataGlobal.Scan.active == false)
			return true;

		if (uiDataGlobal.Scan.state == SCAN_PAUSED || uiDataGlobal.Scan.state == SCAN_SHORT_PAUSED)
			return true;
		return false;
	}

	if (dualWatchChannelData.dualWatchPauseCountdownTimer==0)
		dualWatchChannelData.allowedToAnnounceChannelDetails=false; // It means the channel is being loaded as part of the scan and shouldn't be announced.

	return dualWatchChannelData.allowedToAnnounceChannelDetails;
}
#endif // ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own

static void loadChannelData(bool useChannelDataInMemory, bool loadVoicePromptAnnouncement)
{
	bool rxGroupValid = true;

	if (uiDataGlobal.priorityChannelActive)
	{
		codeplugChannelGetDataForIndex(uiDataGlobal.priorityChannelIndex, &channelScreenChannelData);
		uiDataGlobal.currentSelectedChannelNumber=uiDataGlobal.priorityChannelIndex;
	}
	else if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
	{
		// All Channels virtual zone
		uiDataGlobal.currentSelectedChannelNumber = nonVolatileSettings.currentChannelIndexInAllZone;
	}
	else
	{
		uiDataGlobal.currentSelectedChannelNumber = currentZone.channels[nonVolatileSettings.currentChannelIndexInZone];
	}

	if (!useChannelDataInMemory)
	{
		memset(&lastDialledDTMFContact, 0xffu, sizeof(lastDialledDTMFContact)); // loading a new channel, clear this out.

		if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
		{
			// All Channels virtual zone
			codeplugChannelGetDataForIndex(nonVolatileSettings.currentChannelIndexInAllZone, &channelScreenChannelData);
		}
		else
		{
			codeplugChannelGetDataForIndex(currentZone.channels[nonVolatileSettings.currentChannelIndexInZone], &channelScreenChannelData);
		}
	}

	clearActiveDMRID();
	trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
	trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4 & 0x02) == 0x02));

	if (currentChannelData->chMode == RADIO_MODE_ANALOG)
	{
		trxSetRxCSS(currentChannelData->rxTone);
	}
	else
	{
		uint32_t channelDMRId = codeplugChannelGetOptionalDMRID(currentChannelData);

		if (uiDataGlobal.manualOverrideDMRId == 0)
		{
			if (channelDMRId == 0)
			{
				trxDMRID = uiDataGlobal.userDMRId;
			}
			else
			{
				trxDMRID = channelDMRId;
			}
		}
		else
		{
			trxDMRID = uiDataGlobal.manualOverrideDMRId;
		}


		trxSetDMRColourCode(currentChannelData->txColor);
		if (currentChannelData->rxGroupList != lastLoadedRxGroup)
		{
			rxGroupValid = codeplugRxGroupGetDataForIndex(currentChannelData->rxGroupList, &currentRxGroupData);
			if (rxGroupValid)
			{
				lastLoadedRxGroup = currentChannelData->rxGroupList;
			}
			else
			{
				lastLoadedRxGroup = -1;// RxGroup Contacts are not valid
			}
		}

		// Current contact index is out of group list bounds, select first contact
		if (rxGroupValid && (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] > (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup - 1)))
		{
			settingsSet(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 0);
			menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
		}

		// Check if this channel has an Rx Group
		if (rxGroupValid && nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] < currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup)
		{
			codeplugContactGetDataForIndex(currentRxGroupData.contacts[nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE]], &currentContactData);
		}
		else
		{
			codeplugContactGetDataForIndex(currentChannelData->contact, &currentContactData);
		}

		trxUpdateTsForCurrentChannelWithSpecifiedContact(&currentContactData);

		if (nonVolatileSettings.overrideTG == 0)
		{
			trxTalkGroupOrPcId = currentContactData.tgNumber;

			if (currentContactData.callType == CONTACT_CALLTYPE_PC)
			{
				trxTalkGroupOrPcId |= (PC_CALL_FLAG << 24);
			}
		}
		else
		{
			trxTalkGroupOrPcId = nonVolatileSettings.overrideTG;
		}
	}

#if ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own
	if (AllowedToAnnounceChannelInfo(loadVoicePromptAnnouncement))
	{
		announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE, AnnounceChannelInfoImmediately() ? PROMPT_THRESHOLD_2 : PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY);
		dualWatchChannelData.allowedToAnnounceChannelDetails=false;// once announced, disallow until user explicitly changes the channel.
	}
#endif

}

void uiChannelModeUpdateScreen(int txTimeSecs)
{
	int channelNumber;
	char nameBuf[NAME_BUFFER_LEN];
	char buffer[SCREEN_LINE_BUFFER_SIZE];

	// Only render the header, then wait for the next run
	// Otherwise the screen could remain blank if TG and PC are == 0
	// since uiDataGlobal.displayQSOState won't be set to QSO_DISPLAY_IDLE
	if ((trxGetMode() == RADIO_MODE_DIGITAL) && (HRC6000GetReceivedTgOrPcId() == 0) &&
			((uiDataGlobal.displayQSOState == QSO_DISPLAY_CALLER_DATA) || (uiDataGlobal.displayQSOState == QSO_DISPLAY_CALLER_DATA_UPDATE)))
	{
		uiUtilityRedrawHeaderOnly(notScanning);
		return;
	}

	// We're currently displaying details, and it shouldn't be overridden by QSO data
	if (uiDataGlobal.displayChannelSettings && ((uiDataGlobal.displayQSOState == QSO_DISPLAY_CALLER_DATA)
			|| (uiDataGlobal.displayQSOState == QSO_DISPLAY_CALLER_DATA_UPDATE)))
	{
		// We will not restore the previous QSO Data as a new caller just arose.
		uiDataGlobal.displayQSOStatePrev = QSO_DISPLAY_DEFAULT_SCREEN;
		uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
	}

	ucClearBuf();
	uiUtilityRenderHeader(dualWatchChannelData.dualWatchActive ? (dualWatchChannelData.watchChannelIndex==uiDataGlobal.priorityChannelIndex ? channelPriorityScan :  channelDualWatch) : notScanning);

	switch(uiDataGlobal.displayQSOState)
	{
		case QSO_DISPLAY_DEFAULT_SCREEN:
			uiDataGlobal.displayQSOStatePrev = QSO_DISPLAY_DEFAULT_SCREEN;
			uiDataGlobal.isDisplayingQSOData = false;
			uiDataGlobal.receivedPcId = 0x00;
			if (trxTransmissionEnabled)
			{
				// Squelch is displayed, PTT was pressed
				// Clear its region
				if (uiDataGlobal.displaySquelch)
				{
					uiDataGlobal.displaySquelch = false;
					uiUtilityDisplayInformation(NULL, DISPLAY_INFO_SQUELCH_CLEAR_AREA, -1);
				}

				snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, " %d ", txTimeSecs);
				uiUtilityDisplayInformation(buffer, DISPLAY_INFO_TX_TIMER, -1);
			}
			else
			{
				// Display some channel settings
				if (uiDataGlobal.displayChannelSettings)
				{
					uiUtilityDisplayInformation(NULL, DISPLAY_INFO_TONE_AND_SQUELCH, -1);

					uiUtilityDisplayFrequency(DISPLAY_Y_POS_RX_FREQ, false, false, (uiDataGlobal.reverseRepeater ? currentChannelData->txFreq : currentChannelData->rxFreq), false, false, 0);
					uiUtilityDisplayFrequency(DISPLAY_Y_POS_TX_FREQ, true, false, (uiDataGlobal.reverseRepeater ? currentChannelData->rxFreq : currentChannelData->txFreq), false, false, 0);
				}
				else
				{
					if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
					{
						// All Channels virtual zone
						channelNumber = nonVolatileSettings.currentChannelIndexInAllZone;
						if (directChannelNumber > 0)
						{
							snprintf(nameBuf, NAME_BUFFER_LEN, "%s %d", currentLanguage->gotoChannel, directChannelNumber);
						}
						else
						{
							snprintf(nameBuf, NAME_BUFFER_LEN, "%s Ch:%d",currentLanguage->all_channels, channelNumber);
						}
					}
					else
					{
						channelNumber = nonVolatileSettings.currentChannelIndexInZone + 1;
						if (directChannelNumber > 0)
						{
							snprintf(nameBuf, NAME_BUFFER_LEN, "%s %d", currentLanguage->gotoChannel, directChannelNumber);
						}
						else
						{
							snprintf(nameBuf, NAME_BUFFER_LEN, "%s Ch:%d", currentZoneName, channelNumber);
						}
					}
					uiUtilityDisplayInformation(nameBuf, DISPLAY_INFO_ZONE, -1);

				/*
				 * Roger Clark. Commented out
				 * functionality to display "Scanning" instead of the channel name  (and TG on DMR) whilst scanning
				 * if (uiDataGlobal.Scan.active && (uiDataGlobal.Scan.state == SCAN_SCANNING))
					{
						uiUtilityDisplayInformation(currentLanguage->scanning, DISPLAY_INFO_CONTACT, -1);
						ucRender();
						break;
					}*/
				}
			}

			if (!uiDataGlobal.displayChannelSettings)
			{
				codeplugUtilConvertBufToString(currentChannelData->name, nameBuf, 16);
				uiUtilityDisplayInformation(nameBuf, DISPLAY_INFO_CHANNEL, (trxTransmissionEnabled ? DISPLAY_Y_POS_CHANNEL_SECOND_LINE : -1));
			}

			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				if (uiDataGlobal.displayChannelSettings)
				{
					uint32_t PCorTG = ((nonVolatileSettings.overrideTG != 0) ? nonVolatileSettings.overrideTG : currentContactData.tgNumber);

					snprintf(nameBuf, NAME_BUFFER_LEN, "%s %u",
							(((PCorTG >> 24) == PC_CALL_FLAG) ? currentLanguage->pc : currentLanguage->tg),
							(PCorTG & 0xFFFFFF));
				}
				else
				{
					if (nonVolatileSettings.overrideTG != 0)
					{
						uiUtilityBuildTgOrPCDisplayName(nameBuf, SCREEN_LINE_BUFFER_SIZE);
						uiUtilityDisplayInformation(NULL, DISPLAY_INFO_CONTACT_OVERRIDE_FRAME, (trxTransmissionEnabled ? DISPLAY_Y_POS_CONTACT_TX_FRAME : -1));
					}
					else
					{
						codeplugUtilConvertBufToString(currentContactData.name, nameBuf, 16);
					}
				}

				uiUtilityDisplayInformation(nameBuf, DISPLAY_INFO_CONTACT, (trxTransmissionEnabled ? DISPLAY_Y_POS_CONTACT_TX : -1));
			}
			// Squelch will be cleared later, 1s after last change
			else if(uiDataGlobal.displaySquelch && !trxTransmissionEnabled && !uiDataGlobal.displayChannelSettings)
			{
				strncpy(buffer, currentLanguage->squelch, 9);
				buffer[8] = 0; // Avoid overlap with bargraph
				uiUtilityDisplayInformation(buffer, DISPLAY_INFO_SQUELCH, -1);
			}

			// SK1 is pressed, we don't want to clear the first info row after 1s
			if (uiDataGlobal.displayChannelSettings && uiDataGlobal.displaySquelch)
			{
				uiDataGlobal.displaySquelch = false;
			}

			ucRender();
			break;

		case QSO_DISPLAY_CALLER_DATA:
		case QSO_DISPLAY_CALLER_DATA_UPDATE:
			uiDataGlobal.displayQSOStatePrev = QSO_DISPLAY_CALLER_DATA;
			uiDataGlobal.isDisplayingQSOData = true;
			uiDataGlobal.displayChannelSettings = false;
			uiUtilityRenderQSOData();
			ucRender();
			break;

		case QSO_DISPLAY_IDLE:
			break;
	}

	uiDataGlobal.displayQSOState = QSO_DISPLAY_IDLE;
}

#if !defined(PLATFORM_GD77S)
// these indices are relative to the current zone.
static bool ReorderChannels(int zoneChannelIndex1, int zoneChannelIndex2)
{
	if (!codeplugZoneReorderChannels(zoneChannelIndex1, zoneChannelIndex2, &currentZone))
		return false;
	settingsSetCurrentChannelIndexForZone(zoneChannelIndex2, nonVolatileSettings.currentZone);
	currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
	loadChannelData(false, false);
	announceItem(PROMPT_SEQUENCE_CHANNEL_NUMBER_AND_NAME, AUDIO_PROMPT_MODE_VOICE_LEVEL_2);
	return true;
}

static bool swapCurrentWithNext()
{
	if (!reorderingChannels)
		return false;
	if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone <=1)
		return false;

	int next=((nonVolatileSettings.currentChannelIndexInZone + 1) % currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone);
	return ReorderChannels(nonVolatileSettings.currentChannelIndexInZone, next);
}

static bool swapCurrentWithPrior()
{
	if (!reorderingChannels)
		return false;
	if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone <=1)
		return false;

	int prior=nonVolatileSettings.currentChannelIndexInZone ==0 ? currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone-1 : nonVolatileSettings.currentChannelIndexInZone-1;
	return ReorderChannels(nonVolatileSettings.currentChannelIndexInZone, prior);
}

static bool swapCurrentWithLast()
{
	if (!reorderingChannels)
		return false;
	if (nonVolatileSettings.currentChannelIndexInZone >= currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
		return false;
	return ReorderChannels(nonVolatileSettings.currentChannelIndexInZone, currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone-1);
}

static bool swapCurrentWithFirst()
{
	if (!reorderingChannels)
		return false;

	if (nonVolatileSettings.currentChannelIndexInZone ==0)
		return false;
	return ReorderChannels(nonVolatileSettings.currentChannelIndexInZone, 0);
}
#endif // !defined(PLATFORM_GD77S)

static void handleEvent(uiEvent_t *ev)
{
#if defined(PLATFORM_GD77S)
	handleEventForGD77S(ev);
	return;
#else
	if ((uiDataGlobal.Scan.active) && (ev->events & KEY_EVENT) && (uiDataGlobal.Scan.scanType == SCAN_TYPE_NORMAL_STEP))
	{
		// Key pressed during scanning

		if (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0)
		{
			// if we are scanning and down key is pressed then enter current channel into nuisance delete array.
			if((uiDataGlobal.Scan.state == SCAN_PAUSED) && (ev->keys.key == KEY_RIGHT))
			{
				AddFrequencyToNuisanceList(currentChannelData->rxFreq);
				return;
			}

			// Left key reverses the scan direction
			if (ev->keys.key == KEY_LEFT)
			{
				uiDataGlobal.Scan.direction *= -1;
				keyboardReset();
				return;
			}
		}
		// stop the scan on any button except UP without Shift (allows scan to be manually continued)
		// or SK2 on its own (allows Backlight to be triggered)
		if (ev->keys.key != KEY_UP)
		{
			//if scanning is stopped via a key, we will return to the previous channel unless a signal is still being received.
			// If the scan is stopped with green, however, we'll remain on the last channel even without a signal.
			if (ev->keys.key == KEY_GREEN)
			{
				scanStartZone = NO_ZONE;
				scanStartChannel = NO_CHANNEL;
			}
			uiChannelModeStopScanning();
			keyboardReset();
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);
			loadChannelData(false, true);
			return;
		}
	}

	if (ev->events & FUNCTION_EVENT)
	{
		if (ev->function == FUNC_START_SCANNING)
		{
			directChannelNumber = 0;
			StopDualWatch(false); // change to regular scan.
			if (uiDataGlobal.Scan.active == false)
			{
				scanStart(false);
			}
			return;
		}
		else if (ev->function == FUNC_REDRAW)
		{
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);
			return;
		}
	}

	if ((uiDataGlobal.reverseRepeater == false) && handleMonitorMode(ev))
	{
		return;
	}
	if (ev->events & BUTTON_EVENT)
	{
		// long hold sk1 now summarizes channel for all models.
		if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK1) && (monitorModeData.isEnabled == false) && (uiDataGlobal.DTMFContactList.isKeying == false) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0))
		{
						// Add dual watch info
			if (uiDataGlobal.Scan.active && dualWatchChannelData.dualWatchActive)
				AnnounceDualWatchChannels(nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
			else
				AnnounceChannelSummary((nonVolatileSettings.audioPromptMode <= AUDIO_PROMPT_MODE_VOICE_LEVEL_2), true);
			return;
		}

		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}

		uint32_t tg = (LinkHead->talkGroupOrPcId & 0xFFFFFF);

		// If Blue button is pressed during reception it sets the Tx TG to the incoming TG
		if (uiDataGlobal.isDisplayingQSOData && BUTTONCHECK_SHORTUP(ev, BUTTON_SK2) && (trxGetMode() == RADIO_MODE_DIGITAL) &&
				((trxTalkGroupOrPcId != tg) ||
				((dmrMonitorCapturedTS != -1) && (dmrMonitorCapturedTS != trxGetDMRTimeSlot())) ||
				(trxGetDMRColourCode() != currentChannelData->txColor)))
		{
#if !defined(PLATFORM_GD77S)
			sk2Latch =false;
			sk2LatchTimeout=0;
#endif // !defined(PLATFORM_GD77S)
			lastHeardClearLastID();

			// Set TS to overriden TS
			if ((dmrMonitorCapturedTS != -1) && (dmrMonitorCapturedTS != trxGetDMRTimeSlot()))
			{
				trxSetDMRTimeSlot(dmrMonitorCapturedTS);
				tsSetManualOverride(CHANNEL_CHANNEL, (dmrMonitorCapturedTS + 1));
			}

			if (trxTalkGroupOrPcId != tg)
			{
				trxTalkGroupOrPcId = tg;
				settingsSet(nonVolatileSettings.overrideTG, trxTalkGroupOrPcId);
			}

			currentChannelData->txColor = trxGetDMRColourCode();// Set the CC to the current CC, which may have been determined by the CC finding algorithm in C6000.c
			announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC,PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY);

			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);
			soundSetMelody(MELODY_ACK_BEEP);
			return;
		}

		if ((uiDataGlobal.reverseRepeater == false) && (BUTTONCHECK_DOWN(ev, BUTTON_SK1) && BUTTONCHECK_DOWN(ev, BUTTON_SK2)))
		{
			trxSetFrequency(currentChannelData->txFreq, currentChannelData->rxFreq, DMR_MODE_DMO);// Swap Tx and Rx freqs but force DMR Active
			uiDataGlobal.reverseRepeater = true;
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);
			return;
		}
		else if ((uiDataGlobal.reverseRepeater == true) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0))
		{
			trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
			uiDataGlobal.reverseRepeater = false;

			// We are still displaying channel details (SK1 has been released), force to update the screen
			if (uiDataGlobal.displayChannelSettings)
			{
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}

			return;
		}
		// Display channel settings (RX/TX/etc) while SK1 is pressed
		else if ((uiDataGlobal.displayChannelSettings == false) && BUTTONCHECK_DOWN(ev, BUTTON_SK1))
		{
			if (uiDataGlobal.Scan.active == false)
			{
				int prevQSODisp = uiDataGlobal.displayQSOStatePrev;
				uiDataGlobal.displayChannelSettings = true;
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
				uiDataGlobal.displayQSOStatePrev = prevQSODisp;
			}
			return;

		}
		else if ((uiDataGlobal.displayChannelSettings == true) && (BUTTONCHECK_DOWN(ev, BUTTON_SK1) == 0))
		{
			uiDataGlobal.displayChannelSettings = false;
			uiDataGlobal.displayQSOState = uiDataGlobal.displayQSOStatePrev;

			// Maybe QSO State has been overridden, double check if we could now
			// display QSO Data
			if (uiDataGlobal.displayQSOState == QSO_DISPLAY_DEFAULT_SCREEN)
			{
				if (isQSODataAvailableForCurrentTalker())
				{
					uiDataGlobal.displayQSOState = QSO_DISPLAY_CALLER_DATA;
				}
			}

			// Leaving Channel Details disable reverse repeater feature
			if (uiDataGlobal.reverseRepeater)
			{
				trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
				uiDataGlobal.reverseRepeater = false;
			}

			uiChannelModeUpdateScreen(0);
			return;
		}

#if !defined(PLATFORM_RD5R)
		if (BUTTONCHECK_SHORTUP(ev, BUTTON_ORANGE) && (BUTTONCHECK_DOWN(ev, BUTTON_SK1) == 0))
		{
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				announceItem(PROMPT_SEQUENCE_BATTERY, AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
			}
			else
			{
				// Quick Menu
				menuSystemPushNewMenu(UI_CHANNEL_QUICK_MENU);

				// Trick to beep (AudioAssist), since ORANGE button doesn't produce any beep event
				ev->keys.event |= KEY_MOD_UP;
				ev->keys.key = 127;
				menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
				// End Trick
			}

			return;
		}
#endif
	}

	if (ev->events & KEY_EVENT)
	{
		if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
		{
			if (reorderingChannels)
			{
				reorderingChannels=false;
				announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ_AND_MODE, PROMPT_THRESHOLD_2);
				return;
			}
			if (directChannelNumber > 0)
			{
				if(CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
				{
					// All Channels virtual zone
					if (codeplugAllChannelsIndexIsInUse(directChannelNumber))
					{
						settingsSet(nonVolatileSettings.currentChannelIndexInAllZone, (int16_t) directChannelNumber);
						// Save this in the dual watch as the currently selected channel.
						SetDualWatchCurrentChannelIndex(directChannelNumber);
						loadChannelData(false, true);
					}
					else
					{
						soundSetMelody(MELODY_ERROR_BEEP);
					}
				}
				else
				{
					if ((directChannelNumber - 1) < currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
					{
						settingsSetCurrentChannelIndexForZone(directChannelNumber - 1, nonVolatileSettings.currentZone);
						// Save this in the dual watch as the currently selected channel.
						SetDualWatchCurrentChannelIndex(directChannelNumber - 1);
						loadChannelData(false, true);
					}
					else
					{
						soundSetMelody(MELODY_ERROR_BEEP);
					}

				}
				directChannelNumber = 0;
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}
			else if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				StopDualWatch(true); // Ensure dual watch is stopped.

				menuSystemPushNewMenu(MENU_CHANNEL_DETAILS);
			}
			else
			{
				StopDualWatch(true); // Ensure dual watch is stopped.

				menuSystemPushNewMenu(MENU_MAIN_MENU);
			}
			return;
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_HASH))
		{
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				StopDualWatch(true); // Ensure dual watch is stopped.

				// Assignment for SK2 + #
				menuSystemPushNewMenu(MENU_CONTACT_QUICKLIST);
			}
			else
			{
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					StopDualWatch(true); // Ensure dual watch is stopped.

					menuSystemPushNewMenu(MENU_NUMERICAL_ENTRY);
				}
				else // analog, toggle between repeater offset 0, plus and minus.
				{
					CycleRepeaterOffset(&menuChannelExitStatus);
					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
					uiDataGlobal.displayChannelSettings = true;
					repeaterOffsetDisplayTimeout=2000;
					uiChannelModeUpdateScreen(0);
				}
			}
			return;
		}
		else if (KEYCHECK_LONGDOWN(ev->keys, KEY_RED))
		{// Switch between priority channel and last channel selected by user.
			EnsurePriorityChannelIsSet();
			if (uiDataGlobal.priorityChannelIndex != NO_PRIORITY_CHANNEL)
			{
				if (dualWatchChannelData.dualWatchActive)
				{
					StopDualWatch(true); // Ensure dual watch is stopped.
				}
				
				uint16_t currentChannelIndex=CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ?nonVolatileSettings.currentChannelIndexInAllZone :nonVolatileSettings.currentChannelIndexInZone;
				if (!uiDataGlobal.priorityChannelActive && dualWatchChannelData.currentChannelIndex!=currentChannelIndex) // ensure current is saved off so it can be restored.
					dualWatchChannelData.currentChannelIndex=currentChannelIndex;
				uiDataGlobal.priorityChannelActive=!uiDataGlobal.priorityChannelActive;
				if (uiDataGlobal.priorityChannelActive)
				{
					if (AutoZoneIsCurrentZone(currentZone.NOT_IN_CODEPLUGDATA_indexNumber))
					{
						nonVolatileSettings.currentChannelIndexInZone=uiDataGlobal.priorityChannelIndex-1;
					}
					else if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
					{
						nonVolatileSettings.currentChannelIndexInAllZone=uiDataGlobal.priorityChannelIndex;
					}
					else
					{
						uint16_t priorityChannelIndexInZone;
						if (codeplugFindAllChannelsIndexInCurrentZone(uiDataGlobal.priorityChannelIndex, &priorityChannelIndexInZone) && priorityChannelIndexInZone!=NO_PRIORITY_CHANNEL)
							nonVolatileSettings.currentChannelIndexInZone=priorityChannelIndexInZone;
						else // save it to the all channels zone
							nonVolatileSettings.currentChannelIndexInAllZone=uiDataGlobal.priorityChannelIndex;
					}
				}
				else
				{// restore the prior channel from the dualWatchChannelData.currentChannelIndex as it may have been overridden by a screen update.
					if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
						nonVolatileSettings.currentChannelIndexInAllZone=dualWatchChannelData.currentChannelIndex;
					else
						nonVolatileSettings.currentChannelIndexInZone=dualWatchChannelData.currentChannelIndex;
					menuChannelExitStatus |= MENU_STATUS_FORCE_FIRST;
				}
				loadChannelData(uiDataGlobal.priorityChannelActive, true);
			}
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
		{
			if (reorderingChannels)
			{
				reorderingChannels=false;
				announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ_AND_MODE, PROMPT_THRESHOLD_2);
				return;
			}

			if (dualWatchChannelData.dualWatchActive)
			{
				StopDualWatch(true); // Ensure dual watch is stopped.
				announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ_AND_MODE, PROMPT_THRESHOLD_2);
				return;
			}
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2) && (uiDataGlobal.tgBeforePcMode != 0))
			{
				settingsSet(nonVolatileSettings.overrideTG, uiDataGlobal.tgBeforePcMode);
				menuPrivateCallClear();

				updateTrxID();
				uiDataGlobal.displayQSOState= QSO_DISPLAY_DEFAULT_SCREEN;// Force redraw
				uiChannelModeUpdateScreen(0);
				return;// The event has been handled
			}

			if(directChannelNumber > 0)
			{
				announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ, PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY);

				directChannelNumber = 0;
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}
			else
			{
#if defined(PLATFORM_GD77) || defined(PLATFORM_DM1801A)
				menuSystemSetCurrentMenu(UI_VFO_MODE);
#endif
				return;
			}
		}
#if defined(PLATFORM_DM1801) || defined(PLATFORM_RD5R)
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_VFO_MR))
		{
			StopDualWatch(true); // Ensure dual watch is stopped.

			directChannelNumber = 0;
			menuSystemSetCurrentMenu(UI_VFO_MODE);
			return;
		}
#endif
#if defined(PLATFORM_RD5R)
		else if (KEYCHECK_LONGDOWN(ev->keys, KEY_VFO_MR) && (BUTTONCHECK_DOWN(ev, BUTTON_SK1) == 0))
		{
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				announceItem(PROMPT_SEQUENCE_BATTERY, AUDIO_PROMPT_MODE_VOICE_LEVEL_1);
			}
			else
			{
				menuSystemPushNewMenu(UI_CHANNEL_QUICK_MENU);

				// Trick to beep (AudioAssist), since ORANGE button doesn't produce any beep event
				ev->keys.event |= KEY_MOD_UP;
				ev->keys.key = 127;
				menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
				// End Trick
			}

			return;
		}
#endif
		else if (KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT))
		{
			if (reorderingChannels)
			{
				swapCurrentWithLast();
				return;
			}
			// Long press allows the 5W+ power setting to be selected immediately
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2) && AtMaximumPower())
			{
				if (increasePowerLevel(true, false))
				{
					uiUtilityRedrawHeaderOnly(notScanning);
				}
			}
		}
		else if (reorderingChannels && KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT))
		{
			swapCurrentWithNext();
			return;
		}
		else if (KEYCHECK_PRESS(ev->keys, KEY_RIGHT) && !reorderingChannels)
		{
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				if (increasePowerLevel(false, KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_RIGHT)))
				{
					uiUtilityRedrawHeaderOnly(notScanning);
				}
			}
			else
			{
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					if (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup > 1)
					{
						if (nonVolatileSettings.overrideTG == 0)
						{
							settingsIncrement(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 1);
							if (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] > (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup - 1))
							{
								settingsSet(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 0);
								menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
							}
						}
					}
					settingsSet(nonVolatileSettings.overrideTG, 0);// setting the override TG to 0 indicates the TG is not overridden
					menuPrivateCallClear();
					updateTrxID();
					// We're in digital mode, RXing, and current talker is already at the top of last heard list,
					// hence immediately display complete contact/TG info on screen
					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;//(isQSODataAvailableForCurrentTalker() ? QSO_DISPLAY_CALLER_DATA : QSO_DISPLAY_DEFAULT_SCREEN);
					if (isQSODataAvailableForCurrentTalker())
					{
						addTimerCallback(uiUtilityRenderQSODataAndUpdateScreen, 2000, UI_CHANNEL_MODE, true);
					}
					uiChannelModeUpdateScreen(0);
					announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC,PROMPT_THRESHOLD_2);
				}
				else
				{
					if(currentChannelData->sql == 0)			//If we were using default squelch level
					{
						currentChannelData->sql = nonVolatileSettings.squelchDefaults[trxCurrentBand[TRX_RX_FREQ_BAND]];			//start the adjustment from that point.
					}

					if (currentChannelData->sql < CODEPLUG_MAX_VARIABLE_SQUELCH)
					{
						currentChannelData->sql++;
					}

					announceItem(PROMPT_SQUENCE_SQUELCH,PROMPT_THRESHOLD_2);

					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
					uiDataGlobal.displaySquelch = true;
					uiChannelModeUpdateScreen(0);
				}
			}

		}
		else if (KEYCHECK_LONGDOWN(ev->keys, KEY_LEFT) && reorderingChannels)
		{
			swapCurrentWithFirst();
			return;
		}
		else if (reorderingChannels && KEYCHECK_SHORTUP(ev->keys, KEY_LEFT))
		{
			swapCurrentWithPrior();
			return;
		}
		else if (KEYCHECK_PRESS(ev->keys, KEY_LEFT) && !reorderingChannels)
		{
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				if (decreasePowerLevel(KEYCHECK_LONGDOWN(ev->keys, KEY_LEFT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_LEFT)))
				{
					uiUtilityRedrawHeaderOnly(notScanning);
				}

				if (trxGetPowerLevel() == 0)
				{
					menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
				}
			}
			else
			{
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					if (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup > 1)
					{
						// To Do change TG in on same channel freq
						if (nonVolatileSettings.overrideTG == 0)
						{
							settingsDecrement(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 1);
							if (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] < 0)
							{
								settingsSet(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE],
										(int16_t) (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup - 1));
							}

							if (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] == 0)
							{
								menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
							}
						}
					}
					settingsSet(nonVolatileSettings.overrideTG, 0);// setting the override TG to 0 indicates the TG is not overridden
					menuPrivateCallClear();
					updateTrxID();
					// We're in digital mode, RXing, and current talker is already at the top of last heard list,
					// hence immediately display complete contact/TG info on screen
					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;//(isQSODataAvailableForCurrentTalker() ? QSO_DISPLAY_CALLER_DATA : QSO_DISPLAY_DEFAULT_SCREEN);
					if (isQSODataAvailableForCurrentTalker())
					{
						addTimerCallback(uiUtilityRenderQSODataAndUpdateScreen, 2000, UI_CHANNEL_MODE, true);
					}
					uiChannelModeUpdateScreen(0);
					announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC,PROMPT_THRESHOLD_2);
				}
				else
				{
					if(currentChannelData->sql == 0)			//If we were using default squelch level
					{
						currentChannelData->sql = nonVolatileSettings.squelchDefaults[trxCurrentBand[TRX_RX_FREQ_BAND]];			//start the adjustment from that point.
					}

					if (currentChannelData->sql > CODEPLUG_MIN_VARIABLE_SQUELCH)
					{
						currentChannelData->sql--;
					}

					announceItem(PROMPT_SQUENCE_SQUELCH,PROMPT_THRESHOLD_2);

					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
					uiDataGlobal.displaySquelch = true;
					uiChannelModeUpdateScreen(0);
				}

			}
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_STAR))
		{
			if ( ToggleFMBandwidth(ev, currentChannelData))
			{
				announceItem(PROMPT_SEQUENCE_BANDWIDTH, PROMPT_THRESHOLD_2);
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				if (currentChannelData->flag4&0x02)
					menuChannelExitStatus |= MENU_STATUS_FORCE_FIRST;
				uiChannelModeUpdateScreen(0);
			}
			else if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))  // Toggle Channel Mode
			{
				if (trxGetMode() == RADIO_MODE_ANALOG)
				{
					currentChannelData->chMode = RADIO_MODE_DIGITAL;
					uiDataGlobal.VoicePrompts.inhibitInitial = true;// Stop VP playing in loadChannelData
					loadChannelData(true, false);
					uiDataGlobal.VoicePrompts.inhibitInitial = false;
					menuChannelExitStatus |= MENU_STATUS_FORCE_FIRST;
				}
				else
				{
					currentChannelData->chMode = RADIO_MODE_ANALOG;
					trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4 & 0x02) == 0x02));
					trxSetRxCSS(currentChannelData->rxTone);
				}

				announceItem(PROMPT_SEQUENCE_MODE, PROMPT_THRESHOLD_2);
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}
			else
			{
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					// Toggle timeslot
					trxSetDMRTimeSlot(1 - trxGetDMRTimeSlot());
					tsSetManualOverride(CHANNEL_CHANNEL, (trxGetDMRTimeSlot() + 1));

					if ((nonVolatileSettings.overrideTG == 0) && (currentContactData.reserve1 & 0x01) == 0x00)
					{
						tsSetContactHasBeenOverriden(CHANNEL_CHANNEL, true);
					}

					//	init_digital();
					disableAudioAmp(AUDIO_AMP_MODE_RF);
					clearActiveDMRID();
					lastHeardClearLastID();
					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
					uiChannelModeUpdateScreen(0);

					if (trxGetDMRTimeSlot() == 0)
					{
						menuChannelExitStatus |= MENU_STATUS_FORCE_FIRST;
					}
					announceItem(PROMPT_SEQUENCE_TS,PROMPT_THRESHOLD_2);
				}
				else
				{
					if ((currentChannelData->flag4 & 0x02) == 0x02)
					{
						currentChannelData->flag4 &= ~0x02;// clear 25kHz bit
					}
					else
					{
						currentChannelData->flag4 |= 0x02;// set 25kHz bit
						nextKeyBeepMelody = (int *)MELODY_KEY_BEEP_FIRST_ITEM;
					}
					// ToDo announce VP for bandwidth perhaps

					trxSetModeAndBandwidth(RADIO_MODE_ANALOG, ((currentChannelData->flag4 & 0x02) == 0x02));
					soundSetMelody(MELODY_NACK_BEEP);
					headerRowIsDirty = true;
					uiChannelModeUpdateScreen(0);
				}
			}
		}
		else if (KEYCHECK_LONGDOWN(ev->keys, KEY_STAR) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0))
		{
			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				tsSetManualOverride(CHANNEL_CHANNEL, TS_NO_OVERRIDE);
				tsSetContactHasBeenOverriden(CHANNEL_CHANNEL, false);

				if ((currentRxGroupData.name[0] != 0) && (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] < currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup))
				{
					codeplugContactGetDataForIndex(currentRxGroupData.contacts[nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE]], &currentContactData);
				}
				else
				{
					codeplugContactGetDataForIndex(currentChannelData->contact, &currentContactData);
				}

				trxUpdateTsForCurrentChannelWithSpecifiedContact(&currentContactData);

				announceItem(PROMPT_SEQUENCE_TS,PROMPT_THRESHOLD_2);

				clearActiveDMRID();
				lastHeardClearLastID();
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
				announceItem(PROMPT_SEQUENCE_TS, PROMPT_THRESHOLD_1);
			}
			else
			{
				struct_codeplugChannel_t tempChannel;
				if (uiDataGlobal.priorityChannelActive)
					codeplugChannelGetDataForIndex(uiDataGlobal.priorityChannelIndex, &tempChannel);
				else if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
					codeplugChannelGetDataForIndex(nonVolatileSettings.currentChannelIndexInAllZone, &tempChannel);
				else
					codeplugChannelGetDataForIndex(currentZone.channels[nonVolatileSettings.currentChannelIndexInZone], &tempChannel);
				currentChannelData->flag4=tempChannel.flag4;
				trxSetModeAndBandwidth(currentChannelData->chMode, currentChannelData->flag4);
				announceItem(PROMPT_SEQUENCE_BANDWIDTH, PROMPT_THRESHOLD_2);
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_DOWN) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_DOWN))
		{
			uiDataGlobal.displaySquelch = false;
			
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				selectPrevNextZone(false);
				menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, false);
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN; // Force screen redraw

				if (nonVolatileSettings.currentZone == 0)
				{
					menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
				}

				announceItem(PROMPT_SEQUENCE_ZONE_AND_CHANNEL_NAME, PROMPT_THRESHOLD_2);

				return;
			}
			else
			{
				int16_t prevChan;

				lastHeardClearLastID();
				if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
				{
					if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 1)
					{
						prevChan = nonVolatileSettings.currentChannelIndexInAllZone;

						// All Channels virtual zone
						do
						{
							prevChan = ((((prevChan - 1) + currentZone.NOT_IN_CODEPLUGDATA_highestIndex - 1) % currentZone.NOT_IN_CODEPLUGDATA_highestIndex) + 1);

						} while (!codeplugAllChannelsIndexIsInUse(prevChan));

						settingsSet(nonVolatileSettings.currentChannelIndexInAllZone, prevChan);
						// Set this in the Dual Watch struct as the current channel just selected by the user.
						SetDualWatchCurrentChannelIndex(prevChan);
						if (nonVolatileSettings.currentChannelIndexInAllZone == 1)
						{
							menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
						}
					}
				}
				else
				{
					if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 1)
					{
						prevChan = ((nonVolatileSettings.currentChannelIndexInZone + currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone - 1) % currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone);

						settingsSetCurrentChannelIndexForZone(prevChan, nonVolatileSettings.currentZone);
						// Set this in the Dual Watch struct as the current channel just selected by the user.
						SetDualWatchCurrentChannelIndex(prevChan);

						if (nonVolatileSettings.currentChannelIndexInZone == 0)
						{
							menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
						}
					}
				}
			}
			loadChannelData(false, true);
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);
		}
		else if (KEYCHECK_SHORTUP(ev->keys, KEY_UP) || KEYCHECK_LONGDOWN(ev->keys, KEY_UP) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_UP))
		{
			handleUpKey(ev);
			return;
		}
		else
		{
			int keyval = menuGetKeypadKeyValue(ev, true);
			
			if ((keyval < 10) && (!BUTTONCHECK_DOWN(ev, BUTTON_SK2)) && (!BUTTONCHECK_DOWN(ev, BUTTON_SK1)))
			{
				directChannelNumber = (directChannelNumber * 10) + keyval;
				if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
				{
					// All Channels virtual zone
					if(directChannelNumber > CODEPLUG_CONTACTS_MAX)
					{
						directChannelNumber = 0;
						soundSetMelody(MELODY_ERROR_BEEP);
					}
				}
				else
				{
					if(directChannelNumber > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
					{
						directChannelNumber = 0;
						soundSetMelody(MELODY_ERROR_BEEP);
					}
				}

				if (directChannelNumber > 0)
				{
					voicePromptsInit();
					if (directChannelNumber < 10)
					{
						voicePromptsAppendLanguageString(&currentLanguage->gotoChannel);
					}
					voicePromptsAppendPrompt(PROMPT_0 + keyval);
					voicePromptsPlay();
				}
				else
				{
					announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ, PROMPT_THRESHOLD_2);
				}

				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}
		}
	}
#endif // ! PLATFORM_GD77S
}
// This is used by GD77S scan for multizone scan or by GD77 etc for manual zone selection or multizone scan 
static void selectPrevNextZone(bool nextZone)
{
	int numZones = codeplugZonesGetCount();
	if (!uiDataGlobal.Scan.scanAllZones)
		settingsSetCurrentChannelIndexForZone(nonVolatileSettings.currentChannelIndexInZone, nonVolatileSettings.currentZone);
	if (nextZone)
	{
		settingsIncrement(nonVolatileSettings.currentZone, 1);

		if (nonVolatileSettings.currentZone >= numZones)
		{
			settingsSet(nonVolatileSettings.currentZone, 0);
		}
	}
	else
	{
		if (nonVolatileSettings.currentZone == 0)
		{
			settingsSet(nonVolatileSettings.currentZone, (int16_t) (numZones - 1));
		}
		else
		{
			settingsDecrement(nonVolatileSettings.currentZone, 1);
		}
	}

/*
 * VK3KYY
 * I don't understand why these should be reset when changing zones.
 * Only the change to TG override will be clear to the operator.
 *
 * Changing zones should only change the zone.
 * I can't see a compelling reason to override what the operator has setup just because they change zones
 *
	settingsSet(nonVolatileSettings.overrideTG, 0); // remove any TG override

	tsSetManualOverride(CHANNEL_CHANNEL, TS_NO_OVERRIDE);// remove any TS override
*/
	if (!uiDataGlobal.Scan.scanAllZones)
	{
		settingsSet(nonVolatileSettings.currentChannelIndexInZone, settingsGetCurrentChannelIndexForZone(nonVolatileSettings.currentZone));
		currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
	}
	codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
	EnsurePriorityChannelIsSet();
	uiDataGlobal.priorityChannelActive=false;
}

#if ! defined(PLATFORM_GD77S)
static void handleUpKey(uiEvent_t *ev)
{
	uiDataGlobal.displaySquelch = false;
	bool longHoldUp = KEYCHECK_LONGDOWN(ev->keys, KEY_UP) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_UP);
	bool sk2held=BUTTONCHECK_DOWN(ev, BUTTON_SK2);
	if (sk2held || longHoldUp)
	{
		// long hold sk2+up scan all zones.
		if (longHoldUp)
		{
			if (codeplugZonesGetCount() > 1)
				uiDataGlobal.Scan.scanAllZones=sk2held;
			if (!uiDataGlobal.Scan.active)
			{
				StopDualWatch(true); // change to regular scan.
				scanStart(nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_2);
				announceItem(PROMPT_SEQUENCE_SCAN_TYPE, PROMPT_THRESHOLD_2);
			}
			return;
		}
		selectPrevNextZone(true);
		menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, false);
		uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN; // Force screen redraw

		if (nonVolatileSettings.currentZone == 0)
		{
			menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
		}

		announceItem(PROMPT_SEQUENCE_ZONE_AND_CHANNEL_NAME, PROMPT_THRESHOLD_2);

		return;
	}
	else
	{
		int16_t nextChan;

		lastHeardClearLastID();
		if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
		{
			if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 1)
			{
				nextChan = nonVolatileSettings.currentChannelIndexInAllZone;

				// All Channels virtual zone
				do
				{
					nextChan = ((((nextChan - 1) + 1) % currentZone.NOT_IN_CODEPLUGDATA_highestIndex) + 1);

					if (nextChan == CODEPLUG_CONTACTS_MIN)
					{
						menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
					}

				} while (!codeplugAllChannelsIndexIsInUse(nextChan));

				settingsSet(nonVolatileSettings.currentChannelIndexInAllZone, nextChan);
				// Set this in the Dual Watch struct as the current channel just selected by the user.
				SetDualWatchCurrentChannelIndex(nextChan);
			}
		}
		else
		{
			if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 1)
			{
				nextChan = ((nonVolatileSettings.currentChannelIndexInZone + 1) % currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone);

				settingsSetCurrentChannelIndexForZone(nextChan, nonVolatileSettings.currentZone);
				// Set this in the Dual Watch struct as the current channel just selected by the user.
				SetDualWatchCurrentChannelIndex(nextChan);

				if (nonVolatileSettings.currentChannelIndexInZone == 0)
				{
					menuChannelExitStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
				}
			}
		}
		uiDataGlobal.Scan.timer = 500;
		uiDataGlobal.Scan.state = SCAN_SCANNING;
	}

	loadChannelData(false, true);
	uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
	uiChannelModeUpdateScreen(0);
}
#endif // ! PLATFORM_GD77S


// Quick Menu functions

enum CHANNEL_SCREEN_QUICK_MENU_ITEMS {  CH_SCREEN_QUICK_MENU_COPY2VFO = 0, CH_SCREEN_QUICK_MENU_COPY_FROM_VFO,
	CH_SCREEN_QUICK_MENU_FILTER_FM,
	CH_SCREEN_QUICK_MENU_FILTER_DMR,
	CH_SCREEN_QUICK_MENU_FILTER_DMR_CC,
	CH_SCREEN_QUICK_MENU_FILTER_DMR_TS,
	CH_SCREEN_QUICK_MENU_DUAL_SCAN,
	CH_SCREEN_QUICK_MENU_PRIORITY_SCAN,
	CH_SCREEN_QUICK_MENU_DEL_FROM_ZONE,
	CH_SCREEN_QUICK_MENU_DEL_CHANNEL,
	CH_SCREEN_QUICK_MENU_REORDER_CHANNELS,
	NUM_CH_SCREEN_QUICK_MENU_ITEMS };// The last item in the list is used so that we automatically get a total number of items in the list

menuStatus_t uiChannelModeQuickMenu(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		menuDataGlobal.menuOptionsSetQuickkey = 0;
		menuDataGlobal.menuOptionsTimeout = 0;

		if (quickmenuChannelFromVFOHandled)
		{
			quickmenuChannelFromVFOHandled = false;
			menuSystemPopAllAndDisplayRootMenu();
			return MENU_STATUS_SUCCESS;
		}
		if (quickmenuDeleteChannelHandled)
		{
			quickmenuDeleteChannelHandled = false;
			quickmenuDeleteFromAllZones=false;
			menuSystemPopAllAndDisplayRootMenu();
			return MENU_STATUS_SUCCESS;
		}
		
		uiChannelModeStopScanning();
		uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel = nonVolatileSettings.dmrDestinationFilter;
		uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel = nonVolatileSettings.dmrCcTsFilter;
		uiDataGlobal.QuickMenu.tmpAnalogFilterLevel = nonVolatileSettings.analogFilterLevel;
		menuDataGlobal.endIndex = NUM_CH_SCREEN_QUICK_MENU_ITEMS;

		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->quick_menu);
		voicePromptsAppendPrompt(PROMPT_SILENCE);
		updateQuickMenuScreen(true);
		return (MENU_STATUS_LIST_TYPE | MENU_STATUS_SUCCESS);
	}
	else
	{
		menuQuickChannelExitStatus = MENU_STATUS_SUCCESS;

		if (ev->hasEvent || (menuDataGlobal.menuOptionsTimeout > 0))
		{
			handleQuickMenuEvent(ev);
		}
	}

	return menuQuickChannelExitStatus;
}

static bool validateOverwriteChannel(void)
{
	quickmenuChannelFromVFOHandled = true;

	if (uiDataGlobal.MessageBox.keyPressed == KEY_GREEN)
	{
		struct_codeplugContact_t vfoContact;
		int8_t vfoTS = -1;

		uiVFOLoadContact(&vfoContact);
		memcpy(&currentChannelData->rxFreq, &settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16);// Don't copy the name of the vfo, which are in the first 16 bytes
		// Find out which TS was in use.
		if ((nonVolatileSettings.overrideTG == 0) && (vfoContact.reserve1 & 0x01) == 0x00)
		{
			vfoTS = ((vfoContact.reserve1 & 0x02) != 0);
		}
		else
		{
			vfoTS = tsGetManualOverride((Channel_t)nonVolatileSettings.currentVFONumber);

			// No Override, use the TS from the Channel
			if (vfoTS == 0)
			{
				vfoTS = ((settingsVFOChannel[nonVolatileSettings.currentVFONumber].flag2 & 0x40) != 0);
			}
			else
			{
				vfoTS--; // convert to real TS
			}
		}

		if (vfoTS == 0)
		{
			currentChannelData->flag2 &= 0xBF;
		}
		else
		{
			currentChannelData->flag2 |= 0x40;
		}

		codeplugChannelSaveDataForIndex(uiDataGlobal.currentSelectedChannelNumber, currentChannelData);
	}

	return true;
}

static bool validateDeleteChannel(void)
{
	quickmenuDeleteChannelHandled = true;

	if (uiDataGlobal.MessageBox.keyPressed == KEY_GREEN)
	{
		uint16_t index = CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : currentZone.channels[nonVolatileSettings.currentChannelIndexInZone];

		if (quickmenuDeleteFromAllZones)
		{
			codeplugDeleteChannelWithIndex(index);
		}
		codeplugZoneDeleteChannelFromZone(index, &currentZone);
		currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
		if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
			nonVolatileSettings.currentChannelIndexInAllZone=SAFE_MIN(1,nonVolatileSettings.currentChannelIndexInAllZone-1);
		else
			nonVolatileSettings.currentChannelIndexInZone=SAFE_MIN(0,nonVolatileSettings.currentChannelIndexInZone-1);
	}
	return true;
}

static void updateQuickMenuScreen(bool isFirstRun)
{
	int mNum = 0;
	char buf[SCREEN_LINE_BUFFER_SIZE];
	char * const *leftSide;// initialise to please the compiler
	char * const *rightSideConst;// initialise to please the compiler
	char rightSideVar[SCREEN_LINE_BUFFER_SIZE];

	ucClearBuf();
	bool settingOption = uiShowQuickKeysChoices(buf, SCREEN_LINE_BUFFER_SIZE,currentLanguage->quick_menu);

	for(int i =- 1; i <= 1; i++)
	{
		if ((settingOption == false) || (i == 0))
		{
			mNum = menuGetMenuOffset(NUM_CH_SCREEN_QUICK_MENU_ITEMS, i);
			buf[0] = 0;
			rightSideVar[0] = 0;
			rightSideConst = NULL;
			leftSide = NULL;

			switch(mNum)
			{
				case CH_SCREEN_QUICK_MENU_COPY2VFO:
					rightSideConst = (char * const *)&currentLanguage->channelToVfo;
					break;
				case CH_SCREEN_QUICK_MENU_COPY_FROM_VFO:
					rightSideConst = (char * const *)&currentLanguage->vfoToChannel;
					break;
				case CH_SCREEN_QUICK_MENU_FILTER_FM:
					leftSide = (char * const *)&currentLanguage->filter;
					if (uiDataGlobal.QuickMenu.tmpAnalogFilterLevel == 0)
					{
						rightSideConst = (char * const *)&currentLanguage->none;
					}
					else
					{
						snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%s", ANALOG_FILTER_LEVELS[uiDataGlobal.QuickMenu.tmpAnalogFilterLevel - 1]);
					}
					break;
				case CH_SCREEN_QUICK_MENU_FILTER_DMR:
					leftSide = (char * const *)&currentLanguage->dmr_filter;
					if (uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel == 0)
					{
						rightSideConst = (char * const *)&currentLanguage->none;
					}
					else
					{
						snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%s", DMR_DESTINATION_FILTER_LEVELS[uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel - 1]);
					}
					break;
				case CH_SCREEN_QUICK_MENU_FILTER_DMR_CC:
					leftSide = (char * const *)&currentLanguage->dmr_cc_filter;
					rightSideConst = (uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel & DMR_CC_FILTER_PATTERN)?(char * const *)&currentLanguage->on:(char * const *)&currentLanguage->off;
					break;
				case CH_SCREEN_QUICK_MENU_FILTER_DMR_TS:
					leftSide = (char * const *)&currentLanguage->dmr_ts_filter;
					rightSideConst = (uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel & DMR_TS_FILTER_PATTERN)?(char * const *)&currentLanguage->on:(char * const *)&currentLanguage->off;
					break;
				case CH_SCREEN_QUICK_MENU_DUAL_SCAN:
					rightSideConst = (char * const *)&currentLanguage->dual_watch;
					break;
				case CH_SCREEN_QUICK_MENU_PRIORITY_SCAN:
					rightSideConst = (char * const *)&currentLanguage->priorityScan;
					break;
				case CH_SCREEN_QUICK_MENU_DEL_FROM_ZONE:
					rightSideConst = (char * const *)&currentLanguage->delete_from_zone;
					break;
				case CH_SCREEN_QUICK_MENU_DEL_CHANNEL:
					rightSideConst = (char * const *)&currentLanguage->delete_from_all_zones;
					break;
				case CH_SCREEN_QUICK_MENU_REORDER_CHANNELS:
					rightSideConst = (char * const *)&currentLanguage->reorder_channels;
					break;
				default:
					buf[0] = 0;
			}

			if (leftSide != NULL)
			{
				snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%s:%s", *leftSide, (rightSideVar[0] ? rightSideVar : *rightSideConst));
			}
			else
			{
				snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%s", (rightSideVar[0] ? rightSideVar : *rightSideConst));
			}

			if (i == 0)
			{
				if (!isFirstRun && (menuDataGlobal.menuOptionsSetQuickkey == 0))
				{
					voicePromptsInit();
				}

				if (leftSide != NULL)
				{
					voicePromptsAppendLanguageString((const char * const *)leftSide);
				}

				if (rightSideVar[0] != 0)
				{
					voicePromptsAppendString(rightSideVar);
				}
				else
				{
					voicePromptsAppendLanguageString((const char * const *)rightSideConst);
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

			if (menuDataGlobal.menuOptionsTimeout > 0)
			{
				menuDisplaySettingOption(*leftSide, (rightSideVar[0] ? rightSideVar : *rightSideConst));
			}
			else
			{
				menuDisplayEntry(i, mNum, buf);
			}
		}
	}

	ucRender();
}

static void handleQuickMenuEvent(uiEvent_t *ev)
{
	bool isDirty = false;

	if ((menuDataGlobal.menuOptionsTimeout > 0) && (!BUTTONCHECK_DOWN(ev, BUTTON_SK2)))
	{
		menuDataGlobal.menuOptionsTimeout--;
		if (menuDataGlobal.menuOptionsTimeout == 0)
		{
			menuSystemPopPreviousMenu();
			return;
		}
	}

	if (ev->events & FUNCTION_EVENT)
	{
		if ((QUICKKEY_TYPE(ev->function) == QUICKKEY_MENU) && (QUICKKEY_ENTRYID(ev->function) < NUM_CH_SCREEN_QUICK_MENU_ITEMS))
		{
			menuDataGlobal.currentItemIndex = QUICKKEY_ENTRYID(ev->function);
			menuQuickChannelExitStatus |= MENU_STATUS_LIST_TYPE;
			isDirty = true;
		}

		if ((QUICKKEY_FUNCTIONID(ev->function) != 0))
		{
			menuDataGlobal.menuOptionsTimeout = 1000;
		}
	}

	if (ev->events & BUTTON_EVENT)
	{
		if (repeatVoicePromptOnSK1(ev))
		{
			return;
		}
	}

	if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
	{
		if (menuDataGlobal.menuOptionsSetQuickkey != 0)
		{
			menuDataGlobal.menuOptionsSetQuickkey = 0;
			menuDataGlobal.menuOptionsTimeout = 0;
			menuQuickChannelExitStatus |= MENU_STATUS_ERROR;
			menuSystemPopPreviousMenu();

			return;
		}

		uiChannelModeStopScanning();
		menuSystemPopPreviousMenu();
		return;
	}
	else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
	{
		if ((menuDataGlobal.menuOptionsSetQuickkey != 0) && (menuDataGlobal.menuOptionsTimeout == 0))
		{
			saveQuickkeyMenuIndex(menuDataGlobal.menuOptionsSetQuickkey, menuSystemGetCurrentMenuNumber(), menuDataGlobal.currentItemIndex, 0);
			menuDataGlobal.menuOptionsSetQuickkey = 0;
			updateQuickMenuScreen(false);

			return;
		}

		switch(menuDataGlobal.currentItemIndex)
		{
			case CH_SCREEN_QUICK_MENU_COPY2VFO:
				{
					int8_t currentTS = trxGetDMRTimeSlot();

					memcpy(&settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, &currentChannelData->rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16);// Don't copy the name of channel, which are in the first 16 bytes
					settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxTone = currentChannelData->rxTone;
					settingsVFOChannel[nonVolatileSettings.currentVFONumber].txTone = currentChannelData->txTone;

					if (nonVolatileSettings.overrideTG == 0)
					{
						nonVolatileSettings.overrideTG = trxTalkGroupOrPcId;
					}

					// Set TS override
					tsSetManualOverride(((Channel_t)nonVolatileSettings.currentVFONumber), currentTS + 1);

					if (currentTS == 0)
					{
						settingsVFOChannel[nonVolatileSettings.currentVFONumber].flag2 &= 0xBF;
					}
					else
					{
						settingsVFOChannel[nonVolatileSettings.currentVFONumber].flag2 |= 0x40;
					}

					menuSystemPopAllAndDisplaySpecificRootMenu(UI_VFO_MODE, true);
				}
				break;
			case CH_SCREEN_QUICK_MENU_COPY_FROM_VFO:
				if (quickmenuChannelFromVFOHandled == false)
				{
					snprintf(uiDataGlobal.MessageBox.message, MESSAGEBOX_MESSAGE_LEN_MAX, "%s\n%s", currentLanguage->overwrite_qm, currentLanguage->please_confirm);
					uiDataGlobal.MessageBox.type = MESSAGEBOX_TYPE_INFO;
					uiDataGlobal.MessageBox.decoration = MESSAGEBOX_DECORATION_FRAME;
					uiDataGlobal.MessageBox.buttons = MESSAGEBOX_BUTTONS_YESNO;
					uiDataGlobal.MessageBox.validatorCallback = validateOverwriteChannel;

					menuSystemPushNewMenu(UI_MESSAGE_BOX);
					voicePromptsInit();
					voicePromptsAppendLanguageString(&currentLanguage->overwrite_qm);
					voicePromptsAppendLanguageString(&currentLanguage->please_confirm);
					voicePromptsPlay();
				}
				return;
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_FM:
				settingsSet(nonVolatileSettings.analogFilterLevel, (uint8_t) uiDataGlobal.QuickMenu.tmpAnalogFilterLevel);
				trxSetAnalogFilterLevel(nonVolatileSettings.analogFilterLevel);
				menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR:
				settingsSet(nonVolatileSettings.dmrDestinationFilter, (uint8_t) uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel);
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					init_digital_DMR_RX();
					disableAudioAmp(AUDIO_AMP_MODE_RF);
				}
				menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR_CC:
			case CH_SCREEN_QUICK_MENU_FILTER_DMR_TS:
				settingsSet(nonVolatileSettings.dmrCcTsFilter, (uint8_t) uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel);
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					init_digital_DMR_RX();
					disableAudioAmp(AUDIO_AMP_MODE_RF);
				}
				menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
				break;
			case CH_SCREEN_QUICK_MENU_DUAL_SCAN:
			{
				uint16_t currentChannelIndexRelativeToCurrentZone= CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : nonVolatileSettings.currentChannelIndexInZone;
				uint16_t currentChannelIndexRelativeToAllChannels=currentZone.channels[nonVolatileSettings.currentChannelIndexInZone];
				// The watch channel must be relative to all Channels zone, the current channel is relevant to the current zone.
				StartDualWatch(currentChannelIndexRelativeToAllChannels, currentChannelIndexRelativeToCurrentZone, 1000);
				menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
				break;
			}
			case CH_SCREEN_QUICK_MENU_PRIORITY_SCAN:
			{
				uint16_t priorityChannelIndex= uiDataGlobal.priorityChannelIndex;

				if (priorityChannelIndex != NO_PRIORITY_CHANNEL)
				{
					uint16_t currentChannelIndex= CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : nonVolatileSettings.currentChannelIndexInZone;
					StartDualWatch(priorityChannelIndex, currentChannelIndex, 1000);
				}
				else
				{
					soundSetMelody(MELODY_ERROR_BEEP);
					menuQuickChannelExitStatus |= MENU_STATUS_ERROR;
				}
				menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
				break;
			}
			case CH_SCREEN_QUICK_MENU_DEL_FROM_ZONE:
			case CH_SCREEN_QUICK_MENU_DEL_CHANNEL:
			{
				quickmenuDeleteFromAllZones = (menuDataGlobal.currentItemIndex == CH_SCREEN_QUICK_MENU_DEL_CHANNEL);
				if (quickmenuDeleteChannelHandled == false)
				{
					if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) && !quickmenuDeleteFromAllZones)
					{
						menuQuickChannelExitStatus |= MENU_STATUS_ERROR;
						break; // can't delete from allChannels zone.
					}
					if (quickmenuDeleteFromAllZones)
						snprintf(uiDataGlobal.MessageBox.message, MESSAGEBOX_MESSAGE_LEN_MAX, "%s\n%s", currentLanguage->delete_from_all_zones, currentLanguage->please_confirm);
					else
						snprintf(uiDataGlobal.MessageBox.message, MESSAGEBOX_MESSAGE_LEN_MAX, "%s\n%s", currentLanguage->delete_from_zone, currentLanguage->please_confirm);
					uiDataGlobal.MessageBox.type = MESSAGEBOX_TYPE_INFO;
					uiDataGlobal.MessageBox.decoration = MESSAGEBOX_DECORATION_FRAME;
					uiDataGlobal.MessageBox.buttons = MESSAGEBOX_BUTTONS_YESNO;
					uiDataGlobal.MessageBox.validatorCallback = validateDeleteChannel;

					menuSystemPushNewMenu(UI_MESSAGE_BOX);
					voicePromptsInit();
					if (quickmenuDeleteFromAllZones)
						voicePromptsAppendLanguageString(&currentLanguage->delete_from_all_zones);
					else
						voicePromptsAppendLanguageString(&currentLanguage->delete_from_zone);
					voicePromptsAppendLanguageString(&currentLanguage->please_confirm);
					voicePromptsPlay();
					break;
				}
			}
			case CH_SCREEN_QUICK_MENU_REORDER_CHANNELS:
			reorderingChannels=true;
			menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
			break;
		}
		return;
	}
	else if (KEYCHECK_PRESS(ev->keys, KEY_RIGHT))
	{
		isDirty = true;
		switch(menuDataGlobal.currentItemIndex)
		{
			case CH_SCREEN_QUICK_MENU_FILTER_FM:
				if (uiDataGlobal.QuickMenu.tmpAnalogFilterLevel < NUM_ANALOG_FILTER_LEVELS - 1)
				{
					uiDataGlobal.QuickMenu.tmpAnalogFilterLevel++;
				}
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR:
				if (uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel < NUM_DMR_DESTINATION_FILTER_LEVELS - 1)
				{
					uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel++;
				}
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR_CC:
				if (!(uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel & DMR_CC_FILTER_PATTERN))
				{
					uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel |= DMR_CC_FILTER_PATTERN;
				}
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR_TS:
				if (!(uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel & DMR_TS_FILTER_PATTERN))
				{
					uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel |= DMR_TS_FILTER_PATTERN;
				}
				break;

		}
	}
	else
	{
		if (KEYCHECK_PRESS(ev->keys, KEY_LEFT))
		{
			isDirty = true;
			switch(menuDataGlobal.currentItemIndex)
			{
			case CH_SCREEN_QUICK_MENU_FILTER_FM:
				if (uiDataGlobal.QuickMenu.tmpAnalogFilterLevel > ANALOG_FILTER_NONE)
				{
					uiDataGlobal.QuickMenu.tmpAnalogFilterLevel--;
				}
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR:
				if (uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel > DMR_DESTINATION_FILTER_NONE)
				{
					uiDataGlobal.QuickMenu.tmpDmrDestinationFilterLevel--;
				}
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR_CC:
				if ((uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel & DMR_CC_FILTER_PATTERN))
				{
					uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel &= ~DMR_CC_FILTER_PATTERN;
				}
				break;
			case CH_SCREEN_QUICK_MENU_FILTER_DMR_TS:
				if ((uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel & DMR_TS_FILTER_PATTERN))
				{
					uiDataGlobal.QuickMenu.tmpDmrCcTsFilterLevel &= ~DMR_TS_FILTER_PATTERN;
				}
				break;
			}
		}
		else
		{
			if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
			{
				isDirty = true;
				menuSystemMenuIncrement(&menuDataGlobal.currentItemIndex, NUM_CH_SCREEN_QUICK_MENU_ITEMS);
				menuQuickChannelExitStatus |= MENU_STATUS_LIST_TYPE;
			}
			else
			{
				if (KEYCHECK_PRESS(ev->keys, KEY_UP))
				{
					isDirty = true;
					menuSystemMenuDecrement(&menuDataGlobal.currentItemIndex, NUM_CH_SCREEN_QUICK_MENU_ITEMS);
					menuQuickChannelExitStatus |= MENU_STATUS_LIST_TYPE;
				}
				else
				{
					if (KEYCHECK_SHORTUP_NUMBER(ev->keys)  && (BUTTONCHECK_DOWN(ev, BUTTON_SK2)))
					{
						menuDataGlobal.menuOptionsSetQuickkey = ev->keys.key;
						isDirty = true;
					}
				}
			}
		}
	}

	if (isDirty)
	{
		updateQuickMenuScreen(false);
	}
}

//Scan Mode
static void scanStart(bool longPressBeep)
{
	// At least two channels are needed to run a scan process.
	if (canCurrentZoneBeScanned(&uiDataGlobal.Scan.availableChannelsCount) == false)
	{
		menuChannelExitStatus |= MENU_STATUS_ERROR;
		return;
	}
	scanZoneIndexApplied = nonVolatileSettings.currentZone;
	directChannelNumber = 0;
	uiDataGlobal.Scan.direction = 1;

	// Clear all nuisance delete channels at start of scanning
	for (int i = 0; i < MAX_ZONE_SCAN_NUISANCE_CHANNELS; i++)
	{
		uiDataGlobal.Scan.nuisanceDelete[i] = -1;
	}
	uiDataGlobal.Scan.nuisanceDeleteIndex = 0;

	uiDataGlobal.Scan.active = true;
	uiDataGlobal.Scan.state = SCAN_SCANNING;
	uiDataGlobal.Scan.lastIteration = false;

	uiDataGlobal.Scan.stepTimeMilliseconds = settingsGetScanStepTimeMilliseconds();

	if ((trxGetMode() == RADIO_MODE_DIGITAL) && (trxDMRModeRx != DMR_MODE_SFR) && (uiDataGlobal.Scan.stepTimeMilliseconds < SCAN_DMR_SIMPLEX_MIN_INTERVAL))
	{
		uiDataGlobal.Scan.timerReload = SCAN_DMR_SIMPLEX_MIN_INTERVAL;
	}
	else
	{
		uiDataGlobal.Scan.timerReload = uiDataGlobal.Scan.stepTimeMilliseconds;
	}
	uiDataGlobal.Scan.timer = uiDataGlobal.Scan.timerReload;
	uiDataGlobal.Scan.scanType = SCAN_TYPE_NORMAL_STEP;

	// Need to set the melody here, otherwise long press will remain silent
	// since beeps aren't allowed while scanning
	if (longPressBeep)
	{
		soundSetMelody(MELODY_KEY_LONG_BEEP);
	}
	scanStartZone=nonVolatileSettings.currentZone;
	// Set current channel index
	if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
	{
		nextChannelIndex = nonVolatileSettings.currentChannelIndexInAllZone;
		scanStartChannel=nonVolatileSettings.currentChannelIndexInAllZone;
	}
	else
	{
		nextChannelIndex = currentZone.channels[nonVolatileSettings.currentChannelIndexInZone];
		scanStartChannel=nonVolatileSettings.currentChannelIndexInZone;
	}

	nextChannelReady = false;
}

static void updateTrxID(void)
{
	if (nonVolatileSettings.overrideTG != 0)
	{
		trxTalkGroupOrPcId = nonVolatileSettings.overrideTG;
	}
	else
	{
		/*
		 * VK3KYY
		 * I can't see a compelling reason to remove the TS override when changing TG
		 * The function trxUpdateTsForCurrentChannelWithSpecifiedContact(), used below, is used to handle the TS
		 *
			tsSetManualOverride(CHANNEL_CHANNEL, TS_NO_OVERRIDE);
		*/
		if ((currentRxGroupData.name[0] != 0) && (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] < currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup))
		{
			codeplugContactGetDataForIndex(currentRxGroupData.contacts[nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE]], &currentContactData);
		}
		else
		{
			codeplugContactGetDataForIndex(currentChannelData->contact, &currentContactData);
		}

		tsSetContactHasBeenOverriden(CHANNEL_CHANNEL, false);

		trxUpdateTsForCurrentChannelWithSpecifiedContact(&currentContactData);
		trxTalkGroupOrPcId = currentContactData.tgNumber;

		if (currentContactData.callType == CONTACT_CALLTYPE_PC)
		{
			trxTalkGroupOrPcId |= (PC_CALL_FLAG << 24);
		}
	}
	lastHeardClearLastID();
	menuPrivateCallClear();
}

/*
When scanning, there are two channel buffers, the current channel on which the radio is listening, and the next channel which is ready to be scanned.
With zones however, we only use the currentZone which means that the currentZone may already be synchronized to the next channel rather than the current channel on which the signal is received.
This means that if you hit long hold SK1 when a signal is being received, or stop the scan, the zone may be out of sync.
To use two zone buffers would waste a lot of memory so instead we keep track of the zoneIndex at the time the scan channel was applied.
When the scan stops or is paused, we restore the zone in the event it is synchronized with the next channel rather than the current.
*/
static void EnsureScanZoneIsSetToSameAsAppliedChannel()
{
	if (scanZoneIndexApplied == nonVolatileSettings.currentZone) 
		return;
	// currentZone has already been updated to point to the next scan channel, bring it back to the one containing the received channel.
	settingsSet(nonVolatileSettings.currentZone, scanZoneIndexApplied);
	codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
}

static void scanning(void)
{
	if (voicePromptsIsPlaying()) return;
	// If Dual Watch is active, only hold and pause are relevant since we want to continue watching both channels until cancelled.
	int scanMode=nonVolatileSettings.scanModePause;
	if (dualWatchChannelData.dualWatchActive && scanMode==SCAN_MODE_STOP )
		scanMode=SCAN_MODE_HOLD;

	// After initial settling time
	if((uiDataGlobal.Scan.state == SCAN_SCANNING) && (uiDataGlobal.Scan.timer > SCAN_SKIP_CHANNEL_INTERVAL) && (uiDataGlobal.Scan.timer < (uiDataGlobal.Scan.timerReload - SCAN_FREQ_CHANGE_SETTLING_INTERVAL)))
	{
		//test for presence of RF Carrier.
		// In FM mode the DMR slot_state will always be DMR_STATE_IDLE
		if ((slot_state != DMR_STATE_IDLE) && ((dmrMonitorCapturedTS != -1) &&
				(((trxDMRModeRx == DMR_MODE_DMO) && (dmrMonitorCapturedTS == trxGetDMRTimeSlot())) || trxDMRModeRx == DMR_MODE_RMO)))
		{
			uiDataGlobal.Scan.state = SCAN_PAUSED;
			EnsureScanZoneIsSetToSameAsAppliedChannel();

#if ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own
			// Reload the channel as voice prompts aren't set while scanning
			if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
			{
				loadChannelData(true, true);
			}
#endif
			if (scanMode == SCAN_MODE_STOP)
			{
				uiChannelModeStopScanning();
				return;
			}
			else
			{
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN; // Force screen refresh

				uiDataGlobal.Scan.timer = nonVolatileSettings.scanDelay * 1000;
			}
		}
		else
		{
			if(trxCarrierDetected())
			{
				uiDataGlobal.Scan.state = SCAN_SHORT_PAUSED;		//state 1 = pause and test for valid signal that produces audio
				EnsureScanZoneIsSetToSameAsAppliedChannel();

#if ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own
				// Reload the channel as voice prompts aren't set while scanning

				if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
				{
					loadChannelData(true, true);
				}
#endif
				if (scanMode == SCAN_MODE_STOP)
				{
					uiChannelModeStopScanning();
					return;
				}
				else
				{
					uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN; // Force screen refresh

					uiDataGlobal.Scan.timer = SCAN_SHORT_PAUSE_TIME;	//start short delay to allow full detection of signal
				}

			}
		}
	}

	// Only do this once if scan mode is PAUSE do it every time if scan mode is HOLD
	if(((uiDataGlobal.Scan.state == SCAN_PAUSED) && (scanMode == SCAN_MODE_HOLD)) || (uiDataGlobal.Scan.state == SCAN_SHORT_PAUSED))
	{
	    if (getAudioAmpStatus() & AUDIO_AMP_MODE_RF)
	    {
	    	uiDataGlobal.Scan.timer = nonVolatileSettings.scanDelay * 1000;
	    	uiDataGlobal.Scan.state = SCAN_PAUSED;
	    }
	}

	if(uiDataGlobal.Scan.timer > 0)
	{
		if (nextChannelReady == false)
		{
			scanSearchForNextChannel();
		}

		uiDataGlobal.Scan.timer--;
	}
	else
	{
		if (nextChannelReady)
		{
			scanApplyNextChannel();

			// When less than 2 channel remain in the Zone
			if (uiDataGlobal.Scan.lastIteration)
			{
				uiChannelModeStopScanning();
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
				return;
			}
		}
		else if (dualWatchChannelData.dualWatchActive) // reset the timer, it has expired.
			uiDataGlobal.Scan.timer =uiDataGlobal.Scan.timerReload; // force scan to continue;

		uiDataGlobal.Scan.state = SCAN_SCANNING;													//state 0 = settling and test for carrier present.
	}
}

void uiChannelModeStopScanning(void)
{
	uiDataGlobal.Scan.scanAllZones=false;
	// If these are still set, return to them.
	// Nothing was found during the scan.
	if (uiDataGlobal.Scan.state == SCAN_SCANNING)
	{
		if ((scanStartZone!=NO_ZONE) && (scanStartZone !=nonVolatileSettings.currentZone))
		{
			settingsSet(nonVolatileSettings.currentZone, scanStartZone);
			currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
	
			codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
		}
		if (scanStartChannel!=NO_CHANNEL)
		{
			if (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
				settingsSet(nonVolatileSettings.currentChannelIndexInAllZone, (int16_t) scanStartChannel);
			else
				settingsSetCurrentChannelIndexForZone((int16_t) scanStartChannel, nonVolatileSettings.currentZone);
		}
	}
	scanStartZone = NO_ZONE;
	scanStartChannel = NO_CHANNEL;
	uiDataGlobal.Scan.active = false;
	if (dualWatchChannelData.dualWatchActive)
	{
		dualWatchChannelData.dualWatchActive=false;
		dualWatchChannelData.restartDualWatch=true;
		dualWatchChannelData.dualWatchPauseCountdownTimer=nonVolatileSettings.scanDelay * 1000; // give time after tx finishes before restarting dual watch.
	}
	uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN; // Force screen refresh

#if ! defined(PLATFORM_GD77S) // GD77S handle voice prompts on its own
	// Reload the channel as voice prompts aren't set while scanning
	if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		loadChannelData(false, true);
	}
#endif
}

bool uiChannelModeIsScanning(void)
{
	return uiDataGlobal.Scan.active;
}

void uiChannelModeColdStart(void)
{
	currentChannelData->rxFreq = 0;	// Force to re-read codeplug data (needed due to "All Channels" translation)
	InitDualWatchData();
}

// This can also be called from the VFO, on VFO -> New Channel, as currentZone could be uninitialized.
void uiChannelInitializeCurrentZone(void)
{
	codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
	codeplugUtilConvertBufToString(currentZone.name, currentZoneName, 16);// need to convert to zero terminated string
	settingsSet(nonVolatileSettings.currentChannelIndexInZone, settingsGetCurrentChannelIndexForZone(nonVolatileSettings.currentZone));
	// Ensure stored channel number is within range.
	if (nonVolatileSettings.currentChannelIndexInZone >= currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
		settingsSetCurrentChannelIndexForZone(0, nonVolatileSettings.currentZone);
	EnsurePriorityChannelIsSet();
}

#if defined(PLATFORM_GD77S)
#define GD77S_KEYPAD_BUF_MAX 33
static int GD77sSelectedCharIndex=0;
static int GD77sKeypadBankIndex=0;
static char GD77SKeypadBuffer[GD77S_KEYPAD_BUF_MAX]="\0";
static int GD77SKeypadPos=0;
// bank 0 is DTMF symbols.
#define GD77S_KEYPAD_BANK1_CHARS "ABCDEFGHIJKLMNOP"
#define GD77S_KEYPAD_BANK2_CHARS "QRSTUVWXYZ+-%*# "

static int GetDTMFContactIndexForZoneAndChannelAutoDial(bool checkIfAlreadyDialled)
{
	if (GD77SParameters.dtmfListCount == 0)
		return 0;
	if (nonVolatileSettings.currentChannelIndexInZone > 15) return 0; // can only handle 16 channels (0 to 15)
	if (nonVolatileSettings.currentZone > 15) return 0; // can only handle 16 zones (0 to 15)
	if (trxGetMode() == RADIO_MODE_DIGITAL) return 0; // DTMF dialing disallowed for digital channels.
	// See if we've already dialled this combination 	
	if (checkIfAlreadyDialled && ((GD77SParameters.dialedZones&(1<<nonVolatileSettings.currentZone)) && (GD77SParameters.dialedChannels&(1<<nonVolatileSettings.currentChannelIndexInZone))))
		return 0;
	
	char autoDialContactForZoneAndChannel[7];
	snprintf(autoDialContactForZoneAndChannel, 7, "AD%02d%02d", nonVolatileSettings.currentZone+1, nonVolatileSettings.currentChannelIndexInZone + 1);
	
	int index = codeplugGetDTMFContactIndex(autoDialContactForZoneAndChannel);
	if (index > 0)
		return index;
	// try again without zone 
	snprintf(autoDialContactForZoneAndChannel, 7, "AD00%02d", nonVolatileSettings.currentChannelIndexInZone + 1);
	return codeplugGetDTMFContactIndex(autoDialContactForZoneAndChannel);
}

static void ToggleGD77SDTMFAutoDialer(bool announce)
{
	if (GetDTMFContactIndexForZoneAndChannelAutoDial(false)==0)
		return;
	
	bool alreadyDialed = (GD77SParameters.dialedZones&(1<<nonVolatileSettings.currentZone)) && (GD77SParameters.dialedChannels&(1<<nonVolatileSettings.currentChannelIndexInZone));
	// Only toggle the channel bit as there may be more than one channel in this zone with a dtmf contact associated with it.
	if (alreadyDialed)
	{
		GD77SParameters.dialedChannels&=~(1<<nonVolatileSettings.currentChannelIndexInZone);
	}
	else
	{
		GD77SParameters.dialedChannels|=(1<<nonVolatileSettings.currentChannelIndexInZone);
		GD77SParameters.dialedZones|=(1<<nonVolatileSettings.currentZone);
	}
	if (!announce)
		return;
	alreadyDialed=!alreadyDialed; // toggled above
	voicePromptsInit();
	
	if (!voicePromptsIsPlaying())
	{
		voicePromptsAppendLanguageString(&currentLanguage->Auto);
		voicePromptsAppendLanguageString(&currentLanguage->contact);
	}
	voicePromptsAppendLanguageString(alreadyDialed ? &currentLanguage->off : &currentLanguage->on);
	
	voicePromptsPlay();
}

static void DisableAutoDialerForCurrentChannelAndZone()
{
	if (nonVolatileSettings.currentZone < 16)
		GD77SParameters.dialedZones|=(1<<nonVolatileSettings.currentZone);
	if (nonVolatileSettings.currentChannelIndexInZone < 16)
		GD77SParameters.dialedChannels|=(1<<nonVolatileSettings.currentChannelIndexInZone);
}

bool uiChannelModeTransmitDTMFContactForGD77S(void)
{// if GD77SParameters.virtualVFOMode is true, we're setting frequency not dtmf code.
	if (GD77SParameters.uiMode==GD77S_UIMODE_KEYPAD && !GD77SParameters.virtualVFOMode)
	{// Dial the string in the keyboard buffer.
		if (dtmfSequenceIsKeying())
		{
			dtmfSequenceStop();
			dtmfSequenceTick(false);
			dtmfSequenceReset();
			return true;
		}			
		if (GD77SKeypadBuffer[0])
		{// convert the string to a dialing code.
			uint8_t dtmfCodeBuffer[DTMF_CODE_MAX_LEN+1];
			if (dtmfConvertCharsToCode(GD77SKeypadBuffer, dtmfCodeBuffer, DTMF_CODE_MAX_LEN))
			{
				memset(&lastDialledDTMFContact, 0xffu, sizeof(lastDialledDTMFContact));
				dtmfSequencePrepare(dtmfCodeBuffer, true);
				memcpy(&lastDialledDTMFContact.code,&dtmfCodeBuffer, sizeof(lastDialledDTMFContact.code));

				// We've dialled something, disable autodial.
				DisableAutoDialerForCurrentChannelAndZone();
				return true;
			}
		}

		return false;	
	}
	int onceOffChannelContactIndex = GetDTMFContactIndexForZoneAndChannelAutoDial(true);

	if ((GD77SParameters.uiMode == GD77S_UIMODE_DTMF_CONTACTS) || (onceOffChannelContactIndex > 0))
	{
		if (GD77SParameters.dtmfListCount > 0)
		{
			// start dtmf sequence
			if(dtmfSequenceIsKeying() == false)
			{
				struct_codeplugDTMFContact_t dtmfContact;

				if (voicePromptsIsPlaying())
				{
					voicePromptsTerminate();
				}
				bool dialingContactForChannel = (GD77SParameters.uiMode != GD77S_UIMODE_DTMF_CONTACTS) && (onceOffChannelContactIndex > 0);
	uint16_t contactIndex = dialingContactForChannel ? onceOffChannelContactIndex : GD77SParameters.dtmfListSelected + 1;
				codeplugDTMFContactGetDataForNumber(contactIndex, &dtmfContact);
				memcpy(&lastDialledDTMFContact, &dtmfContact, sizeof(lastDialledDTMFContact));

				dtmfSequencePrepare(dtmfContact.code, true);
				// We've dialed something, disable the autodial for this channel and zone.
				DisableAutoDialerForCurrentChannelAndZone();
			}
			else
			{
				dtmfSequenceStop();
				dtmfSequenceTick(false);
				dtmfSequenceReset();
			}
		}
		else
		{
			voicePromptsInit();
			voicePromptsAppendLanguageString(&currentLanguage->empty_list);
			voicePromptsPlay();
		}

		return true;
	}

	return false;
}

void toggleTimeslotForGD77S(void)
{
	if (trxGetMode() == RADIO_MODE_DIGITAL)
	{
		// Toggle timeslot
		trxSetDMRTimeSlot(1 - trxGetDMRTimeSlot());
		tsSetManualOverride(CHANNEL_CHANNEL, (trxGetDMRTimeSlot() + 1));

		//	init_digital();
		disableAudioAmp(AUDIO_AMP_MODE_RF);
		clearActiveDMRID();
		lastHeardClearLastID();
		uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
		uiChannelModeUpdateScreen(0);
	}
}

void uiChannelModeHeartBeatActivityForGD77S(uiEvent_t *ev)
{
	static const uint32_t periods[] = { 6000, 100, 100, 100, 100, 100 };
	static const uint32_t periodsScan[] = { 2000, 50, 2000, 50, 2000, 50 };
	static uint8_t        beatRoll = 0;
	static uint32_t       mTime = 0;

	// <paranoid_mode>
	//   We use real time GPIO readouts, as LED could be turned on/off by another task.
	// </paranoid_mode>
	if ((LEDs_PinRead(GPIO_LEDred, Pin_LEDred) || LEDs_PinRead(GPIO_LEDgreen, Pin_LEDgreen)) // Any led is ON
			&& (trxTransmissionEnabled || (uiDataGlobal.DTMFContactList.isKeying) || (ev->buttons & BUTTON_PTT) || (getAudioAmpStatus() & (AUDIO_AMP_MODE_RF | AUDIO_AMP_MODE_BEEP | AUDIO_AMP_MODE_PROMPT)) || trxCarrierDetected() || ev->hasEvent)) // we're transmitting, or receiving, or user interaction.
	{
		// Turn off the red LED, if not transmitting
		if (LEDs_PinRead(GPIO_LEDred, Pin_LEDred) // Red is ON
				&& ((uiDataGlobal.DTMFContactList.isKeying == false) && ((trxTransmissionEnabled == false) || ((ev->buttons & BUTTON_PTT) == 0)))) // No TX
		{
			if ((rxPowerSavingIsRxOn() == false) && (ev->hasEvent == false))
			{
				rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);
			}

			LEDs_PinWrite(GPIO_LEDred, Pin_LEDred, 0);
		}

		// Turn off the green LED, if not receiving, or no AF output
		if (LEDs_PinRead(GPIO_LEDgreen, Pin_LEDgreen)) // Green is ON
		{
			if ((trxTransmissionEnabled || (uiDataGlobal.DTMFContactList.isKeying) || (ev->buttons & BUTTON_PTT))
					|| ((trxGetMode() == RADIO_MODE_DIGITAL) && (slot_state != DMR_STATE_IDLE))
					|| (((getAudioAmpStatus() & (AUDIO_AMP_MODE_RF | AUDIO_AMP_MODE_BEEP | AUDIO_AMP_MODE_PROMPT)) != 0) || trxCarrierDetected()))
			{
				if ((ev->buttons & BUTTON_PTT) && (trxTransmissionEnabled == false)) // RX Only or Out of Band
				{
					if ((rxPowerSavingIsRxOn() == false) && (ev->hasEvent == false))
					{
						rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);
					}

					LEDs_PinWrite(GPIO_LEDgreen, Pin_LEDgreen, 0);
				}
			}
			else
			{
				if ((rxPowerSavingIsRxOn() == false) && (ev->hasEvent == false))
				{
					rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);
				}

				LEDs_PinWrite(GPIO_LEDgreen, Pin_LEDgreen, 0);
			}
		}

		// Reset pattern sequence
		beatRoll = 0;
		// And update the timer for the next first starting (OFF for 5 seconds) blink sequence.
		mTime = ev->time;
		return;
	}

	if (!rxPowerSavingIsRxOn())
	{
		return;
	}

	// Nothing is happening, blink
	if (((trxTransmissionEnabled == false) && (uiDataGlobal.DTMFContactList.isKeying == false) && ((ev->buttons & BUTTON_PTT) == 0))
			&& ((ev->hasEvent == false) && ((getAudioAmpStatus() & (AUDIO_AMP_MODE_RF | AUDIO_AMP_MODE_BEEP | AUDIO_AMP_MODE_PROMPT)) == 0) && (trxCarrierDetected() == false)))
	{
		// Blink both LEDs to have Orange color
		if ((ev->time - mTime) > (uiDataGlobal.Scan.active ? periodsScan[beatRoll] : periods[beatRoll]))
		{
			if ((nonVolatileSettings.ecoLevel > 0) && (ev->hasEvent == false))
			{
				rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);
			}

			mTime = ev->time;
			beatRoll = (beatRoll + 1) % (uiDataGlobal.Scan.active ? (sizeof(periodsScan) / sizeof(periodsScan[0])) : (sizeof(periods) / sizeof(periods[0])));
			LEDs_PinWrite(GPIO_LEDred, Pin_LEDred, (beatRoll % 2));
			LEDs_PinWrite(GPIO_LEDgreen, Pin_LEDgreen, (beatRoll % 2));
		}
	}
	else
	{
		// Reset pattern sequence
		beatRoll = 0;
		// And update the timer for the next first starting (OFF for 5 seconds) blink sequence.
		mTime = ev->time;
	}
}

static uint16_t getCurrentChannelInCurrentZoneForGD77S(void)
{
	return (CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : nonVolatileSettings.currentChannelIndexInZone + 1);
}

static void checkAndUpdateSelectedChannelForGD77S(uint16_t chanNum, bool forceSpeech)
{
	bool updateDisplay = false;

	if(CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone))
	{
		GD77SParameters.channelOutOfBounds = false;
		if (codeplugAllChannelsIndexIsInUse(chanNum))
		{
			if (chanNum != nonVolatileSettings.currentChannelIndexInAllZone)
			{
				settingsSet(nonVolatileSettings.currentChannelIndexInAllZone, (int16_t) chanNum);
				loadChannelData(false, false);
				updateDisplay = true;
			}
		}
		else
		{
			GD77SParameters.channelOutOfBounds = true;
			if (voicePromptsIsPlaying() == false)
			{
				voicePromptsInit();
				voicePromptsAppendPrompt(PROMPT_CHANNEL);
				voicePromptsAppendLanguageString(&currentLanguage->error);
				voicePromptsPlay();
			}
		}
	}
	else
	{
		if ((chanNum - 1) < currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
		{
			GD77SParameters.channelOutOfBounds = false;
			if ((chanNum - 1) != nonVolatileSettings.currentChannelIndexInZone)
			{
				settingsSet(nonVolatileSettings.currentChannelIndexInZone, (int16_t) (chanNum - 1));
				loadChannelData(false, false);
				updateDisplay = true;
			}
		}
		else
		{
			GD77SParameters.channelOutOfBounds = true;
			if (voicePromptsIsPlaying() == false)
			{
				voicePromptsInit();
				voicePromptsAppendPrompt(PROMPT_CHANNEL);
				voicePromptsAppendLanguageString(&currentLanguage->error);
				voicePromptsPlay();
			}
		}
	}

	// Prevent TXing while an invalid channel is selected
	if (getCurrentChannelInCurrentZoneForGD77S() != chanNum)
	{
		PTTLocked = true;
	}
	else
	{
		if (PTTLocked)
		{
			PTTLocked = false;
			forceSpeech = true;
		}
	}

	if (updateDisplay || forceSpeech)
	{
		// Remove TS override when a new channel is selected, otherwise it will be set until the zone is changed.
		if (updateDisplay && (tsGetManualOverride(CHANNEL_CHANNEL) != 0))
		{
			tsSetManualOverride(CHANNEL_CHANNEL, TS_NO_OVERRIDE);
		}

		if (GD77SParameters.channelOutOfBounds == false)
		{
			char buf[17];

			voicePromptsInit();
			voicePromptsAppendPrompt(PROMPT_CHANNEL);
			voicePromptsAppendInteger(chanNum);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			codeplugUtilConvertBufToString(currentChannelData->name, buf, 16);
			if (chanNum!=atoi(buf))
			{// Only append name if different from the channel number.
				voicePromptsAppendString(buf);
			}
			if (trxGetMode() == RADIO_MODE_DIGITAL)
				announceContactNameTgOrPc(true); // avoid saying "contact", just announce the tg.

			voicePromptsPlay();
		}

		if (!forceSpeech)
		{
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);
		}
	}
}

static char GetGD77SKeypadChar()
{
	if (GD77sSelectedCharIndex < 0 || GD77sSelectedCharIndex > 15)
		return 0;
	
	switch (GD77sKeypadBankIndex)
	{
		case 1:
			return GD77S_KEYPAD_BANK1_CHARS[GD77sSelectedCharIndex];
		case 2:
			return GD77S_KEYPAD_BANK2_CHARS[GD77sSelectedCharIndex];
		default:
		return DTMF_AllowedChars[GD77sSelectedCharIndex];
	}
}
static void 			AnnounceGD77sKeypadChar(bool init)
{
	char buf[2] = {0,0};
	buf[0]=GetGD77SKeypadChar();
	if (init)
		voicePromptsInit();
	voicePromptsAppendStringEx(buf, vpAnnounceSpaceAndSymbols);
	voicePromptsPlay();
}

static void 			AnnounceGD77sKeypadBuffer(void)
{
	voicePromptsInit();
	voicePromptsAppendStringEx(GD77SKeypadBuffer, vpAnnounceSpaceAndSymbols);
	voicePromptsPlay();
}

static void AddGD77sKeypadChar(void)
{
	if (GD77SKeypadPos >=GD77S_KEYPAD_BUF_MAX-1)
		return;
	
	GD77SKeypadBuffer[GD77SKeypadPos++]= GetGD77SKeypadChar();
	AnnounceGD77sKeypadChar(true);
	GD77SKeypadBuffer[GD77SKeypadPos]='\0';
}

static void BackspaceGD77sKeypadChar(void)
{
	if (GD77SKeypadPos <= 0)
	{
		soundSetMelody(MELODY_KEY_LONG_BEEP);
		return;
	}
	GD77SKeypadPos--;
	announceChar(GD77SKeypadBuffer[GD77SKeypadPos]);
	GD77SKeypadBuffer[GD77SKeypadPos]= '\0';
	if (GD77SKeypadPos==0)
		GD77SParameters.virtualVFOMode=false;
}

static void ClearGD77sKeypadBuffer(void)
{
	GD77SKeypadPos = 0;
	memset(GD77SKeypadBuffer, 0, sizeof(GD77SKeypadBuffer));
	GD77SParameters.virtualVFOMode=false;
	soundSetMelody(MELODY_KEY_LONG_BEEP);
}

static bool AnnounceGD77sVoiceOption(bool alwaysAnnounceOptionName, bool clearPriorPromptInProgress)
{
	bool voicePromptWasPlaying = alwaysAnnounceOptionName ? false : voicePromptsIsPlaying();

	voicePrompt_t vp = NUM_VOICE_PROMPTS;
	char * const *vpString = NULL;
	char * const *vpValueString = NULL;
	char buf[5]="\0";
	bool announceVoiceName=false;
	switch (GD77SParameters.option)
	{
	case 		GD77S_OPTION_VOLUME:
		vpString = (char * const *)&currentLanguage->voice_prompt_vol;
		snprintf(buf, 5, "%d%%", nonVolatileSettings.voicePromptVolumePercent);
		announceVoiceName=true;
		break;
	case 	GD77S_OPTION_RATE:
		vpString = (char * const *)&currentLanguage->voice_prompt_rate;
		snprintf(buf, 5, "%d", nonVolatileSettings.voicePromptRate+1);
		announceVoiceName=true;
		break;
	case 	GD77S_OPTION_PHONETIC_SPELL:
		vpString = (char * const *)&currentLanguage->phoneticSpell;
		if (settingsIsOptionBitSet(BIT_PHONETIC_SPELL))
			vpValueString= (char * const *)&currentLanguage->on;
		else
			vpValueString= (char * const *)&currentLanguage->off;
		break;
	case 	GD77S_OPTION_DMR_ID_ANNOUNCE:
		vpString = (char * const *)&currentLanguage->dmr_id;
		if (settingsIsOptionBitSet(BIT_ANNOUNCE_LASTHEARD))
			vpValueString= (char * const *)&currentLanguage->on;
		else
			vpValueString= (char * const *)&currentLanguage->off;
		break;
	case 	GD77S_OPTION_CUSTOM_PROMPT_REVIEW:
	case 	GD77S_OPTION_CUSTOM_PROMPT_RECORD:
		vpString = (char * const *)&currentLanguage->audio_prompt;
		break;
	case 	GD77S_OPTION_EDIT_START:
		vpString = (char * const *)&currentLanguage->start;
		break;
	case 	GD77S_OPTION_EDIT_END:
		vpString = (char * const *)&currentLanguage->end;
		break;
	default:
		return false;
	}
	if (clearPriorPromptInProgress)
	voicePromptsInit();
	if (!voicePromptWasPlaying)
	{
		if (announceVoiceName)
			voicePromptsAppendPrompt(PROMPT_VOICE_NAME);
		if (GD77SParameters.option == 	GD77S_OPTION_EDIT_START || GD77SParameters.option == 	GD77S_OPTION_EDIT_END)
			voicePromptsAppendPrompt(PROMPT_EDIT_VOICETAG);
		// distinguish between ptt prompt record mode and prompt review mode.
		if (GD77SParameters.option == 	GD77S_OPTION_CUSTOM_PROMPT_RECORD)
			voicePromptsAppendLanguageString(&currentLanguage->ptt);
		if (GD77SParameters.option == 	GD77S_OPTION_CUSTOM_PROMPT_REVIEW)
			voicePromptsAppendLanguageString(&currentLanguage->all);

		if (vp != NUM_VOICE_PROMPTS)
			voicePromptsAppendPrompt(vp);
		else
		{
			if (vpString != NULL)
				voicePromptsAppendLanguageString((const char * const *)vpString);
		}
	}
	if (vpValueString != NULL)
		voicePromptsAppendLanguageString((const char * const *)vpValueString);
	else
	{
		if (buf[0])
			voicePromptsAppendString(buf);
		if ((GD77SParameters.option == 	GD77S_OPTION_CUSTOM_PROMPT_REVIEW) && (GD77SParameters.customPromptIndex > 0))
		{
			voicePromptsAppendInteger(GD77SParameters.customPromptIndex);
			voicePromptsAppendPrompt(VOICE_PROMPT_CUSTOM + GD77SParameters.customPromptIndex);
		}
	}
	voicePromptsPlay();
	return true;
}

static void AnnounceGD77SOption(bool alwaysAnnounceOptionName, bool clearPriorPromptInProgress)
{
	if (AnnounceGD77sVoiceOption( alwaysAnnounceOptionName,  clearPriorPromptInProgress))
		return;
	bool voicePromptWasPlaying = alwaysAnnounceOptionName ? false : voicePromptsIsPlaying();
	if (clearPriorPromptInProgress)
		voicePromptsInit();
	
	char rightSideVar[SCREEN_LINE_BUFFER_SIZE]="\0";

	switch (GD77SParameters.option)
	{
		case GD77S_OPTION_POWER:
			announcePowerLevel(voicePromptWasPlaying);
			break;
		case GD77S_OPTION_BAND_LIMITS:
		{
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->band_limits);
			switch(nonVolatileSettings.txFreqLimited)
			{
				case BAND_LIMITS_NONE:
					voicePromptsAppendLanguageString(&currentLanguage->off);
					break;
				case BAND_LIMITS_ON_LEGACY_DEFAULT:
					voicePromptsAppendLanguageString(&currentLanguage->on);
					break;
				case BAND_LIMITS_FROM_CPS:
					strcpy(rightSideVar,"CPS");
					voicePromptsAppendString(rightSideVar);
					break;
			}
			break;
		}
		case GD77S_OPTION_HOTSPOT_MODE:
		{
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->hotspot_mode);
			if (uiDataGlobal.dmrDisabled)
			{
				voicePromptsAppendLanguageString(&currentLanguage->n_a);
			}
			else
			{
				if (nonVolatileSettings.hotspotType == 0)
				{
					voicePromptsAppendLanguageString(&currentLanguage->off);
				}
				else
				{
					voicePromptsAppendString((nonVolatileSettings.hotspotType == HOTSPOT_TYPE_MMDVM) ? "MMDVM" : "BlueDV");
				}
			}
			break;
		}
		case GD77S_OPTION_FM_MIC_GAIN:
			announceMicGain(voicePromptWasPlaying==false, true, false);
			break;
		case GD77S_OPTION_DMR_MIC_GAIN:
			announceMicGain(voicePromptWasPlaying==false, true, true);
			break;
		case GD77S_OPTION_VHF_SQUELCH:
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->squelch_VHF);
			snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%d%%", (nonVolatileSettings.squelchDefaults[RADIO_BAND_VHF] - 1) * 5);// 5% steps
			voicePromptsAppendString(rightSideVar);
			break;
		case GD77S_OPTION_UHF_SQUELCH:
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->squelch_UHF);
			snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%d%%", (nonVolatileSettings.squelchDefaults[RADIO_BAND_UHF] - 1) * 5);// 5% steps
			voicePromptsAppendString(rightSideVar);
			break;
		case GD77S_OPTION_CSS_TAIL_ELIM:
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->css_tail);
			if (nonVolatileSettings.ctcssSqlTail > 0)
			{
				voicePromptsAppendInteger(nonVolatileSettings.ctcssSqlTail*10);
				voicePromptsAppendPrompt(PROMPT_MILLISECONDS);
			}
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
			break;
		case GD77S_OPTION_FM_BEEP:
		case GD77S_OPTION_DMR_BEEP:
		{
			const char * const *beepTX[] = { &currentLanguage->none, &currentLanguage->start, &currentLanguage->stop, &currentLanguage->both };
			bool dmrBeep=GD77SParameters.option==GD77S_OPTION_DMR_BEEP;
			
			if (dmrBeep)
				strcpy(rightSideVar, "DMR");
			else
				strcpy(rightSideVar, "FM");
			voicePromptsAppendString(rightSideVar);
			voicePromptsAppendLanguageString(&currentLanguage->beep);
			if (dmrBeep)
				voicePromptsAppendLanguageString( (const char * const *)beepTX[nonVolatileSettings.beepOptions&(BEEP_TX_START+BEEP_TX_STOP)]);
			else
				voicePromptsAppendLanguageString( (const char * const *)beepTX[nonVolatileSettings.beepOptions>>2]);
			break;
		}
		case GD77S_OPTION_RX_END_BEEP:
		{
			if (!voicePromptWasPlaying)
			{
				voicePromptsAppendLanguageString(&currentLanguage->end);
				voicePromptsAppendPrompt(PROMPT_RECEIVE);
				voicePromptsAppendLanguageString(&currentLanguage->beep);
			}
			if (nonVolatileSettings.endRXBeep==END_RX_BEEP_OFF)
				voicePromptsAppendLanguageString(&currentLanguage->off);
			if (nonVolatileSettings.endRXBeep==END_RX_BEEP_DMR)
				voicePromptsAppendPrompt(PROMPT_DMR);
			if (nonVolatileSettings.endRXBeep==END_RX_BEEP_FM)
				voicePromptsAppendPrompt(PROMPT_FM);
			if (nonVolatileSettings.endRXBeep==END_RX_BEEP_BOTH)
				voicePromptsAppendLanguageString(&currentLanguage->both);
			break;
		}
		case GD77S_OPTION_DTMF_VOL:
			voicePromptsAppendLanguageString(&currentLanguage->dtmf_vol);
			if (nonVolatileSettings.dtmfVol > 0)
			{
				snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%d",nonVolatileSettings.dtmfVol);
				voicePromptsAppendString(rightSideVar);
			}
			else
			{
				voicePromptsAppendLanguageString(&currentLanguage->off);
			}
			break;
		case GD77S_OPTION_TOT_MASTER:
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->tot);
			if (nonVolatileSettings.totMaster > 0)
			{
				snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%u", nonVolatileSettings.totMaster * 15);
				voicePromptsAppendString(rightSideVar);
			}
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
			break;
		case GD77S_OPTION_PTT_LATCH:
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->ptt_toggle);
			if (settingsIsOptionBitSet(BIT_PTT_LATCH))
				voicePromptsAppendLanguageString(&currentLanguage->on);
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
			break;
		case GD77S_OPTION_ECO:
			announceEcoLevel(voicePromptWasPlaying);
			break;
		case GD77S_OPTION_AUTOZONE:
			if (!voicePromptWasPlaying)
				voicePromptsAppendLanguageString(&currentLanguage->autoZone);
			AutoZoneInitialize(GD77SParameters.autoZoneTypeIndex);
			voicePromptsAppendString((char*)autoZone.name);
			if (IsBitSet(nonVolatileSettings.autoZonesEnabled, GD77SParameters.autoZoneTypeIndex))
				voicePromptsAppendLanguageString(&currentLanguage->on);
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
			break;
		case GD77S_OPTION_FIRMWARE_INFO:
			snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "[ %s", GITVERSION);
			rightSideVar[9] = 0; // git hash id 7 char long;
			strcat(rightSideVar, (uiDataGlobal.dmrDisabled ? " F ]" : " D ]"));
			voicePromptsAppendLanguageString(&currentLanguage->firmware_info);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			voicePromptsAppendLanguageString(&currentLanguage->openGD77S);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			voicePromptsAppendLanguageString(&currentLanguage->built);
			voicePromptsAppendString(__TIME__);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			voicePromptsAppendString(__DATE__);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			voicePromptsAppendLanguageString(&currentLanguage->gitCommit);
			voicePromptsAppendString(rightSideVar);
			break;
					case 	GD77S_OPTION_VOX_THRESHOLD:
			voicePromptsAppendLanguageString(&currentLanguage->vox_threshold);
			if (nonVolatileSettings.voxThreshold > 0)
			{
				snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%d", nonVolatileSettings.voxThreshold);
				voicePromptsAppendString(rightSideVar);
			}
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
			break;
		case GD77S_OPTION_VOX_TAIL:
			voicePromptsAppendLanguageString(&currentLanguage->vox_tail);
			if (nonVolatileSettings.voxThreshold != 0)
			{
				float tail = (nonVolatileSettings.voxTailUnits * 0.5);
				uint8_t secs = (uint8_t)tail;
				uint8_t fracSec = (tail - secs) * 10;

				snprintf(rightSideVar, SCREEN_LINE_BUFFER_SIZE, "%d.%d", secs, fracSec);
				voicePromptsAppendString(rightSideVar);
				voicePromptsAppendLanguageString(&currentLanguage->seconds);
			}
			else
			{
				voicePromptsAppendLanguageString(&currentLanguage->n_a);
			}
			break;
		case GD77S_OPTION_KEYPAD_SOURCE:
			voicePromptsAppendLanguageString(&currentLanguage->keypad);
			if (nonVolatileSettings.currentVFONumber==0 || nonVolatileSettings.currentVFONumber==1)
				announceVFOChannelName();
			else
				voicePromptsAppendPrompt(PROMPT_CHANNEL);
			break;
		case GD77S_OPTION_MAX:
			return;
	};
	voicePromptsPlay();
}

static void buildSpeechUiModeForGD77S(GD77S_UIMODES_t uiMode)
{
	char buf[17];

	if (voicePromptsIsPlaying())
	{
		voicePromptsTerminate();
	}

	switch (uiMode)
	{
		case GD77S_UIMODE_TG_OR_SQUELCH: // Channel
			codeplugUtilConvertBufToString(currentChannelData->name, buf, 16);
			voicePromptsAppendString(buf);

			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				announceTS();
			}
			else
			{
				announceSquelchLevel(voicePromptsIsPlaying());
			}
			break;

		case GD77S_UIMODE_SCAN: // Scan
			voicePromptsAppendLanguageString(&currentLanguage->scan);
			voicePromptsAppendLanguageString(uiDataGlobal.Scan.active ? &currentLanguage->start : &currentLanguage->stop);
			break;

		case GD77S_UIMODE_TS: // Timeslot
			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				announceTS();
			}
			break;

		case GD77S_UIMODE_CC: // Color code
			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				announceCC();
			}
			break;

		case GD77S_UIMODE_FILTER: // DMR/Analog filter
			voicePromptsAppendLanguageString(&currentLanguage->filter);
			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				if (nonVolatileSettings.dmrDestinationFilter == DMR_DESTINATION_FILTER_NONE)
				{
					voicePromptsAppendLanguageString(&currentLanguage->none);
				}
				else
				{
					voicePromptsAppendString((char *)DMR_DESTINATION_FILTER_LEVELS[nonVolatileSettings.dmrDestinationFilter - 1]);
				}

			}
			else
			{
				if (nonVolatileSettings.analogFilterLevel == ANALOG_FILTER_NONE)
				{
					voicePromptsAppendLanguageString(&currentLanguage->none);
				}
				else
				{
					voicePromptsAppendString((char *)ANALOG_FILTER_LEVELS[nonVolatileSettings.analogFilterLevel - 1]);
				}
			}
			break;

		case GD77S_UIMODE_DTMF_CONTACTS:
			if (GD77SParameters.dtmfListCount > 0)
			{
				struct_codeplugDTMFContact_t dtmfContact;

				codeplugDTMFContactGetDataForNumber(GD77SParameters.dtmfListSelected + 1, &dtmfContact);
				codeplugUtilConvertBufToString(dtmfContact.name, buf, 16);
				voicePromptsAppendString(buf);
			}
			else
			{
				voicePromptsAppendLanguageString(&currentLanguage->empty_list);
			}
			break;

		case GD77S_UIMODE_ZONE: // Zone
			announceZoneName(voicePromptsIsPlaying());
			break;
		case GD77S_UIMODE_KEYPAD:
			if (nonVolatileSettings.currentVFONumber==0 || nonVolatileSettings.currentVFONumber==1)
				announceVFOChannelName();
			else
				announceChannelName(true, false);
			AnnounceGD77sKeypadChar(false);
			break;
		case GD77S_UIMODE_VOX:
			if (currentChannelData->flag4 & 0x40)
				voicePromptsAppendLanguageString(&currentLanguage->on);
			else
				voicePromptsAppendLanguageString(&currentLanguage->off);
			break;
		case GD77S_UIMODE_OPTIONS1:
		case GD77S_UIMODE_OPTIONS2:
			AnnounceGD77SOption(true, false);
			break;
		case GD77S_UIMODE_MAX:
			break;
	}
}

static bool SaveChannelToCurrentZone(uint16_t zoneChannelIndex)
{
	bool addToZone= zoneChannelIndex > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone;
	uint16_t physicalChannelIndex;
	if (addToZone)
	{
		for (physicalChannelIndex = CODEPLUG_CHANNELS_MIN; physicalChannelIndex <= CODEPLUG_CHANNELS_MAX; physicalChannelIndex++)
		{
			if (!codeplugAllChannelsIndexIsInUse(physicalChannelIndex))
			{
				break;
			}
		}
	}
	else
		physicalChannelIndex = currentZone.channels[zoneChannelIndex-1];
	if (physicalChannelIndex >= CODEPLUG_CONTACTS_MAX)
	{// memory full.
		soundSetMelody(MELODY_ERROR_BEEP);
		return false;
	}
	
	if (!addToZone) // grab the existing name if there is one so it isn't overwritten.
		GetChannelName(zoneChannelIndex-1, false, currentChannelData->name, 16);
	
	if (addToZone || !currentChannelData->name[0])
	{
		char channelName[16]="\0";
		snprintf(channelName, 16, "%d", zoneChannelIndex);
		codeplugUtilConvertStringToBuf(channelName, currentChannelData->name, 16);
	}
	codeplugChannelSaveDataForIndex(physicalChannelIndex, currentChannelData);

	if (addToZone)
	{
		codeplugAllChannelsIndexSetUsed(physicalChannelIndex, true); //Set channel index as valid
		if (currentZone.NOT_IN_CODEPLUGDATA_indexNumber == 0xDEADBEEF)
		{
			uiChannelInitializeCurrentZone();
		}

		if (codeplugZoneAddChannelToZoneAndSave(physicalChannelIndex, &currentZone))
		{
			settingsSet(nonVolatileSettings.currentChannelIndexInZone , currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone - 1);
		}
	}

	voicePromptsInit();
	voicePromptsAppendLanguageString(&currentLanguage->vfoToChannel);
	voicePromptsAppendInteger(zoneChannelIndex);
	voicePromptsPlay();
	return true;
}

static int GetPowerLevelFromMilliwatValue(int number)
{
	if (number <= 125)
	{
		return 1;
	}
	if (number <= 375)
	{
		return 2;
	}
	if (number <= 625)
	{
		return 3;
	}
	if (number <= 875)
	{
		return 4;
	}
	if (number <= 1500)
	{
		return 5;
	}
	if (number <= 2500)
	{
		return 6;
	}
	if (number <= 3500)
	{
			return 7;
	}
	if (number <= 4500)
	{
		return 8;
	}
	if (number <= 5000)
	{
		return 9;
	}
	return 10;
}

static bool ProcessGD77SKeypadCmd(uiEvent_t *ev)
{
	if (!GD77SKeypadBuffer[0])
	{// copy channel to vfo
		memcpy(&settingsVFOChannel[CHANNEL_VFO_A].rxFreq, &channelScreenChannelData.rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16); // Don't copy the name of the vfo, which is in the first 16 bytes

		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->channelToVfo);
		voicePromptsPlay();
		
		return true; // will force saving to VFO.
	}
	if (strcmp(GD77SKeypadBuffer, "*")==0)
	{// Toggle DMR and FM mode.
		if (trxGetMode() == RADIO_MODE_ANALOG)
		{
			currentChannelData->chMode = RADIO_MODE_DIGITAL;
			// in case it is not set:
			currentChannelData->rxColor=trxGetDMRColourCode();
			currentChannelData->txColor=currentChannelData->rxColor;
			uiDataGlobal.VoicePrompts.inhibitInitial = true;// Stop VP playing in loadChannelData
			loadChannelData(true, false);
			uiDataGlobal.VoicePrompts.inhibitInitial = false;
		}
		else
		{
			currentChannelData->chMode = RADIO_MODE_ANALOG;
			trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4 & 0x02) == 0x02));
			trxSetRxCSS(currentChannelData->rxTone);
		}
		announceItem(PROMPT_SEQUENCE_MODE, PROMPT_THRESHOLD_2);
		return true;	
	}
	if (strncmp(GD77SKeypadBuffer, "*##", 3)==0 && (isdigit(GD77SKeypadBuffer[3]) || GD77SKeypadBuffer[3]=='*'))
	{// save custom voice prompt.
		int customPromptNumber=(GD77SKeypadBuffer[3]=='*') ? GetNextFreeVoicePromptIndex(false) : atoi(GD77SKeypadBuffer+3);
		char* phrasePtr=GD77SKeypadBuffer+3;
		while (phrasePtr && *phrasePtr && (isdigit(*phrasePtr) || *phrasePtr=='*'))
		{
			phrasePtr++;
		}
		SaveCustomVoicePrompt(customPromptNumber, phrasePtr);
		GD77SParameters.customPromptIndex=customPromptNumber;
		return true;
	}

	if (strncmp(GD77SKeypadBuffer, "*#", 2)==0 && strlen(GD77SKeypadBuffer) >= 8)
	{// Add a DTMF contact name is first 6 chars after *#, code is rest of string.
		char name[7]="\0";
		char code[DTMF_CODE_MAX_LEN+1]="\0";
		strncpy(name, GD77SKeypadBuffer+2, 6);
		name[6]='\0';
		bool deleting=strlen(GD77SKeypadBuffer)==8;
		if (!deleting)
			strcpy(code,GD77SKeypadBuffer+8);
		struct_codeplugDTMFContact_t tmpDTMFContact;
		memset(&tmpDTMFContact, 0xFFU, sizeof(struct_codeplugDTMFContact_t));
		codeplugUtilConvertStringToBuf(name, tmpDTMFContact.name, DTMF_NAME_MAX_LEN);
		
		int dtmfContactIndex =codeplugGetDTMFContactIndex(name);
		// see if we are deleting the contact.
		if (deleting)
			memset(&tmpDTMFContact, 0xFFU, sizeof(struct_codeplugDTMFContact_t));
		else
			dtmfConvertCharsToCode(code, tmpDTMFContact.code, DTMF_CODE_MAX_LEN);

		if (dtmfContactIndex==0 && !deleting)
			dtmfContactIndex=GD77SParameters.dtmfListCount+1;
		voicePromptsInit();

		if ((dtmfContactIndex >= CODEPLUG_DTMF_CONTACTS_MIN) && (dtmfContactIndex <= CODEPLUG_DTMF_CONTACTS_MAX))
		{
			codeplugContactSaveDTMFDataForIndex(dtmfContactIndex, &tmpDTMFContact);
			GD77SParameters.dtmfListCount = codeplugDTMFContactsGetCount();
			GD77SParameters.dtmfListSelected=dtmfContactIndex-1; // GD77S parameters uses 0-based index.
			voicePromptsAppendString(name);
			if (deleting)
			voicePromptsAppendLanguageString(&currentLanguage->contact_deleted);
			else
			voicePromptsAppendLanguageString(&currentLanguage->contact_saved);
		}
		else
		{// error.
			if (dtmfContactIndex > CODEPLUG_DTMF_CONTACTS_MAX)
				voicePromptsAppendLanguageString(&currentLanguage->list_full);
			else
				voicePromptsAppendLanguageString(&currentLanguage->error);
						}
		voicePromptsPlay();
		return true;
	}
	if (strncmp(GD77SKeypadBuffer, "**", 2)==0)
	{// set or clear channel specific power.
		int number = isdigit(GD77SKeypadBuffer[2]) ? atoi(GD77SKeypadBuffer+2) : 0;
		if (number >=50)
			number=GetPowerLevelFromMilliwatValue(number);
		if (number==0)
		{// Clear channel specific power.
			currentChannelData->libreDMR_Power = 0x00;
			trxSetPowerFromLevel(nonVolatileSettings.txPowerLevel);
		}
		else if (number >=CODEPLUG_MIN_PER_CHANNEL_POWER && number <=(MAX_POWER_SETTING_NUM + CODEPLUG_MIN_PER_CHANNEL_POWER))
		{
			currentChannelData->libreDMR_Power=number;
			trxSetPowerFromLevel(currentChannelData->libreDMR_Power - 1);
		}
				
		announceItem(PROMPT_SEQUENCE_POWER, PROMPT_THRESHOLD_1);
		if ( currentChannelData->libreDMR_Power == 0)
		{
			voicePromptsAppendLanguageString(&currentLanguage->from_master);
		}
		voicePromptsPlay();
		return true;
	}
	if (strncmp(GD77SKeypadBuffer,"DEL", 3)==0 && isdigit(GD77SKeypadBuffer[3]))
	{// DEL1 through DEL16 delete  channel name 
		int zoneChannelIndex = atoi(GD77SKeypadBuffer+3);
		if (zoneChannelIndex < 1 || zoneChannelIndex > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}
		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->delete_from_all_zones);
		voicePromptsPlay();
		codeplugDeleteChannelWithIndex(currentZone.channels[zoneChannelIndex-1]);
		codeplugZoneDeleteChannelFromZone(currentZone.channels[zoneChannelIndex-1], &currentZone);

		return true;
	}
	
	if (GD77SKeypadBuffer[0]=='S' && isdigit(GD77SKeypadBuffer[1]))
	{// S1 3 swap  ch 1 with 3.
		char temp[GD77S_KEYPAD_BUF_MAX];
		strncpy(temp, GD77SKeypadBuffer, GD77S_KEYPAD_BUF_MAX);
		char* srcStr=strtok(temp+1, ", ");
		char* destStr=strtok(NULL, ", ");
		if (!srcStr || !destStr || !isdigit(*srcStr) || !isdigit(*destStr))
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}
		int srcChannelIndex = atoi(srcStr);
		int destIndex = atoi(destStr);
		if (srcChannelIndex < 1 || srcChannelIndex > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone || destIndex < 1 || destIndex > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone || srcChannelIndex==destIndex)
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}
		if (!codeplugZoneReorderChannels(srcChannelIndex-1, destIndex-1, &currentZone))
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}
		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->reorder_channels);
		voicePromptsPlay();

		return true;
	}

	if (trxGetMode() == RADIO_MODE_DIGITAL && GD77SKeypadBuffer[0]=='#')
	{// talkgroup # followed by digits. 
		int len=strlen(GD77SKeypadBuffer);
		int hashCount=1;
		while (GD77SKeypadBuffer[hashCount]=='#' && hashCount < len)
		{
			hashCount++;
		}
		bool privateCall=hashCount==2;
		bool changeUserDMRID = hashCount==3;
		int digitOffset=hashCount; 
		if (isdigit(GD77SKeypadBuffer[digitOffset]))
		{
			uint32_t dmrID=atol(GD77SKeypadBuffer+digitOffset);
			if (changeUserDMRID)
			{
				trxDMRID=dmrID;
				codeplugSetUserDMRID(trxDMRID);
				uiDataGlobal.userDMRId = trxDMRID;

				voicePromptsInit();
				voicePromptsAppendLanguageString(&currentLanguage->user_dmr_id);
				voicePromptsAppendInteger(trxDMRID);
				voicePromptsPlay();
			}
			else
			{
				setOverrideTGorPC(dmrID, privateCall);
				updateTrxID();
				announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC, PROMPT_THRESHOLD_2);
			}
		}
		else // toggle time slot.
		{
			trxSetDMRTimeSlot(1 - trxGetDMRTimeSlot());
			tsSetManualOverride(CHANNEL_CHANNEL, (trxGetDMRTimeSlot() + 1));

			if ((nonVolatileSettings.overrideTG == 0) && (currentContactData.reserve1 & 0x01) == 0x00)
			{
				tsSetContactHasBeenOverriden(CHANNEL_CHANNEL, true);
			}

			//	init_digital();
			disableAudioAmp(AUDIO_AMP_MODE_RF);
			clearActiveDMRID();
			lastHeardClearLastID();
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);

			announceItem(PROMPT_SEQUENCE_TS,PROMPT_THRESHOLD_2);
		}	
		return true;
	}
	if (trxGetMode() == RADIO_MODE_ANALOG)
	{
		if (strcmp(GD77SKeypadBuffer, "B")==0)
		{
			uint8_t mask=currentChannelData->flag4;
			if (mask&0x02) // wide, set to narrow.
				mask&=~0x02;
			else // narrow, set to wide.
				mask|=0x02;
			currentChannelData->flag4=mask;
			trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4&0x02)==0x02));
			announceItem(PROMPT_SEQUENCE_BANDWIDTH, PROMPT_THRESHOLD_2);
						return true;
		}
		if ((GD77SKeypadBuffer[0]=='C' || GD77SKeypadBuffer[0]=='D'))
		{// CTCSS/DCS
			bool dcs=GD77SKeypadBuffer[0]=='D';
			bool hasDigits=isdigit(GD77SKeypadBuffer[1]);
			int bufLen=strlen(GD77SKeypadBuffer);
			bool forBothRXAndTX=GD77SKeypadBuffer[bufLen-1]=='*';
			uint16_t tone=CODEPLUG_CSS_TONE_NONE;
			if (hasDigits)
			{
				if (dcs)
					tone = strtol(GD77SKeypadBuffer+1, NULL, 16) | CSS_TYPE_DCS; //need hex which is why we use strtol.
				else
					tone = atoi(GD77SKeypadBuffer+1);//tone is decimal
			}
			// If DCS, or in the CSS_TYPE_DCS_MASK
			currentChannelData->rxTone=CODEPLUG_CSS_TONE_NONE;
			currentChannelData->txTone=tone;
			if (forBothRXAndTX)
				currentChannelData->rxTone=tone;
			else
				currentChannelData->rxTone=CODEPLUG_CSS_TONE_NONE;

			announceCSSCode(tone, codeplugGetCSSType(tone), DIRECTION_NONE, true, AUDIO_PROMPT_MODE_VOICE_LEVEL_2);
			return true;
		}
	}
	if (GD77SKeypadBuffer[0]=='A' && strlen(GD77SKeypadBuffer) <=3)
	{
		// a1 through a16 copy vfo back to permanent channel 1 to 16 in real zone (won't work in autozone), note name is left unchanged.
		int zoneChannelIndex=0;
		if (isdigit(GD77SKeypadBuffer[1])) 
			zoneChannelIndex=atoi(GD77SKeypadBuffer+1);
		else
			zoneChannelIndex=currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone+1; // create one.
		SaveChannelToCurrentZone(zoneChannelIndex);
		return true;
	}
	if (GD77SKeypadBuffer[0]=='R' && strlen(GD77SKeypadBuffer) <=3)
	{
		// R1 through R16 copy channel to current vfo
		int zoneChannelIndex=0;
		if (!isdigit(GD77SKeypadBuffer[1]) || nonVolatileSettings.currentVFONumber > 1)
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}			
		zoneChannelIndex=atoi(GD77SKeypadBuffer+1);
		if (zoneChannelIndex < 1 || zoneChannelIndex > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}
		codeplugChannelGetDataForIndex(currentZone.channels[zoneChannelIndex-1], currentChannelData);
		memcpy(&settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, &channelScreenChannelData.rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16);// copy all channel details except name.
		codeplugSetVFO_ChannelData(&settingsVFOChannel[nonVolatileSettings.currentVFONumber], nonVolatileSettings.currentVFONumber);
		trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
		trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4 & 0x02) == 0x02));
		voicePromptsInit();
		voicePromptsAppendLanguageString(&currentLanguage->channelToVfo);
		voicePromptsPlay();
		return true;
	}

	if (GD77SKeypadBuffer[0]=='E' && isdigit(GD77SKeypadBuffer[1]) && strlen(GD77SKeypadBuffer) > 2)
	{// e1 through e16 edit channel name 
		int zoneChannelIndex = atoi(GD77SKeypadBuffer+1);
		char* namePtr=GD77SKeypadBuffer+1;
		while (namePtr && *namePtr && isdigit(*namePtr))
		{
			namePtr++;
		}
		// copy the name to the specific channel.
		if (zoneChannelIndex < 1 || zoneChannelIndex > currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone)
		{
			soundSetMelody(MELODY_ERROR_BEEP);
			return true;	
		}
		codeplugChannelGetDataForIndex(currentZone.channels[zoneChannelIndex-1], currentChannelData);
		codeplugUtilConvertStringToBuf(namePtr, currentChannelData->name, 16);
		codeplugChannelSaveDataForIndex(currentZone.channels[zoneChannelIndex-1], currentChannelData);
		voicePromptsInit();
		voicePromptsAppendString(namePtr);
		voicePromptsPlay();
		return true;
	}
	
	return false;	
}

static bool HandleGD77sKbdEvent(uiEvent_t *ev)
{
	if (GD77SParameters.uiMode!=GD77S_UIMODE_KEYPAD)
		return false;
	if (GD77SParameters.cycleFunctionsInReverse)
		return false;

	if (BUTTONCHECK_SHORTUP(ev, BUTTON_ORANGE))
		return false;
	if (ev->events & ROTARY_EVENT && ev->rotary > 0)
	{
		GD77sSelectedCharIndex=ev->rotary-1;
		AnnounceGD77sKeypadChar(true);
	}
	else if (BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK1))
	{
		AnnounceChannelSummary(false, nonVolatileSettings.currentVFONumber > 1);
	}
	else if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK1))
	{
		AnnounceGD77sKeypadBuffer();
	}
	else if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK1))
	{
		AddGD77sKeypadChar();
	}
	else if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK2))
	{
		if (GD77sKeypadBankIndex==2)
			GD77sKeypadBankIndex=0;
		else
			GD77sKeypadBankIndex++;
		AnnounceGD77sKeypadChar(true);
	}
	else if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK2))
	{
		BackspaceGD77sKeypadChar();
	}
	else if (BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK2))
	{
		voicePromptsInit();
		ClearGD77sKeypadBuffer();
	}
	else if (BUTTONCHECK_LONGDOWN(ev, BUTTON_ORANGE))
	{
		if (ProcessGD77SKeypadCmd(ev))
		{
			if (nonVolatileSettings.currentVFONumber < 2)
			{
				memcpy(&settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, &channelScreenChannelData.rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16);// copy all channel details except name.
				codeplugSetVFO_ChannelData(&settingsVFOChannel[nonVolatileSettings.currentVFONumber], nonVolatileSettings.currentVFONumber);
			}
			GD77SParameters.virtualVFOMode=true;
			return true;
		}
		// copy first 8 digits to receive and second 8 digits to transmit.
		char rxBuf[9]="\0";
		char txBuf[9]="\0";
		int len=strlen(GD77SKeypadBuffer);
		bool copyRxToTx=len < 9;
		for (int i=0; i < 8; ++i)
		{
			if (isdigit(GD77SKeypadBuffer[i]))
				rxBuf[i]=GD77SKeypadBuffer[i];
			else
				rxBuf[i]='0';
			if (i+8 <len && isdigit(GD77SKeypadBuffer[i+8]))
				txBuf[i]=GD77SKeypadBuffer[i+8];
			else
				txBuf[i]='0';
		}			
		// now set the rx and tx freq in the current channel data.
		currentChannelData->rxFreq=atol(rxBuf);
		currentChannelData->txFreq=copyRxToTx ? currentChannelData->rxFreq : atol(txBuf);
		trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
		if (nonVolatileSettings.currentVFONumber < 2)
		{
			memcpy(&settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, &channelScreenChannelData.rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16); // copy all channel details except name.
			codeplugSetVFO_ChannelData(&settingsVFOChannel[nonVolatileSettings.currentVFONumber], nonVolatileSettings.currentVFONumber);
		}
		GD77SParameters.virtualVFOMode=true;
		voicePromptsInit();
		announceFrequency();
		voicePromptsPlay();
	}
	
	return true;
}

static void SaveGD77SAutoZoneParams()
{
	GD77SParameters.channelbankOffset = 0;

	if (nonVolatileSettings.autoZonesEnabled==0)
	{
		autoZone.flags=0;
		if (AutoZoneIsCurrentZone(currentZone.NOT_IN_CODEPLUGDATA_indexNumber))
		{
			nonVolatileSettings.currentZone = 0;
			currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
			codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
		}
	}
	settingsSetDirty();
}

static bool SetGD77S_VoiceOption(int dir) // 0 default, 1 increment, -1 decrement
{
	switch (GD77SParameters.option)
	{
		case 		GD77S_OPTION_VOLUME:
		{
			if ((dir > 0) && (nonVolatileSettings.voicePromptVolumePercent < 100))
			{
				settingsIncrement(nonVolatileSettings.voicePromptVolumePercent, 5);
			}
			else if ((dir < 0) && (nonVolatileSettings.voicePromptVolumePercent > 15))
			{
				settingsDecrement(nonVolatileSettings.voicePromptVolumePercent, 5);
			}
			else if (dir==0)
			{
				settingsSet(nonVolatileSettings.voicePromptVolumePercent, 100);
			}	
			break;
		}
		case 	GD77S_OPTION_RATE:
		{
			if ((dir > 0) && (nonVolatileSettings.voicePromptRate < 9))
			{
				settingsIncrement(nonVolatileSettings.voicePromptRate, 1);
			}
			else if ((dir < 0) && (nonVolatileSettings.voicePromptRate > 0))
			{
				settingsDecrement(nonVolatileSettings.voicePromptRate, 1);
			}
			else if (dir == 0)
			{
				settingsSet(nonVolatileSettings.voicePromptRate, 0);
			}	
			break;
		}
		case 	GD77S_OPTION_PHONETIC_SPELL:
		{
			if (dir > 0)
				settingsSetOptionBit(BIT_PHONETIC_SPELL, true);
			else
				settingsSetOptionBit(BIT_PHONETIC_SPELL, false);
			break;
		}
		case 	GD77S_OPTION_DMR_ID_ANNOUNCE:
		{
			if (dir > 0)
				settingsSetOptionBit(BIT_ANNOUNCE_LASTHEARD, true);
			else
				settingsSetOptionBit(BIT_ANNOUNCE_LASTHEARD, false);
			break;
		}
		case 	GD77S_OPTION_CUSTOM_PROMPT_RECORD:
		{
			if (dir > 0)
			{
				ReplayDMR();
			}
			else if (dir < 0)
			{
				voicePromptsEditAutoTrim();
			}
			else
			{// save to next available slot.
				GD77SParameters.customPromptIndex=GetNextFreeVoicePromptIndex(false);
				SaveCustomVoicePrompt(GD77SParameters.customPromptIndex, NULL);
			}
			return true; // do not break or the announce at the bottom will kill the replay.	
		}
		case 	GD77S_OPTION_CUSTOM_PROMPT_REVIEW:
		{
			if (dir > 0)
			{
				if (GD77SParameters.customPromptIndex < GetMaxCustomVoicePrompts())
					GD77SParameters.customPromptIndex++;
			}
			else if (dir < 0)
			{
				if (GD77SParameters.customPromptIndex > 1)
					GD77SParameters.customPromptIndex--;
			}
			else
			{
				voicePromptsCopyCustomPromptToEditBuffer(GD77SParameters.customPromptIndex);
				ReplayDMR();
				return true; // return so that announce at bottom won't clobber replay.
			}
			break;	
		}
		case 	GD77S_OPTION_EDIT_START:
		case 	GD77S_OPTION_EDIT_END:
		{
			bool editingStart=GD77SParameters.option==	GD77S_OPTION_EDIT_START;
			if (dir != 0)
			{
				if (!editingStart)
					dir=-dir; // need to switch the direction if editing the end.
				voicePromptsAdjustEnd(editingStart, dir < 0 ? -1 : 1, false);
			}
			else
			{
				voicePromptsAdjustEnd(editingStart, 0, true);
			}
			return true; // do not break or the announce at the bottom will interrupt the announcement of the edit.
		}
		default:
			return false;
	}
	AnnounceGD77sVoiceOption(false, true);
	return true;
}

static void SetGD77S_GlobalOption(int dir) // 0 default, 1 increment, -1 decrement
{
	if (SetGD77S_VoiceOption(dir))
		return;
	
	switch (GD77SParameters.option)
	{
		case GD77S_OPTION_POWER:
			if (dir > 0)
				increasePowerLevel(true, false);// true = Allow 5W++
			else if (dir < 0)
				decreasePowerLevel(false);
			else // set default.
			{
				currentChannelData->libreDMR_Power=0x00;
				nonVolatileSettings.txPowerLevel = 3u;
				trxSetPowerFromLevel(nonVolatileSettings.txPowerLevel);
			}
			break;
		case GD77S_OPTION_BAND_LIMITS:
			if (dir > 0)
			{
				if (nonVolatileSettings.txFreqLimited < BAND_LIMITS_FROM_CPS)
				{
					settingsIncrement(nonVolatileSettings.txFreqLimited, 1);
				}
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.txFreqLimited > BAND_LIMITS_NONE)
				{
					settingsDecrement(nonVolatileSettings.txFreqLimited, 1);
				}
			}
			else
			{
				nonVolatileSettings.txFreqLimited =BAND_LIMITS_NONE;
			}
			break;
		case GD77S_OPTION_HOTSPOT_MODE:
			if (dir > 0 && nonVolatileSettings.hotspotType < HOTSPOT_TYPE_BLUEDV)
				settingsIncrement(nonVolatileSettings.hotspotType, 1);
			if (dir < 0 && nonVolatileSettings.hotspotType > 0)
				settingsDecrement(nonVolatileSettings.hotspotType, 1);
			if (dir == 0)
				settingsSet(nonVolatileSettings.hotspotType, 0);
			break;
		case GD77S_OPTION_FM_MIC_GAIN:
			if (dir > 0)
			{
				if (nonVolatileSettings.micGainFM < 31)
				{
					settingsIncrement(nonVolatileSettings.micGainFM, 1);
					trxSetMicGainFM(nonVolatileSettings.micGainFM);
				}	
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.micGainFM > 1)
				{
					settingsDecrement(nonVolatileSettings.micGainFM, 1);
					trxSetMicGainFM(nonVolatileSettings.micGainFM);
				}
			}
			else
			{
				nonVolatileSettings.micGainFM = 17U; // Default (from all of my cals, datasheet default: 16)
				trxSetMicGainFM(nonVolatileSettings.micGainFM);
			}
			break;
		case GD77S_OPTION_DMR_MIC_GAIN:
			if (dir > 0)
			{
				if (nonVolatileSettings.micGainDMR < 15)
				{
					settingsIncrement(nonVolatileSettings.micGainDMR, 1);
					setMicGainDMR(nonVolatileSettings.micGainDMR);
				}	
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.micGainDMR > 0)
				{
					settingsDecrement(nonVolatileSettings.micGainDMR, 1);
					setMicGainDMR(nonVolatileSettings.micGainDMR);
				}	
			}
			else
			{
				nonVolatileSettings.micGainDMR=11u;
				setMicGainDMR(nonVolatileSettings.micGainDMR);
			}
			break;
		case GD77S_OPTION_VHF_SQUELCH:
		case GD77S_OPTION_UHF_SQUELCH:
		{
			int band=GD77SParameters.option==GD77S_OPTION_VHF_SQUELCH?RADIO_BAND_VHF : RADIO_BAND_UHF;
			if (dir > 0)
			{
				if (nonVolatileSettings.squelchDefaults[band] < CODEPLUG_MAX_VARIABLE_SQUELCH)
				{
					settingsIncrement(nonVolatileSettings.squelchDefaults[band], 1);
				}
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.squelchDefaults[band] > 1)
				{
					settingsDecrement(nonVolatileSettings.squelchDefaults[band], 1);
				}
			}
			else
			{
				nonVolatileSettings.squelchDefaults[band]=10u;
			}
			break;
		}
		case GD77S_OPTION_CSS_TAIL_ELIM:
		{
			if (dir > 0)
			{
				if (nonVolatileSettings.ctcssSqlTail < 50)
					settingsIncrement(nonVolatileSettings.ctcssSqlTail, 5);
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.ctcssSqlTail > 0)
					settingsDecrement(nonVolatileSettings.ctcssSqlTail, 5);
			}
			else
			{
				settingsSet(nonVolatileSettings.ctcssSqlTail,35);
			}
			break;
		}
		case GD77S_OPTION_FM_BEEP:
		case GD77S_OPTION_DMR_BEEP:
		{
			uint8_t dmrBeepOptions=nonVolatileSettings.beepOptions&(BEEP_TX_START | BEEP_TX_STOP); // only care about bits 0 and 1, 2 and 3 are used for fm.
			uint8_t fmBeepOptions=(nonVolatileSettings.beepOptions&(BEEP_FM_TX_START | BEEP_FM_TX_STOP))>>2;
			bool wantDmrBeep=GD77SParameters.option==GD77S_OPTION_DMR_BEEP;
			if (dir > 0)
			{
				if (wantDmrBeep)
				{
					if (dmrBeepOptions < (BEEP_TX_START | BEEP_TX_STOP))
					{
						dmrBeepOptions++;
					}
				}
				else
				{
					if (fmBeepOptions < (BEEP_TX_START | BEEP_TX_STOP)) // we shifted right by 2 for ease of manipulation and testing
					{
						fmBeepOptions++;
					}
				}
			}
			else if (dir < 0)
			{
				if (wantDmrBeep)
				{
					if (dmrBeepOptions > 0)
					{
						dmrBeepOptions--;
					}
				}
				else
				{
					if (fmBeepOptions > 0)
					{
						fmBeepOptions--;
					}
				}
			}
			else // default
			{
				fmBeepOptions=0;
				dmrBeepOptions= BEEP_TX_START | BEEP_TX_STOP;
			}
			settingsSet(nonVolatileSettings.beepOptions, ((fmBeepOptions<<2)|dmrBeepOptions));
			break;
		}
		case GD77S_OPTION_RX_END_BEEP:
			if ((dir > 0) && (nonVolatileSettings.endRXBeep < END_RX_BEEP_BOTH))
			{
				settingsIncrement(nonVolatileSettings.endRXBeep, 1);
			}
			else if ((dir < 0) && (nonVolatileSettings.endRXBeep > END_RX_BEEP_OFF))
			{
				settingsDecrement(nonVolatileSettings.endRXBeep, 1);
			}
			else if (dir ==0)
			{
				settingsSet(nonVolatileSettings.endRXBeep, 0);
			}
			break;
		case GD77S_OPTION_DTMF_VOL:
		{
			if (dir > 0)
			{
				if (nonVolatileSettings.dtmfVol < 10)
					settingsIncrement(nonVolatileSettings.dtmfVol, 1);
			}
			else  if (dir < 0)
			{
				if (nonVolatileSettings.dtmfVol > 0)
					settingsDecrement(nonVolatileSettings.dtmfVol, 1);
			}
			else // default
			{
				settingsSet(nonVolatileSettings.dtmfVol, 10);
			}
			break;
		}
		case GD77S_OPTION_TOT_MASTER:
			if (dir > 0)
			{
				if (nonVolatileSettings.totMaster < 255)
				{
					settingsIncrement(nonVolatileSettings.totMaster,1);
				}
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.totMaster > 0)
				{
					settingsDecrement(nonVolatileSettings.totMaster,1);
				}
			}
			else
			{
				nonVolatileSettings.totMaster=0;
			}
			break;
		case GD77S_OPTION_PTT_LATCH:
			if (dir > 0)
			{
				if (settingsIsOptionBitSet(BIT_PTT_LATCH) == false)
				{
					settingsSetOptionBit(BIT_PTT_LATCH, true);
				}
			}
			else if (dir < 0)
			{
				if (settingsIsOptionBitSet(BIT_PTT_LATCH))
				{
					settingsSetOptionBit(BIT_PTT_LATCH, false);
				}
			}
			else
			{
				settingsSetOptionBit(BIT_PTT_LATCH, false);
			}
			break;
		case GD77S_OPTION_ECO:
			if (dir > 0)
			{
				if (nonVolatileSettings.ecoLevel < ECO_LEVEL_MAX)
				{
					settingsIncrement(nonVolatileSettings.ecoLevel, 1);
					rxPowerSavingSetLevel(nonVolatileSettings.ecoLevel);
				}
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.ecoLevel > 0)
				{
					settingsDecrement(nonVolatileSettings.ecoLevel, 1);
					rxPowerSavingSetLevel(nonVolatileSettings.ecoLevel);
				}
			}
			else
			{
				nonVolatileSettings.ecoLevel = 1;
				rxPowerSavingSetLevel(nonVolatileSettings.ecoLevel);
			}
			break;
		case GD77S_OPTION_FIRMWARE_INFO:
			break;
		case GD77S_OPTION_AUTOZONE:
			if (dir > 0)
			{
				if (GD77SParameters.autoZoneTypeIndex < AutoZone_TYPE_MAX-1)
					GD77SParameters.autoZoneTypeIndex++;
				else
					GD77SParameters.autoZoneTypeIndex=1;
			}
			else if (dir < 0)
			{
				bool isBitSet=IsBitSet(nonVolatileSettings.autoZonesEnabled, GD77SParameters.autoZoneTypeIndex);
				SetBit(&nonVolatileSettings.autoZonesEnabled, GD77SParameters.autoZoneTypeIndex, !isBitSet);
			}
			else
			{
				nonVolatileSettings.autoZonesEnabled=0;
			}
			SaveGD77SAutoZoneParams();
			break;
		case 	GD77S_OPTION_VOX_THRESHOLD:
			if (dir > 0)
			{
				if (nonVolatileSettings.voxThreshold < 30)
				{
					settingsIncrement(nonVolatileSettings.voxThreshold, 1);
					voxSetParameters(nonVolatileSettings.voxThreshold, nonVolatileSettings.voxTailUnits);
				}
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.voxThreshold > 2)
				{
					settingsDecrement(nonVolatileSettings.voxThreshold, 1);
					voxSetParameters(nonVolatileSettings.voxThreshold, nonVolatileSettings.voxTailUnits);
				}
			}
			else
			{
				nonVolatileSettings.voxThreshold = 20U;
				voxSetParameters(nonVolatileSettings.voxThreshold, nonVolatileSettings.voxTailUnits);
			}
			break;
		case GD77S_OPTION_VOX_TAIL:
			if (dir > 0)
			{
				if (nonVolatileSettings.voxTailUnits < 10) // 5 seconds max
				{
					settingsIncrement(nonVolatileSettings.voxTailUnits, 1);
					voxSetParameters(nonVolatileSettings.voxThreshold, nonVolatileSettings.voxTailUnits);
				}
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.voxTailUnits > 1) // .5 minimum
				{
					settingsDecrement(nonVolatileSettings.voxTailUnits, 1);
					voxSetParameters(nonVolatileSettings.voxThreshold, nonVolatileSettings.voxTailUnits);
				}
			}
			else
			{
				nonVolatileSettings.voxTailUnits = 4U; // 2 seconds tail
				voxSetParameters(nonVolatileSettings.voxThreshold, nonVolatileSettings.voxTailUnits);
			}
			break;
		case GD77S_OPTION_KEYPAD_SOURCE:
			if (dir > 0)
			{
				if (nonVolatileSettings.currentVFONumber < 2)
					settingsIncrement(nonVolatileSettings.currentVFONumber, 1);
			}
			else if (dir < 0)
			{
				if (nonVolatileSettings.currentVFONumber > 0)
					settingsDecrement(nonVolatileSettings.currentVFONumber, 1);
			}
			else
				settingsSet(nonVolatileSettings.currentVFONumber, 2); // current channel.
			if (nonVolatileSettings.currentVFONumber < 2)
			{
				memcpy(&channelScreenChannelData.rxFreq, &settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16); // Don't copy the name of the vfo, which is in the first 16 bytes
			}
			else
			{// recall the last known channel prior to going into Options
				codeplugChannelGetDataForIndex(currentZone.channels[nonVolatileSettings.currentChannelIndexInZone], &channelScreenChannelData);
			}
			loadChannelData(true, false);
			break;
		case GD77S_OPTION_MAX:
			return;
	};
	AnnounceGD77SOption(false, true);
}

static bool HandleGD77sOptionEvent(uiEvent_t *ev)
{
	if ((GD77SParameters.uiMode!=GD77S_UIMODE_OPTIONS1) && (GD77SParameters.uiMode!=GD77S_UIMODE_OPTIONS2))
		return false;
	if (GD77SParameters.cycleFunctionsInReverse)
		return false;
	if (BUTTONCHECK_SHORTUP(ev, BUTTON_ORANGE))
	{
		if (GD77SParameters.uiMode == GD77S_UIMODE_OPTIONS2)
		{
			if (voicePromptsGetEditMode())
				voicePromptsSetEditMode(false);
			encodingCustomVoicePrompt = false;

			if (BUTTONCHECK_DOWN(ev, BUTTON_SK1))
				return false; // we're cycling in reverse, let the main handler handle this.
			GD77SParameters.uiMode=GD77S_UIMODE_ZONE;
			checkAndUpdateSelectedChannelForGD77S(ev->rotary+GD77SParameters.channelbankOffset, false);
			keyboardReset();
			voicePromptsInit();
			voicePromptsAppendPrompt(PROMPT_ZONE_MODE);
			voicePromptsAppendPrompt(PROMPT_CHANNEL);
			voicePromptsAppendInteger(ev->rotary+GD77SParameters.channelbankOffset);
			voicePromptsPlay();
			return true;
		}
		return false; // let main handler increment.
	}
	if (ev->events & ROTARY_EVENT && ev->rotary > 0)
	{
		GD77SParameters.option=ev->rotary-1;
		if (GD77SParameters.uiMode==GD77S_UIMODE_OPTIONS2)
			GD77SParameters.option+=16;
		// See if we should enable or disable voice tag edit mode.
		bool voiceTagEditMode = ((GD77SParameters.uiMode == GD77S_UIMODE_OPTIONS2) && ((GD77SParameters.option >=	GD77S_OPTION_EDIT_START) && (GD77SParameters.option <=	GD77S_OPTION_EDIT_END)));
		if (voiceTagEditMode != voicePromptsGetEditMode())
			voicePromptsSetEditMode(voiceTagEditMode);
		// See if we should turn on decode mode where we record a voice prompt on PTT but do not tx.
		encodingCustomVoicePrompt = ((GD77SParameters.uiMode == GD77S_UIMODE_OPTIONS2) && (GD77SParameters.option == 	GD77S_OPTION_CUSTOM_PROMPT_RECORD));

		AnnounceGD77SOption(true, true); // announce after setting edit mode as this prompt should trump the edit mode change prompt.

		return true;
	}
	if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK1))
	{
		// special case for editing a voice tag, replayDMR instead.
		if ((GD77SParameters.uiMode == GD77S_UIMODE_OPTIONS2) && (GD77SParameters.option >=	GD77S_OPTION_CUSTOM_PROMPT_RECORD && GD77SParameters.option <=	GD77S_OPTION_EDIT_END))
			ReplayDMR();
		else
			AnnounceGD77SOption(true, true);  // repeat the current option and its value.
		return true;
	}
	else if (BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK1))
	{
		bool keypadIsInChannelMode=((GD77SParameters.option!=GD77S_OPTION_KEYPAD_SOURCE) || (nonVolatileSettings.currentVFONumber > 1));
		AnnounceChannelSummary(false, keypadIsInChannelMode);
		return true;
	}
	else if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK2))
	{
		SetGD77S_GlobalOption(0); // 0 means default.
		keyboardReset();

		return true;
	}
	else if (BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK2))
	{// do nothing.
		keyboardReset();

		return true;
	}
	else if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK1))
	{
		SetGD77S_GlobalOption(1); // 1 means increment if possible.
		return true;
	}
	else if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK2))
	{
		SetGD77S_GlobalOption(-1); // -1 means decrement if possible.
		return true;
	}
	
	return false;
}

static void 						ToggleGD77SVFOMode(uiEvent_t *ev)
{
			GD77SParameters.virtualVFOMode=!GD77SParameters.virtualVFOMode;
	voicePromptsTerminate();
	if (GD77SParameters.virtualVFOMode && nonVolatileSettings.currentVFONumber < 2)
	{
		memcpy(&channelScreenChannelData, &settingsVFOChannel[nonVolatileSettings.currentVFONumber < 2], CODEPLUG_CHANNEL_DATA_STRUCT_SIZE );
		loadChannelData(true, false);
		voicePromptsInit();
		voicePromptsAppendString("vfo");
		announceFrequency();
		voicePromptsPlay();
	}
	else
	{
		loadChannelData(false, false);
		announceItem(PROMPT_SEQUENCE_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE, PROMPT_THRESHOLD_2);
	}	
}

static void handleEventForGD77S(uiEvent_t *ev)
{
	if (HandleGD77sKbdEvent(ev))
		return;
	if (HandleGD77sOptionEvent(ev))
		return;
	
	if (ev->events & ROTARY_EVENT)
	{
		if (dtmfSequenceIsKeying())
		{
			dtmfSequenceStop();
		}

		if (!trxTransmissionEnabled && (ev->rotary > 0))
		{
			if (uiDataGlobal.Scan.active)
			{
				uiChannelModeStopScanning();
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);
			}

			settingsSet(nonVolatileSettings.overrideTG, 0);
			checkAndUpdateSelectedChannelForGD77S(ev->rotary+GD77SParameters.channelbankOffset, false);
			clearActiveDMRID();
			lastHeardClearLastID();
		}
		// If rotary knob is turned, immediately reset to channel mode in any mode except keypad.
		// This is a quick shortcut to reset to the channel mode.
		if (!ModeUsesChannelKnob())
		{
			if (GD77SParameters.uiMode!=GD77S_UIMODE_TG_OR_SQUELCH)
			{
				GD77SParameters.uiMode=GD77S_UIMODE_TG_OR_SQUELCH;
				GD77SParameters.virtualVFOMode=false;
				voicePromptsInit();
				voicePromptsAppendPrompt(PROMPT_CHANNEL_MODE);
				voicePromptsAppendPrompt(PROMPT_CHANNEL);
				voicePromptsAppendInteger(ev->rotary+GD77SParameters.channelbankOffset);
				voicePromptsPlay();
			}
		}
	}

	if (handleMonitorMode(ev))
	{
		return;
	}

	if (ev->events & BUTTON_EVENT)
	{
		if (dtmfSequenceIsKeying() && (ev->buttons & (BUTTON_SK1 | BUTTON_SK2 | BUTTON_ORANGE)))
		{
			dtmfSequenceStop();
		}

		if (BUTTONCHECK_SHORTUP(ev, BUTTON_ORANGE) && uiDataGlobal.Scan.active)
		{
			uiChannelModeStopScanning();
			uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
			uiChannelModeUpdateScreen(0);

			voicePromptsInit();
			buildSpeechUiModeForGD77S(GD77S_UIMODE_SCAN);
			voicePromptsPlay();
			return;
		}
if (GD77SParameters.cycleFunctionsInReverse && BUTTONCHECK_DOWN(ev, BUTTON_SK1)==0 && BUTTONCHECK_DOWN(ev, BUTTON_ORANGE)==0)
{
	GD77SParameters.cycleFunctionsInReverse=false;
	return;
}
		if (BUTTONCHECK_LONGDOWN(ev, BUTTON_ORANGE) && (uiDataGlobal.DTMFContactList.isKeying == false))
		{
			announceItem(PROMPT_SEQUENCE_BATTERY, PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY);// next call will call play for immediate announcement.
			announceItemWithInit(false, PROMPT_SEQUENCE_TEMPERATURE, PROMPT_THRESHOLD_2);
			return;
		}
		else if (BUTTONCHECK_SHORTUP(ev, BUTTON_ORANGE) && (uiDataGlobal.DTMFContactList.isKeying == false))
		{
			GD77SParameters.cycleFunctionsInReverse =BUTTONCHECK_DOWN(ev, BUTTON_SK1);
			voicePrompt_t vp = NUM_VOICE_PROMPTS;
			char * const *vpString = NULL;
			if (GD77SParameters.cycleFunctionsInReverse)
				GD77SParameters.uiMode = (GD77SParameters.uiMode > 0) ? (GD77SParameters.uiMode - 1) : (GD77S_UIMODE_MAX-1);
			else
				GD77SParameters.uiMode = (GD77S_UIMODES_t) (GD77SParameters.uiMode + 1) % GD77S_UIMODE_MAX;
			GD77SParameters.virtualVFOMode=false;
			//skip over Digital controls if the radio is in Analog mode
			if (trxGetMode() == RADIO_MODE_ANALOG)
			{
				// Analog
				if ((GD77SParameters.uiMode == GD77S_UIMODE_TS) ||
					(GD77SParameters.uiMode == GD77S_UIMODE_CC))
				{
					GD77SParameters.uiMode = GD77SParameters.cycleFunctionsInReverse ? GD77S_UIMODE_TS-1 :GD77S_UIMODE_CC+1;
				}
			}
			else
			{
				// digital
				if (GD77SParameters.uiMode == GD77S_UIMODE_DTMF_CONTACTS)
				{
					GD77SParameters.uiMode =GD77SParameters.cycleFunctionsInReverse ? GD77S_UIMODE_DTMF_CONTACTS-1:GD77S_UIMODE_DTMF_CONTACTS+1;
				}
			}

			switch (GD77SParameters.uiMode)
			{
				case GD77S_UIMODE_TG_OR_SQUELCH:
				{
					if (trxGetMode() == RADIO_MODE_ANALOG)
						vpString=(char * const *)&currentLanguage->squelch;
					else 
						vp =PROMPT_TALKGROUP;
					break;
				}

				case GD77S_UIMODE_SCAN:
					vp = PROMPT_CHANNEL_MODE;
					break;
				case GD77S_UIMODE_TS: // Timeslot Mode
					vp = PROMPT_TIMESLOT_MODE;
					break;

				case GD77S_UIMODE_CC: // ColorCode Mode
					vp = PROMPT_COLORCODE_MODE;
					break;

				case GD77S_UIMODE_FILTER: // DMR/Analog Filter
					vp = PROMPT_FILTER_MODE;
					break;

				case GD77S_UIMODE_DTMF_CONTACTS:
					vpString = (char * const *)&currentLanguage->dtmf_contact_list;
					break;

				case GD77S_UIMODE_ZONE: // Zone Mode
					vp = PROMPT_ZONE_MODE;
					break;
				case GD77S_UIMODE_OPTIONS1:
					vpString = (char * const *)&currentLanguage->options;
					GD77SParameters.option=rotarySwitchGetPosition()-1;
					break;
				case GD77S_UIMODE_OPTIONS2:
					vpString = (char * const *)&currentLanguage->options;
					GD77SParameters.option=(rotarySwitchGetPosition()-1)+16;
					break;
				case GD77S_UIMODE_KEYPAD:
					vpString = (char * const *)&currentLanguage->keypad;
					GD77sSelectedCharIndex = rotarySwitchGetPosition()-1;
					if (nonVolatileSettings.currentVFONumber < 2)
					{
						memcpy(&channelScreenChannelData.rxFreq, &settingsVFOChannel[nonVolatileSettings.currentVFONumber].rxFreq, CODEPLUG_CHANNEL_DATA_STRUCT_SIZE - 16); // Don't copy the name of the vfo, which is in the first 16 bytes
						loadChannelData(true, false);
					}
					break;
				case GD77S_UIMODE_VOX:
					vp = PROMPT_VOX;
					break;
				case GD77S_UIMODE_MAX:
					break;
			}

			if ((vp != NUM_VOICE_PROMPTS) || (vpString != NULL))
			{
				voicePromptsInit();
				if (vpString)
				{
					voicePromptsAppendLanguageString((const char * const *)vpString);
					if ((GD77SParameters.uiMode==GD77S_UIMODE_OPTIONS1) || (GD77SParameters.uiMode==GD77S_UIMODE_OPTIONS2))
						voicePromptsAppendInteger(GD77SParameters.uiMode-GD77S_UIMODE_OPTIONS1+1);
					voicePromptsAppendPrompt(PROMPT_MODE);
				}
				else
				{
					voicePromptsAppendPrompt(vp);
				}
				voicePromptsAppendPrompt(PROMPT_SILENCE);
				buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
				voicePromptsPlay();
			}
		}
		else if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK1) && (monitorModeData.isEnabled == false) && (uiDataGlobal.DTMFContactList.isKeying == false) && !GD77SParameters.cycleFunctionsInReverse)
		{
			if (GD77SParameters.channelOutOfBounds == false)
			{
				AnnounceChannelSummary((nonVolatileSettings.audioPromptMode <= AUDIO_PROMPT_MODE_VOICE_LEVEL_2), true);
			}
		}
		else if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK1) && (uiDataGlobal.DTMFContactList.isKeying == false) && !GD77SParameters.cycleFunctionsInReverse)
		{
			switch (GD77SParameters.uiMode)
			{
				case GD77S_UIMODE_TG_OR_SQUELCH:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						// Next in TGList
						if (nonVolatileSettings.overrideTG == 0)
						{
							settingsIncrement(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 1);
							if (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] > (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup - 1))
							{
								settingsSet(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 0);
							}
						}
						settingsSet(nonVolatileSettings.overrideTG, 0);// setting the override TG to 0 indicates the TG is not overridden
						menuPrivateCallClear();
						updateTrxID();
						uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
						uiChannelModeUpdateScreen(0);
						announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC, PROMPT_THRESHOLD_2);
					}
					else
					{
						if(currentChannelData->sql == 0)			//If we were using default squelch level
						{
							currentChannelData->sql = nonVolatileSettings.squelchDefaults[trxCurrentBand[TRX_RX_FREQ_BAND]];			//start the adjustment from that point.
						}

						if (currentChannelData->sql < CODEPLUG_MAX_VARIABLE_SQUELCH)
						{
							currentChannelData->sql++;
						}

						announceItem(PROMPT_SQUENCE_SQUELCH, PROMPT_THRESHOLD_2);
					}
					break;

				case GD77S_UIMODE_SCAN:
					if (uiDataGlobal.Scan.active)
					{
						if (uiDataGlobal.Scan.scanAllZones==false)
						{
							uiDataGlobal.Scan.scanAllZones=true;
							announceItem(PROMPT_SEQUENCE_SCAN_TYPE, PROMPT_THRESHOLD_2);
							return;
						}
						uiChannelModeStopScanning();
						uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
						uiChannelModeUpdateScreen(0);
					}
					else
					{
						if (GD77SParameters.virtualVFOMode==false || InitGD77SScan())
							scanStart(false);
					}

					voicePromptsInit();
					voicePromptsAppendLanguageString(&currentLanguage->scan);
					voicePromptsAppendLanguageString(uiDataGlobal.Scan.active ? &currentLanguage->start : &currentLanguage->stop);
					voicePromptsPlay();
					break;

				case GD77S_UIMODE_TS:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						toggleTimeslotForGD77S();
						announceItem(PROMPT_SEQUENCE_TS, PROMPT_THRESHOLD_2);
					}
					break;

				case GD77S_UIMODE_CC:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						if (currentChannelData->rxColor < 15)
						{
							currentChannelData->rxColor++;
							currentChannelData->txColor=currentChannelData->rxColor;
							trxSetDMRColourCode(currentChannelData->rxColor);
						}

						voicePromptsInit();
						announceCC();
						voicePromptsPlay();
					}
					break;

				case GD77S_UIMODE_FILTER:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						if (nonVolatileSettings.dmrDestinationFilter < NUM_DMR_DESTINATION_FILTER_LEVELS - 1)
						{
							settingsIncrement(nonVolatileSettings.dmrDestinationFilter, 1);
							init_digital_DMR_RX();
							disableAudioAmp(AUDIO_AMP_MODE_RF);
						}
					}
					else
					{
						if (nonVolatileSettings.analogFilterLevel < NUM_ANALOG_FILTER_LEVELS - 1)
						{
							settingsIncrement(nonVolatileSettings.analogFilterLevel, 1);
							trxSetAnalogFilterLevel(nonVolatileSettings.analogFilterLevel);
						}
					}

					voicePromptsInit();
					buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
					voicePromptsPlay();
					break;

				case GD77S_UIMODE_DTMF_CONTACTS:
					// select next DTMF contact and spell it
					if (GD77SParameters.dtmfListCount > 0)
					{
						GD77SParameters.dtmfListSelected = (GD77SParameters.dtmfListSelected + 1) % GD77SParameters.dtmfListCount;
					}
					voicePromptsInit();
					buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
					voicePromptsPlay();
					break;

				case GD77S_UIMODE_ZONE: // Zones
					// No "All Channels" on GD77S
					nonVolatileSettings.currentZone = (nonVolatileSettings.currentZone + 1) % (codeplugZonesGetCount() - 1);

					settingsSet(nonVolatileSettings.overrideTG, 0); // remove any TG override
					tsSetManualOverride(CHANNEL_CHANNEL, TS_NO_OVERRIDE);
					settingsSet(nonVolatileSettings.currentChannelIndexInZone, (int16_t) -2); // Will be updated when reloading the UiChannelMode screen
					currentChannelData->rxFreq = 0x00; // Flag to the Channel screen that the channel data is now invalid and needs to be reloaded
					codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
					GD77SParameters.channelbankOffset =0; // reset this to avoid a possible channel out of range when switching zones.
					menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
					checkAndUpdateSelectedChannelForGD77S(rotarySwitchGetPosition()+GD77SParameters.channelbankOffset, false);
					GD77SParameters.uiMode = GD77S_UIMODE_ZONE;

					announceItem(PROMPT_SEQUENCE_ZONE, PROMPT_THRESHOLD_2);
					break;
				case GD77S_UIMODE_KEYPAD:
					break; // handled by separate handler.
				case GD77S_UIMODE_VOX:
					if ((currentChannelData->flag4 & 0x40)==0)
						currentChannelData->flag4 |=0x40;
					voicePromptsInit();
					buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
					voicePromptsPlay();
					break;
				case GD77S_UIMODE_OPTIONS1:
				case GD77S_UIMODE_OPTIONS2:
					break; // handled by its own handler. 
				case GD77S_UIMODE_MAX:
					break;
			}
		}
		else if (AutoZoneIsCurrentZone(currentZone.NOT_IN_CODEPLUGDATA_indexNumber) && (autoZone.flags & AutoZoneDuplexAvailable) && BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK1) && (monitorModeData.isEnabled == false) && (uiDataGlobal.DTMFContactList.isKeying == false))
		{
			CycleRepeaterOffset(NULL);
		}
		else if (BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK2) && (monitorModeData.isEnabled == false) && (uiDataGlobal.DTMFContactList.isKeying == false) && !GD77SParameters.virtualVFOMode && (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 16))
		{
			if (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone > 16 && rotarySwitchGetPosition()+GD77SParameters.channelbankOffset <= (currentZone.NOT_IN_CODEPLUGDATA_numChannelsInZone-16))
				GD77SParameters.channelbankOffset+=16;
			else
				GD77SParameters.channelbankOffset=0;
			checkAndUpdateSelectedChannelForGD77S(rotarySwitchGetPosition()+GD77SParameters.channelbankOffset, true);
		}
		else if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK2) && (monitorModeData.isEnabled == false) && (uiDataGlobal.DTMFContactList.isKeying == false))
		{
			uint32_t tg = (LinkHead->talkGroupOrPcId & 0xFFFFFF);

			// If Blue button is long pressed during reception it sets the Tx TG to the incoming TG
			if (uiDataGlobal.isDisplayingQSOData && BUTTONCHECK_DOWN(ev, BUTTON_SK2) && (trxGetMode() == RADIO_MODE_DIGITAL) &&
					((trxTalkGroupOrPcId != tg) ||
							((dmrMonitorCapturedTS != -1) && (dmrMonitorCapturedTS != trxGetDMRTimeSlot())) ||
							(trxGetDMRColourCode() != currentChannelData->rxColor)))
			{
				lastHeardClearLastID();

				// Set TS to overriden TS
				if ((dmrMonitorCapturedTS != -1) && (dmrMonitorCapturedTS != trxGetDMRTimeSlot()))
				{
					trxSetDMRTimeSlot(dmrMonitorCapturedTS);
					tsSetManualOverride(CHANNEL_CHANNEL, (dmrMonitorCapturedTS + 1));
				}
				if (trxTalkGroupOrPcId != tg)
				{
					if ((tg >> 24) & PC_CALL_FLAG)
					{
						acceptPrivateCall(tg & 0xffffff, -1);
					}
					else
					{
						trxTalkGroupOrPcId = tg;
						settingsSet(nonVolatileSettings.overrideTG, trxTalkGroupOrPcId);
					}
				}

				currentChannelData->rxColor = trxGetDMRColourCode();// Set the CC to the current CC, which may have been determined by the CC finding algorithm in C6000.c

				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				uiChannelModeUpdateScreen(0);

				voicePromptsInit();
				voicePromptsAppendLanguageString(&currentLanguage->select_tx);
				voicePromptsPlay();
				return;
			}
			else
			{
				if (trxGetMode() == RADIO_MODE_DIGITAL)
				{
					ReplayDMR();
					return;
				}
				else
				{
					// toggle  autodialer for this zone and channel combination for analog.
					ToggleGD77SDTMFAutoDialer(true);
					return;
				}
			}
		}
		else if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK2) && (uiDataGlobal.DTMFContactList.isKeying == false))
		{
			switch (GD77SParameters.uiMode)
			{
				case GD77S_UIMODE_TG_OR_SQUELCH: // Previous in TGList
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						// To Do change TG in on same channel freq
						if (nonVolatileSettings.overrideTG == 0)
						{
							settingsDecrement(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], 1);
							if (nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE] < 0)
							{
								settingsSet(nonVolatileSettings.currentIndexInTRxGroupList[SETTINGS_CHANNEL_MODE], (int16_t) (currentRxGroupData.NOT_IN_CODEPLUG_numTGsInGroup - 1));
							}
						}
						settingsSet(nonVolatileSettings.overrideTG, 0);// setting the override TG to 0 indicates the TG is not overridden
						menuPrivateCallClear();
						updateTrxID();
						uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
						uiChannelModeUpdateScreen(0);
						announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC, PROMPT_THRESHOLD_2);
					}
					else
					{
						if(currentChannelData->sql == 0)			//If we were using default squelch level
						{
							currentChannelData->sql = nonVolatileSettings.squelchDefaults[trxCurrentBand[TRX_RX_FREQ_BAND]];			//start the adjustment from that point.
						}

						if (currentChannelData->sql > CODEPLUG_MIN_VARIABLE_SQUELCH)
						{
							currentChannelData->sql--;
						}

						announceItem(PROMPT_SQUENCE_SQUELCH, PROMPT_THRESHOLD_2);
					}
					break;

				case GD77S_UIMODE_SCAN:
					if (uiDataGlobal.Scan.active)
					{
						// if we are scanning and down key is pressed then enter current channel into nuisance delete array.
						if(uiDataGlobal.Scan.state == SCAN_PAUSED)
						{
							AddFrequencyToNuisanceList(currentChannelData->rxFreq);

							return;
						}

						// Left key reverses the scan direction
						if (uiDataGlobal.Scan.state == SCAN_SCANNING)
						{
							uiDataGlobal.Scan.direction *= -1;
							return;
						}
					}
					else
					{
						ToggleGD77SVFOMode(ev);
						return;
					}
					break;

				case GD77S_UIMODE_TS:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						toggleTimeslotForGD77S();
						announceItem(PROMPT_SEQUENCE_TS, PROMPT_THRESHOLD_2);
					}
					break;

				case GD77S_UIMODE_CC:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						if (currentChannelData->rxColor > 0)
						{
							currentChannelData->rxColor--;
							currentChannelData->txColor=currentChannelData->rxColor;
							trxSetDMRColourCode(currentChannelData->rxColor);
						}

						voicePromptsInit();
						announceCC();
						voicePromptsPlay();
					}
					break;

				case GD77S_UIMODE_FILTER:
					if (trxGetMode() == RADIO_MODE_DIGITAL)
					{
						if (nonVolatileSettings.dmrDestinationFilter > DMR_DESTINATION_FILTER_NONE)
						{
							settingsDecrement(nonVolatileSettings.dmrDestinationFilter, 1);
							init_digital_DMR_RX();
							disableAudioAmp(AUDIO_AMP_MODE_RF);
						}
					}
					else
					{
						if (nonVolatileSettings.analogFilterLevel > ANALOG_FILTER_NONE)
						{
							settingsDecrement(nonVolatileSettings.analogFilterLevel, 1);
							trxSetAnalogFilterLevel(nonVolatileSettings.analogFilterLevel);
						}
					}

					voicePromptsInit();
					buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
					voicePromptsPlay();
					break;

				case GD77S_UIMODE_DTMF_CONTACTS:
					// select previous DTMF contact and spell it
					if (GD77SParameters.dtmfListCount > 0)
					{
						GD77SParameters.dtmfListSelected = (GD77SParameters.dtmfListSelected + GD77SParameters.dtmfListCount - 1) % GD77SParameters.dtmfListCount;
					}
					voicePromptsInit();
					buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
					voicePromptsPlay();
					break;

				case GD77S_UIMODE_ZONE: // Zones
					// No "All Channels" on GD77S
					nonVolatileSettings.currentZone = (nonVolatileSettings.currentZone + (codeplugZonesGetCount() - 1) - 1) % (codeplugZonesGetCount() - 1);

					settingsSet(nonVolatileSettings.overrideTG, 0); // remove any TG override
					tsSetManualOverride(CHANNEL_CHANNEL, TS_NO_OVERRIDE);
					settingsSet(nonVolatileSettings.currentChannelIndexInZone, (int16_t) -2); // Will be updated when reloading the UiChannelMode screen
					currentChannelData->rxFreq = 0x00; // Flag to the Channel screeen that the channel data is now invalid and needs to be reloaded
					codeplugZoneGetDataForNumber(nonVolatileSettings.currentZone, &currentZone);
					GD77SParameters.channelbankOffset =0; // reset this to avoid a possible channel out of range when switching zones.
					menuSystemPopAllAndDisplaySpecificRootMenu(UI_CHANNEL_MODE, true);
					checkAndUpdateSelectedChannelForGD77S(rotarySwitchGetPosition()+GD77SParameters.channelbankOffset, false);
					GD77SParameters.uiMode = GD77S_UIMODE_ZONE;

					announceItem(PROMPT_SEQUENCE_ZONE, PROMPT_THRESHOLD_2);
					break;
				case GD77S_UIMODE_KEYPAD:
					break;// handled by separate handler
				case GD77S_UIMODE_VOX:
					if (currentChannelData->flag4 & 0x40)
						currentChannelData->flag4 &=~0x40;
					voicePromptsInit();
					buildSpeechUiModeForGD77S(GD77SParameters.uiMode);
					voicePromptsPlay();
					break;
				case GD77S_UIMODE_OPTIONS1:
				case GD77S_UIMODE_OPTIONS2:
					break; // handled by its own handler. 
				case GD77S_UIMODE_MAX:
					break;
			}
		}
	}
}
#endif // PLATFORM_GD77S
