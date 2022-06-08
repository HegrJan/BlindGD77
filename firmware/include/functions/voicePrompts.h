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
#ifndef _OPENGD77_VOICEPROMPTS_H_
#define _OPENGD77_VOICEPROMPTS_H_

typedef enum
{
	PROMPT_SILENCE = 0,
	PROMPT_POINT,
	PROMPT_0, PROMPT_1,	PROMPT_2, PROMPT_3, PROMPT_4, PROMPT_5, PROMPT_6, PROMPT_7, PROMPT_8, PROMPT_9,
	PROMPT_A, PROMPT_B, PROMPT_C, PROMPT_D, PROMPT_E, PROMPT_F, PROMPT_G, PROMPT_H, PROMPT_I, PROMPT_J,
	PROMPT_K, PROMPT_L, PROMPT_M, PROMPT_N, PROMPT_O, PROMPT_P, PROMPT_Q, PROMPT_R, PROMPT_S, PROMPT_T,
	PROMPT_U, PROMPT_V, PROMPT_W, PROMPT_X, PROMPT_Y, PROMPT_Z, PROMPT_CAP,
	PROMPT_CHANNEL,
	PROMPT_CONTACT,
	PROMPT_MILLISECONDS,
	PROMPT_MEGAHERTZ,
	PROMPT_KILOHERTZ,
	PROMPT_TALKGROUP,
	PROMPT_TIMESLOT,
	PROMPT_VFO,
	PROMPT_SECONDS,
	PROMPT_MINUTES,
	PROMPT_VOLTS,
	PROMPT_WATT,
	PROMPT_WATTS,
	PROMPT_MILLIWATTS,
	PROMPT_PERCENT,
	PROMPT_RECEIVE,
	PROMPT_TRANSMIT,
	PROMPT_MODE,
	PROMPT_DMR,
	PROMPT_FM,
	PROMPT_PLUS,
	PROMPT_MINUS,
	PROMPT_VFO_EXCHANGE_TX_RX,
	PROMPT_VFO_COPY_RX_TO_TX,
	PROMPT_VFO_COPY_TX_TO_RX,
	PROMPT_POWER,
	PROMPT_CHANNEL_MODE,
	PROMPT_SCAN_MODE,
	PROMPT_TIMESLOT_MODE,
	PROMPT_FILTER_MODE,
	PROMPT_ZONE_MODE,
	PROMPT_POWER_MODE,
	PROMPT_COLORCODE_MODE,
	PROMPT_HERTZ,
	PROMPT_SETTINGS_UPDATE,
	PROMPT_STAR,
	PROMPT_HASH,
	PROMPT_VOX,
	PROMPT_SWEEP_SCAN_MODE,
	PROMPT_ECO_MODE,
	PROMPT_EDIT_VOICETAG,
	PROMPT_SPACE,
	PROMPT_A_PHONETIC,
PROMPT_B_PHONETIC,
PROMPT_C_PHONETIC,
PROMPT_D_PHONETIC,
PROMPT_E_PHONETIC,
PROMPT_F_PHONETIC,
PROMPT_G_PHONETIC,
PROMPT_H_PHONETIC,
PROMPT_I_PHONETIC,
PROMPT_J_PHONETIC,
PROMPT_K_PHONETIC,
PROMPT_L_PHONETIC,
PROMPT_M_PHONETIC,
PROMPT_N_PHONETIC,
PROMPT_O_PHONETIC,
PROMPT_P_PHONETIC,
PROMPT_Q_PHONETIC,
PROMPT_R_PHONETIC,
PROMPT_S_PHONETIC,
PROMPT_T_PHONETIC,
PROMPT_U_PHONETIC,
PROMPT_V_PHONETIC,
PROMPT_W_PHONETIC,
PROMPT_X_PHONETIC,
PROMPT_Y_PHONETIC,
PROMPT_Z_PHONETIC,
PROMPT_EXCLAIM,
PROMPT_COMMA,
PROMPT_AT,
PROMPT_COLON,
PROMPT_QUESTION,
PROMPT_LEFT_PAREN,
PROMPT_RIGHT_PAREN,
PROMPT_TILDE,
PROMPT_SLASH,
PROMPT_LEFT_BRACKET,
PROMPT_RIGHT_BRACKET,
PROMPT_LESS,
PROMPT_GREATER,
PROMPT_EQUALS,
PROMPT_DOLLAR,
PROMPT_APOSTROPHE,
PROMPT_GRAVE,
PROMPT_AMPERSAND,
PROMPT_BAR,
PROMPT_UNDERLINE,
PROMPT_CARET,
PROMPT_LEFT_BRACE,
PROMPT_RIGHT_BRACE,
PROMPT_CUSTOM1, // Hotspot
PROMPT_CUSTOM2, // ClearNode
PROMPT_CUSTOM3, // ShariNode
PROMPT_CUSTOM4, // MicroHub
PROMPT_CUSTOM5, // Openspot
PROMPT_CUSTOM6, // repeater
PROMPT_CUSTOM7, // BlindHams
PROMPT_CUSTOM8, // Allstar
PROMPT_CUSTOM9, // parrot
PROMPT_REVERSE,
NUM_VOICE_PROMPTS,
	__MAKE_ENUM_16BITS = INT16_MAX
} voicePrompt_t;

