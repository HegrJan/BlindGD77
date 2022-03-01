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
#ifndef _OPENGD77_UIUTILITIES_H_
#define _OPENGD77_UIUTILITIES_H_

#include "user_interface/uiGlobals.h"
#include "user_interface/menuSystem.h"
#include "functions/settings.h"


#define COMPUTE_BUILD_YEAR \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )


#define COMPUTE_BUILD_DAY \
    ( \
        ((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + \
        (__DATE__[5] - '0') \
    )


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define COMPUTE_BUILD_MONTH \
    ( \
        (BUILD_MONTH_IS_JAN) ?  1 : \
        (BUILD_MONTH_IS_FEB) ?  2 : \
        (BUILD_MONTH_IS_MAR) ?  3 : \
        (BUILD_MONTH_IS_APR) ?  4 : \
        (BUILD_MONTH_IS_MAY) ?  5 : \
        (BUILD_MONTH_IS_JUN) ?  6 : \
        (BUILD_MONTH_IS_JUL) ?  7 : \
        (BUILD_MONTH_IS_AUG) ?  8 : \
        (BUILD_MONTH_IS_SEP) ?  9 : \
        (BUILD_MONTH_IS_OCT) ? 10 : \
        (BUILD_MONTH_IS_NOV) ? 11 : \
        (BUILD_MONTH_IS_DEC) ? 12 : \
        /* error default */  99 \
    )

#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')


#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')

#define BUILD_YEAR  ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY   ((BUILD_DATE_IS_BAD) ? 99 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')

#define BUILD_HOUR  ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC   ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])


#if defined(PLATFORM_GD77S)
#define ANNOUNCE_STATIC
#else
#define ANNOUNCE_STATIC static
#endif

#if defined(PLATFORM_GD77S)
// Displaying ANNOUNCE_STATIC here is a bit overkill but it makes things clearer.
ANNOUNCE_STATIC void announceRadioMode(bool voicePromptWasPlaying);
ANNOUNCE_STATIC void announceZoneName(bool voicePromptWasPlaying);
ANNOUNCE_STATIC void announceContactNameTgOrPc(bool voicePromptWasPlaying);
ANNOUNCE_STATIC void announcePowerLevel(bool voicePromptWasPlaying);
void announceEcoLevel(bool voicePromptWasPlaying);
ANNOUNCE_STATIC void announceBatteryPercentage(void);
ANNOUNCE_STATIC void announceTS(void);
ANNOUNCE_STATIC void announceCC(void);
ANNOUNCE_STATIC void announceChannelName(bool voicePromptWasPlaying);
ANNOUNCE_STATIC void announceFrequency(void);
ANNOUNCE_STATIC void announceVFOChannelName(void);
ANNOUNCE_STATIC void announceVFOAndFrequency(bool announceVFOName);
ANNOUNCE_STATIC void announceSquelchLevel(bool voicePromptWasPlaying);

#endif

typedef enum
{
	DIRECTION_NONE = 0U,
	DIRECTION_TRANSMIT,
	DIRECTION_RECEIVE
} Direction_t;

typedef enum
{
	PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ_AND_MODE,
	PROMPT_SEQUENCE_ZONE_NAME_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE,
	PROMPT_SEQUENCE_ZONE_NAME_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE_AND_TS_AND_CC,
	PROMPT_SEQUENCE_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE_AND_TS_AND_CC,
	PROMPT_SEQUENCE_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE,
	PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ,
	PROMPT_SEQUENCE_VFO_FREQ_UPDATE,
	PROMPT_SEQUENCE_ZONE,
	PROMPT_SEQUENCE_MODE,
	PROMPT_SEQUENCE_CONTACT_TG_OR_PC,
	PROMPT_SEQUENCE_TS,
	PROMPT_SEQUENCE_CC,
	PROMPT_SEQUENCE_POWER,
	PROMPT_SEQUENCE_BATTERY,
	PROMPT_SQUENCE_SQUELCH,
	PROMPT_SEQUENCE_TEMPERATURE,
	PROMPT_SEQUENCE_DIRECTION_TX,
	PROMPT_SEQUENCE_DIRECTION_RX,
	NUM_PROMPT_SEQUENCES
} voicePromptItem_t;

typedef enum
{
	DISPLAY_INFO_CONTACT_INVERTED = 0U,
	DISPLAY_INFO_CONTACT,
	DISPLAY_INFO_CONTACT_OVERRIDE_FRAME,
	DISPLAY_INFO_CHANNEL,
	DISPLAY_INFO_TONE_AND_SQUELCH,
	DISPLAY_INFO_TX_TIMER,
	DISPLAY_INFO_ZONE
} displayInformation_t;

typedef struct
{
	uint32_t			entries;
	uint8_t				contactLength;
	uint32_t			slices[ID_SLICES]; // [0] is min availabel ID, [ID_SLICES - 1] is max available ID
	uint32_t			IDsPerSlice;
} dmrIDsCache_t;


#define TS_NO_OVERRIDE  0
void tsSetManualOverride(Channel_t chan, int8_t ts);
void tsSetFromContactOverride(Channel_t chan, struct_codeplugContact_t *contact);
int8_t tsGetManualOverride(Channel_t chan);
int8_t tsGetManualOverrideFromCurrentChannel(void);
bool tsIsManualOverridden(Channel_t chan);
void tsSetContactHasBeenOverriden(Channel_t chan, bool isOverriden);
bool tsIsContactHasBeenOverridden(Channel_t chan);
bool tsIsContactHasBeenOverriddenFromCurrentChannel(void);

bool isQSODataAvailableForCurrentTalker(void);
int alignFrequencyToStep(int freq, int step);
char *chomp(char *str);
int32_t getFirstSpacePos(char *str);
void dmrIDCacheInit(void);
bool dmrIDLookup(uint32_t targetId, dmrIdDataStruct_t *foundRecord);
bool contactIDLookup(uint32_t id, uint32_t calltype, char *buffer);
void uiUtilityRenderQSOData(void);
void uiUtilityRenderHeader(bool isVFODualWatchScanning, bool isVFOSweepScanning);
void uiUtilityRedrawHeaderOnly(bool isVFODualWatchScanning, bool isVFOSweepScanning);
LinkItem_t *lastheardFindInList(uint32_t id);
void lastheardInitList(void);
void lastHeardClearWorkingTAData(void);
bool lastHeardListUpdate(uint8_t *dmrDataBuffer, bool forceOnHotspot);
void lastHeardClearLastID(void);
int getRSSIdBm(void);
void uiUtilityDrawRSSIBarGraph(void);
void uiUtilityDrawFMMicLevelBarGraph(void);
void uiUtilityDrawDMRMicLevelBarGraph(void);
void setOverrideTGorPC(uint32_t tgOrPc, bool privateCall);
void uiUtilityDisplayFrequency(uint8_t y, bool isTX, bool hasFocus, uint32_t frequency, bool displayVFOChannel, bool isScanMode, uint8_t dualWatchVFO);

uint16_t cssGetToneFromIndex(uint8_t index, CodeplugCSSTypes_t type);
uint8_t cssGetToneIndex(uint16_t tone, CodeplugCSSTypes_t type);
void cssIncrement(uint16_t *tone, uint8_t *index, uint8_t step, CodeplugCSSTypes_t *type, bool loop, bool stayInCSSType);
void cssDecrement(uint16_t *tone, uint8_t *index, uint8_t step, CodeplugCSSTypes_t *type, bool loop, bool stayInCSSType);

size_t dcsPrintf(char *dest, size_t maxLen, char *prefix, uint16_t tone);

void freqEnterReset(void);
int freqEnterRead(int startDigit, int endDigit, bool simpleDigits);

int getBatteryPercentage(void);
void getBatteryVoltage(int *volts, int *mvolts);
bool decreasePowerLevel(void);
bool increasePowerLevel(bool allowFullPower);

void announceChar(char ch);

void buildCSSCodeVoicePrompts(uint16_t tone, CodeplugCSSTypes_t cssType, Direction_t direction, bool announceType);
void announceCSSCode(uint16_t tone, CodeplugCSSTypes_t cssType, Direction_t direction, bool announceType, audioPromptThreshold_t immediateAnnounceThreshold);

void announceItemWithInit(bool init, voicePromptItem_t item, audioPromptThreshold_t immediateAnnounceThreshold);
void announceItem(voicePromptItem_t item, audioPromptThreshold_t immediateAnnouceThreshold);
void promptsPlayNotAfterTx(void);
void playNextSettingSequence(void);
void uiUtilityBuildTgOrPCDisplayName(char *nameBuf, int bufferLen);
void acceptPrivateCall(uint32_t id, int timeslot);
bool rebuildVoicePromptOnExtraLongSK1(uiEvent_t *ev);
bool repeatVoicePromptOnSK1(uiEvent_t *ev);
bool handleMonitorMode(uiEvent_t *ev);
void uiUtilityDisplayInformation(const char *str, displayInformation_t line, int8_t yOverride);
void uiUtilityRenderQSODataAndUpdateScreen(void);

// QuickKeys
void saveQuickkeyMenuIndex(char key, uint8_t menuId, uint8_t entryId, uint8_t function);
void saveQuickkeyMenuLongValue(char key, uint8_t menuId, uint16_t entryiI);
void saveQuickkeyContactIndex(char key, uint16_t contactId);

void cssIncrement2(uint16_t *tone, uint8_t *index, CodeplugCSSTypes_t *type, bool loop, bool stayInCSSType);
void cssIncrement(uint16_t *tone, uint8_t *index, uint8_t step, CodeplugCSSTypes_t *type, bool loop, bool stayInCSSType);
uint16_t cssGetTone(uint8_t index, CodeplugCSSTypes_t type);

bool uiShowQuickKeysChoices(char *buf, const int bufferLen, const char *menuTitle);

// DTMF contact sequences
void dtmfSequenceReset(void);
bool dtmfSequenceIsKeying(void);
void dtmfSequencePrepare(uint8_t *seq, bool autoStart);
void dtmfSequenceStart(void);
void dtmfSequenceStop(void);
void dtmfSequenceTick(bool popPreviousMenuOnEnding);

void resetOriginalSettingsData(void);
struct tm *gmtime_r_Custom(const time_t_custom *__restrict tim_p, struct tm *__restrict res);
time_t_custom mktime_custom(const struct tm * tb);

uint8_t *coordsToMaidenhead(uint8_t *maidenheadBuffer, double longitude, double latitude);

#endif