typedef struct
{
	const char* userWord;
	const voicePrompt_t vp;
} userDictEntry;

#define PROMPT_VOICE_NAME (NUM_VOICE_PROMPTS + (sizeof(stringsTable_t)/sizeof(char*)))
#define VOICE_PROMPT_CUSTOM 500

typedef enum
{
	vpAnnounceCaps=0x01,
	vpAnnounceCustomPrompts=0x02,
	vpAnnounceSpaceAndSymbols=0x04,
	vpAnnouncePhoneticRendering=0x08,
} VoicePromptFlags_T;

extern bool voicePromptDataIsLoaded;
extern const uint32_t VOICE_PROMPTS_FLASH_HEADER_ADDRESS;
extern const uint32_t VOICE_PROMPTS_FLASH_OLD_HEADER_ADDRESS;

void voicePromptsCacheInit(void);
void voicePromptsTick(void);// Called from HR-C6000.c

void voicePromptsInit(void);// Call before building the prompt sequence
void voicePromptsAppendPrompt(uint16_t prompt);// Append an individual prompt item. This can be a single letter number or a phrase
void voicePromptsAppendString(char *);// Append a text string e.g. "VK3KYY"
void voicePromptsAppendStringEx(char *promptString, VoicePromptFlags_T flags);
void voicePromptsAppendInteger(int32_t value); // Append a signed integer
void voicePromptsAppendLanguageString(const char * const *);//Append a text from the current language e.g. &currentLanguage->battery
void voicePromptsPlay(void);// Starts prompt playback
extern bool voicePromptsIsPlaying(void);
bool voicePromptsHasDataToPlay(void);
void voicePromptsTerminate(void);
bool voicePromptsCheckMagicAndVersion(uint32_t *bufferAddress);
 void ReplayDMR(void);
void ReplayInit(void);
void AddAmbeBlocksToReplayBuffer(uint8_t* ambeBlockPtr, uint8_t blockLen, bool reset, bool wrapWhenFull);
void SaveCustomVoicePrompt(int customPromptNumber, char* phrase); // phrase is an optional string to map to the ambe data. I.e. can map one's name to the recording of their name.
uint8_t GetMaxCustomVoicePrompts();
uint8_t GetNextFreeVoicePromptIndex(bool forDMRVoiceTag);
void DeleteDMRVoiceTag(int dmrVoiceTagNumber);
void voicePromptsSetEditMode(bool flag);
bool voicePromptsGetEditMode();
void voicePromptsAdjustEnd(bool adjustStart, int clipStep, bool absolute);
void voicePromptsEditAutoTrim();
void AnnounceEditBufferLength();
uint8_t voicePromptsGetLastCustomPromptNumberAnnounced();
bool voicePromptsCopyCustomPromptToEditBuffer(uint8_t customPromptNumber);
void SetDMRContinuousSave(bool flag);
bool GetDMRContinuousSave();
#endif
