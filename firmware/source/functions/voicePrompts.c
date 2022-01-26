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
#include "dmr_codec/codec.h"
#include "functions/sound.h"
#include "functions/voicePrompts.h"
#include "functions/settings.h"
#include "user_interface/uiLocalisation.h"
#include "functions/rxPowerSaving.h"
#include "functions/sonic_lite.h"
const uint32_t VOICE_PROMPTS_DATA_MAGIC = 0x5056;//'VP'
const uint32_t VOICE_PROMPTS_DATA_VERSION = 0x0006; // Version 6 TOC increased to 320. Added PROMPT_VOX and PROMPT_UNUSED_1 to PROMPT_UNUSED_10
													// Version 5 TOC increased to 300
													// Version 4 does not have unused items
                                                    // Version 3 does not have the PROMPT_TBD items in it
#define VOICE_PROMPTS_TOC_SIZE 320
#define CUSTOM_VOICE_PROMPT_PHRASE_LENGTH 16
static void getAmbeData(int offset,int length);
static int GetCustomVoicePromptData(int customPromptNumber);

typedef struct
{
	uint32_t magic;
	uint32_t version;
} VoicePromptsDataHeader_t;


typedef struct
{
	char customVPSignature[2];
	char  phrase[CUSTOM_VOICE_PROMPT_PHRASE_LENGTH]; // can map a phrase rather than just ##Digit to a custom prompt.
	uint16_t customVPLength;
} CustomVoicePromptsHeader_t;

const uint32_t VOICE_PROMPTS_FLASH_HEADER_ADDRESS 		= 0x8F400;
static uint32_t voicePromptsFlashDataAddress;// = VOICE_PROMPTS_FLASH_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t) + sizeof(uint32_t)*VOICE_PROMPTS_TOC_SIZE ;
// 76 x 27 byte ambe frames
#define AMBE_DATA_BUFFER_SIZE  2052
#define CUSTOM_VOICE_PROMPT_MAX_SIZE 1024 // approx 4 seconds of ambe data.
#define CUSTOM_VOICE_PROMPT_MIN_SIZE 27
#define VOICE_PROMPTS_REGION_TOP 0xfffff
#define maxCustomVoicePrompts 32
#define maxDMRVoiceTags 64
#define DMR_VOICE_TAG_BASE maxCustomVoicePrompts // DMR voice tags start at 33 and increase.

static uint8_t highestUsedCustomVoicePromptNumberWithPhrase = 0; // 1-based number of highest custom vp to stop searching at.
bool voicePromptDataIsLoaded = false;
static bool voicePromptIsActive = false;
static int promptDataPosition = -1;
static int currentPromptLength = -1;
static bool replayingDMR=false;
 
#define PROMPT_TAIL  30
static int promptTail = 0;

__attribute__((section(".data.$RAM2")))static uint8_t ambeData[AMBE_DATA_BUFFER_SIZE];

#define VOICE_PROMPTS_SEQUENCE_BUFFER_SIZE 128

typedef struct
{
	uint16_t  Buffer[VOICE_PROMPTS_SEQUENCE_BUFFER_SIZE];
	int  Pos;
	int  Length;
} VoicePromptsSequence_t;

__attribute__((section(".data.$RAM2"))) static VoicePromptsSequence_t voicePromptsCurrentSequence =
{
	.Pos = 0,
	.Length = 0
};

__attribute__((section(".data.$RAM2"))) uint32_t tableOfContents[VOICE_PROMPTS_TOC_SIZE];

__attribute__((section(".data.$RAM2"))) char phraseCache[maxCustomVoicePrompts][CUSTOM_VOICE_PROMPT_PHRASE_LENGTH]; //cache the phrases we have custom prompts for.

const userDictEntry userDictionary[]=
{
	{"hotspot", PROMPT_UNUSED_3},
	{"clearnode", PROMPT_UNUSED_4},
	{"openspot", PROMPT_UNUSED_5},
	{"microhub", PROMPT_UNUSED_6},
	{"allstar", PROMPT_UNUSED_7},
	{"disconnect", PROMPT_UNUSED_8},
	{"parrot", PROMPT_UNUSED_9},
		{0, 0}
};
// replay logic shares ambe buffer so is colocated here also

// Each ambe buffer is 9 bytes, encoding happens in lots of 3 blocks, i.e. 27 bytes at a time.
// Buffer is 50 lots of 27 bytes.
#define ambeREPLAY_BUFFER_LEN 1350

typedef struct
{
	CustomVoicePromptsHeader_t hdr; // stored hear to make it easier to write to flash in one write.
	uint8_t ambeBuffer[ambeREPLAY_BUFFER_LEN];
	uint8_t* head;
	uint8_t* tail;
	uint8_t* end;
	bool allowWrap;
} replayAmbeCircularBuffer_t;

static replayAmbeCircularBuffer_t replayBuffer;

static void replayAmbeCircularBufferInit(replayAmbeCircularBuffer_t *cb)
{
	cb->end = &cb->ambeBuffer[ambeREPLAY_BUFFER_LEN - 1];
	memset(&cb->ambeBuffer, 0, sizeof(cb->ambeBuffer));
	cb->head = cb->ambeBuffer;
	cb->tail = cb->ambeBuffer;
	cb->allowWrap=true;
	cb->hdr.customVPSignature[0]='v';
	cb->hdr.customVPSignature[1]='p';
	memset(cb->hdr.phrase, 0, sizeof(cb->hdr.phrase));
	cb->hdr.customVPLength=0; // set this once we know the length.
}

static void replayAmbeCircularBufferPushBack(replayAmbeCircularBuffer_t *cb, uint8_t* ambeBlockPtr, uint8_t blockLen, bool reset, bool wrapWhenFull)
{
	if (reset)
		replayAmbeCircularBufferInit(cb);
	cb->allowWrap=wrapWhenFull;

	if (!cb->allowWrap && (cb->head == cb->end))
		return;
	size_t  count = 0;
	uint8_t*p = ambeBlockPtr;

	while (count < blockLen)
     {
		*cb->head = *p;

		p++;
		count++;
		cb->head++;
		if (cb->head == cb->end)
		{
			if (!cb->allowWrap)
				return;
			cb->head= cb->ambeBuffer;
		}
		
		if (cb->tail == cb->head)
		{// the tail must always be on a block boundary.
			cb->tail++;
while (((cb->tail-cb->head)%9)!=0)
{
	cb->tail++;
}
			if(cb->tail >= cb->end)
			{
				int diff=cb->tail - cb->end;
				cb->tail = cb->ambeBuffer+diff;
			}
		}
	}
}

static uint16_t replayAmbeGetLength(replayAmbeCircularBuffer_t *cb)
{
	if (!cb->allowWrap)
		return cb->head-cb->ambeBuffer;

	uint16_t count = 0;
	uint8_t*p = cb->tail;

	while (p != cb->head)
	{
		p++;
		count++;

		if (p == cb->end)
		{
			p = cb->ambeBuffer;
		}
	}
	return count;
}

static size_t replayAmbeGetData(replayAmbeCircularBuffer_t* cb, uint8_t* data, size_t dataLen)
{
     size_t  count = 0;
     uint8_t*p = cb->tail;

     while ((p != cb->head) && (count < dataLen))
     {
    	 *(data + count) = *p;

    	 p++;
    	 count++;

    	 if (p == cb->end)
    	 {
			 if (!cb->allowWrap)
				 return count;
    		 p = cb->ambeBuffer;
    	 }
     }

     return count;
}

static bool CheckCustomVPSignature(CustomVoicePromptsHeader_t* hdr)
{
	return hdr->customVPSignature[0]=='v' && hdr->customVPSignature[1]=='p' && hdr->customVPLength >= CUSTOM_VOICE_PROMPT_MIN_SIZE && hdr->customVPLength < CUSTOM_VOICE_PROMPT_MAX_SIZE;
}

static void InitPhraseCache()
{
	memset(phraseCache,0, sizeof(phraseCache));
	for (int i=0; i < maxCustomVoicePrompts; ++i)
	{
		uint32_t addr=VOICE_PROMPTS_REGION_TOP-((i+1)*CUSTOM_VOICE_PROMPT_MAX_SIZE);
		CustomVoicePromptsHeader_t hdr;
		if (!SPI_Flash_read(addr, (uint8_t*)&hdr, sizeof(hdr)) || !CheckCustomVPSignature(&hdr) || !hdr.phrase || !*hdr.phrase)
			continue;
		highestUsedCustomVoicePromptNumberWithPhrase = i+1;
		int len=strlen(hdr.phrase);
		strncpy(phraseCache[i], hdr.phrase, len);
		phraseCache[i][CUSTOM_VOICE_PROMPT_PHRASE_LENGTH-1]='\0';// in case the prompt was exactly 16 chars.
	}	
}

void voicePromptsCacheInit(void)
{
	VoicePromptsDataHeader_t header;

	SPI_Flash_read(VOICE_PROMPTS_FLASH_HEADER_ADDRESS,(uint8_t *)&header,sizeof(VoicePromptsDataHeader_t));

	if (voicePromptsCheckMagicAndVersion((uint32_t *)&header))
	{
		voicePromptDataIsLoaded = SPI_Flash_read(VOICE_PROMPTS_FLASH_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t), (uint8_t *)&tableOfContents, sizeof(uint32_t) * VOICE_PROMPTS_TOC_SIZE);
		voicePromptsFlashDataAddress =  VOICE_PROMPTS_FLASH_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t) + sizeof(uint32_t)*VOICE_PROMPTS_TOC_SIZE ;
	}

	// DMR (digital) is disabled.
	if (uiDataGlobal.dmrDisabled)
	{
		voicePromptDataIsLoaded = false;
	}

	// is data is not loaded change prompt mode back to beep.
	if ((nonVolatileSettings.audioPromptMode > AUDIO_PROMPT_MODE_BEEP) && (voicePromptDataIsLoaded == false))
	{
		settingsSet(nonVolatileSettings.audioPromptMode, AUDIO_PROMPT_MODE_BEEP);
	}
	ReplayInit();
	InitPhraseCache();
}

bool voicePromptsCheckMagicAndVersion(uint32_t *bufferAddress)
{
	VoicePromptsDataHeader_t *header = (VoicePromptsDataHeader_t *)bufferAddress;

	return ((header->magic == VOICE_PROMPTS_DATA_MAGIC) && (header->version == VOICE_PROMPTS_DATA_VERSION));
}

static void getAmbeData(int offset,int length)
{
	if (length <= AMBE_DATA_BUFFER_SIZE)
	{
		SPI_Flash_read(voicePromptsFlashDataAddress + offset, (uint8_t *)&ambeData, length);
	}
}

static void RetractWriteBufferPtr(int retractBy)
{
	if (retractBy==0) 
		return;
	
	if (wavbuffer_count < retractBy) 
		return;
	
	wavbuffer_count-=retractBy;
	// also retract the write index.
	if (wavbuffer_write_idx-retractBy >= 0)
		wavbuffer_write_idx-=retractBy;
	else
	{// last decode caused write ptr to wrap around to start so move it back relative to the end of the circular buffer.
		int diff=retractBy-wavbuffer_write_idx;
		wavbuffer_write_idx=WAV_BUFFER_COUNT-diff;
	} 
}

static void ApplyVolAndRateToCurrentWaveBuffers(int startDecodeIndex, bool flush)
{
	if (replayingDMR) return;
	
	const int singleBufferSamples=WAV_BUFFER_SIZE/2; // WAV_BUFFER_SIZE is in bytes, each sample takes up two bytes (16 bits).
	const int maxSamples= 6 * singleBufferSamples; // 6 buffers are decoded at once, each sample takes up two bytes.

	// Because we only speed up prompts, the number of samples in total will always be less than or equal to the original number of samples.
	int readIdx=startDecodeIndex;
	for (int bufferCount=0; bufferCount < 6; ++bufferCount)
	{
		sonicWriteShortToStream((short*)audioAndHotspotDataBuffer.wavbuffer[readIdx], singleBufferSamples);
		readIdx=(readIdx+1)%WAV_BUFFER_COUNT;
	}
	if (flush)
		sonicFlushStream();
	
	int numSamples = SAFE_MIN(sonicSamplesAvailable(), maxSamples);
			int fullBuffers=6;

	if ((numSamples < maxSamples) && !flush)
	{// only read an exact multiple of the buffer size so that the read ptr only ever sees full buffers.
		// Leave the rest in the stream for next time.
		fullBuffers=(numSamples / singleBufferSamples);
		int retractBufferCount=6-fullBuffers;// 6 buffers are used when decoding a DMR frame.
		RetractWriteBufferPtr(retractBufferCount); // retract over any partial buffer and reuse for next decode.
	}
	readIdx=startDecodeIndex;
	for (int bufferCount=0; bufferCount < fullBuffers; ++bufferCount)
	{
		sonicReadShortFromStream((short*)audioAndHotspotDataBuffer.wavbuffer[readIdx], singleBufferSamples); // write the processed samples back to the buffers.
		readIdx=(readIdx+1)%WAV_BUFFER_COUNT;
	}
}

void voicePromptsTick(void)
{
	if (voicePromptIsActive)
	{
		if (promptDataPosition < currentPromptLength)
		{
			if (wavbuffer_count <= (WAV_BUFFER_COUNT / 2))
			{
				int startDecodeIndex=wavbuffer_write_idx; // save it off as decode will fill 6 buffers.
				codecDecode((uint8_t *)&ambeData[promptDataPosition], 3);
				promptDataPosition += 27;
				ApplyVolAndRateToCurrentWaveBuffers(startDecodeIndex, promptDataPosition>=currentPromptLength);
			}

			soundTickRXBuffer();
		}
		else
		{
			if (!replayingDMR && voicePromptsCurrentSequence.Pos < (voicePromptsCurrentSequence.Length - 1))
			{
				voicePromptsCurrentSequence.Pos++;
				promptDataPosition = 0;

				int promptNumber = voicePromptsCurrentSequence.Buffer[voicePromptsCurrentSequence.Pos];
				if (promptNumber > VOICE_PROMPT_CUSTOM)
				{
					currentPromptLength=GetCustomVoicePromptData(promptNumber-VOICE_PROMPT_CUSTOM);
				}
				else
				{
					currentPromptLength = tableOfContents[promptNumber + 1] - tableOfContents[promptNumber];

					getAmbeData(tableOfContents[promptNumber], currentPromptLength);
				}
			}
			else
			{
				// wait for wave buffer to empty when prompt has finished playing

				if (wavbuffer_count == 0)
				{
					voicePromptsTerminate();
				}
			}
		}
	}
	else
	{
		if (promptTail > 0)
		{
			promptTail--;

			if ((promptTail == 0) && trxCarrierDetected() && (trxGetMode() == RADIO_MODE_ANALOG))
			{
				GPIO_PinWrite(GPIO_RX_audio_mux, Pin_RX_audio_mux, 1); // Set the audio path to AT1846 -> audio amp.
			}
		}
	}
}

void voicePromptsTerminate(void)
{
	if (voicePromptIsActive)
	{
		disableAudioAmp(AUDIO_AMP_MODE_PROMPT);

		voicePromptsCurrentSequence.Pos = 0;
		soundTerminateSound();
		soundInit();
		promptTail = PROMPT_TAIL;

		taskENTER_CRITICAL();
		voicePromptIsActive = false;
		replayingDMR=false;
		taskEXIT_CRITICAL();
	}
}

void voicePromptsInit(void)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	if (voicePromptIsActive)
	{
		voicePromptsTerminate();
	}
	
	sonicInit();
	float volume = nonVolatileSettings.voicePromptVolumePercent * 0.01;
	float speed = (10+nonVolatileSettings.voicePromptRate) * 0.1;
	sonicSetSpeed(speed);
	sonicSetVolume(volume);

	voicePromptsCurrentSequence.Length = 0;
	voicePromptsCurrentSequence.Pos = 0;
}

void voicePromptsAppendPrompt(uint16_t prompt)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	if (voicePromptIsActive)
	{
		voicePromptsInit();
	}
	if (voicePromptsCurrentSequence.Length < VOICE_PROMPTS_SEQUENCE_BUFFER_SIZE)
	{
		voicePromptsCurrentSequence.Buffer[voicePromptsCurrentSequence.Length] = prompt;
		voicePromptsCurrentSequence.Length++;
	}
}

static uint16_t LookupCustomPrompt(char* ptr, int* advanceBy)
{
	for (int i=0; i < highestUsedCustomVoicePromptNumberWithPhrase; ++i)
	{
		if (!phraseCache[i][0])
			continue;
		int len=strlen(phraseCache[i]);
		if (strncasecmp(phraseCache[i], ptr, len)==0)
		{
			*advanceBy+=len-1;
			return i+1; // prompts are numbered from 1.
		}
	}	
	return 0;
}

static uint16_t Lookup(char* ptr, int* advanceBy, bool includeCustomPrompts)
{
	if (!ptr) return 0;
	
	for (int index=0; userDictionary[index].userWord!=0; ++index)
	{
		int len=strlen(userDictionary[index].userWord);
		if (strncasecmp(userDictionary[index].userWord, ptr, len)==0)
		{
			*advanceBy=len-1;
			return userDictionary[index].vp;
		}
		// look up ## followed by digit and speak as custom prompt.
		if (includeCustomPrompts)
		{
		if (strncmp(ptr, "##", 2)==0 && isdigit(ptr[2]))
		{
			int customPromptNumber=atoi(ptr+2);
			*advanceBy=3;
			return VOICE_PROMPT_CUSTOM+customPromptNumber;
		}
		// look it up in the custum prompt's dictionary.
		int customPromptNumber=LookupCustomPrompt(ptr, advanceBy);
		if (customPromptNumber > 0)
			return VOICE_PROMPT_CUSTOM+customPromptNumber;
		}
	}
	return 0;
}

void voicePromptsAppendStringWithCaps(char *promptString, bool indicateCaps, bool includeCustomPrompts, bool saySpaceAndSymbols)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	if (voicePromptIsActive)
	{
		voicePromptsInit();
	}
	while (*promptString != 0)
	{
		int advanceBy=0;
		uint16_t vp=Lookup(promptString, &advanceBy, includeCustomPrompts);
		if (vp)
		{
			voicePromptsAppendPrompt(vp);
			promptString+=advanceBy;
		}
		else if ((*promptString >= '0') && (*promptString <= '9'))
		{
			voicePromptsAppendPrompt(*promptString - '0' + PROMPT_0);
		}
		else if ((*promptString >= 'A') && (*promptString <= 'Z'))
		{
			if (indicateCaps)
				voicePromptsAppendPrompt(PROMPT_CAP);
			voicePromptsAppendPrompt(*promptString - 'A' + PROMPT_A);
		}
		else if ((*promptString >= 'a') && (*promptString <= 'z'))
		{
			voicePromptsAppendPrompt(*promptString - 'a' + PROMPT_A);
		}
		else if (*promptString == '.')
		{
			voicePromptsAppendPrompt(PROMPT_POINT);
		}
		else if (*promptString == '+')
		{
			voicePromptsAppendPrompt(PROMPT_PLUS);
		}
		else if (*promptString == '-')
		{
			voicePromptsAppendPrompt(PROMPT_MINUS);
		}
		else if (*promptString == '%')
		{
			voicePromptsAppendPrompt(PROMPT_PERCENT);
		}
		else if (*promptString == '*')
		{
			voicePromptsAppendPrompt(PROMPT_STAR);
		}
		else if (*promptString == '#')
		{
			voicePromptsAppendPrompt(PROMPT_HASH);
		}
		else if (saySpaceAndSymbols)
		{
			if (*promptString==' ')
				voicePromptsAppendPrompt(PROMPT_SPACE);
			else
			{
				int32_t val = *promptString;
				voicePromptsAppendLanguageString(&currentLanguage->dtmf_code); // just the word "code" as we don't have character.
				voicePromptsAppendInteger(val);
			}
		}
		else
			// otherwise just add silence
			voicePromptsAppendPrompt(PROMPT_SILENCE);
		
		promptString++;
	}
}

void voicePromptsAppendString(char *promptString)
{
	voicePromptsAppendStringWithCaps(promptString, false, true, false);
}

void voicePromptsAppendInteger(int32_t value)
{
	char buf[12] = {0}; // min: -2147483648, max: 2147483647
	itoa(value, buf, 10);
	voicePromptsAppendString(buf);
}

void voicePromptsAppendLanguageString(const char * const *languageStringAdd)
{
	if ((nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1) || (languageStringAdd == NULL))
	{
		return;
	}
	voicePromptsAppendPrompt(NUM_VOICE_PROMPTS + (languageStringAdd - &currentLanguage->LANGUAGE_NAME));
}

void voicePromptsPlay(void)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	if ((voicePromptIsActive == false) && (voicePromptsCurrentSequence.Length > 0))
	{
		rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);

		taskENTER_CRITICAL();

		// Early state change to lock out HR-C6000.
		voicePromptIsActive = true;// Start the playback
		if (melody_play && (melody_play != MELODY_ERROR_BEEP) && (melody_play != MELODY_ACK_BEEP))
		{
			soundStopMelody();
		}

		int promptNumber = voicePromptsCurrentSequence.Buffer[0];

		voicePromptsCurrentSequence.Pos = 0;
		if (promptNumber > VOICE_PROMPT_CUSTOM)
		{
			currentPromptLength=GetCustomVoicePromptData(promptNumber-VOICE_PROMPT_CUSTOM);
		}
		else
		{
			currentPromptLength = tableOfContents[promptNumber + 1] - tableOfContents[promptNumber];
			getAmbeData(tableOfContents[promptNumber], currentPromptLength);
		}
		GPIO_PinWrite(GPIO_RX_audio_mux, Pin_RX_audio_mux, 0);// set the audio mux HR-C6000 -> audio amp
		enableAudioAmp(AUDIO_AMP_MODE_PROMPT);

		codecInit(true);
		promptDataPosition = 0;

		taskEXIT_CRITICAL();
	}
}

inline bool voicePromptsIsPlaying(void)
{
	return (voicePromptIsActive || (promptTail > 0));
}

bool voicePromptsHasDataToPlay(void)
{
	return (voicePromptsCurrentSequence.Length > 0);
}

 void ReplayDMR(void)
{
	if (voicePromptIsActive) return;
	
	rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);

	taskENTER_CRITICAL();
	voicePromptIsActive=true;
	replayingDMR=true;
	if (melody_play && (melody_play != MELODY_ERROR_BEEP) && (melody_play != MELODY_ACK_BEEP))
	{
		soundStopMelody();
	}
	currentPromptLength =replayAmbeGetLength(&replayBuffer);
	replayAmbeGetData(&replayBuffer, (uint8_t *)&ambeData, currentPromptLength);
	promptDataPosition = 0;

	GPIO_PinWrite(GPIO_RX_audio_mux, Pin_RX_audio_mux, 0);// set the audio mux HR-C6000 -> audio amp
	enableAudioAmp(AUDIO_AMP_MODE_PROMPT);

	codecInit(true);

	taskEXIT_CRITICAL();
}

void ReplayInit()
{
	replayingDMR=false;
	replayAmbeCircularBufferInit(&replayBuffer);
}

void AddAmbeBlocksToReplayBuffer(uint8_t* ambeBlockPtr, uint8_t blockLen, bool reset, bool wrapWhenFull)
{
	replayAmbeCircularBufferPushBack(&replayBuffer, ambeBlockPtr, blockLen, reset, wrapWhenFull);
}

static bool SaveAMBEBufferAsCustomVoicePrompt(int customPromptNumber, char* phrase)
{
	if (!voicePromptDataIsLoaded) return false;

	uint16_t length=replayAmbeGetLength(&replayBuffer);
	if (customPromptNumber < 1 || customPromptNumber > maxCustomVoicePrompts+maxDMRVoiceTags) 
		return false;
	// custom voice prompts are saved moving downward from the top of the voice prompt area. Each one is a fixed size for ease of changing.
	bool deleting=length < CUSTOM_VOICE_PROMPT_MIN_SIZE;
	if (!deleting && phrase && *phrase)
	{
		strncpy(replayBuffer.hdr.phrase, phrase, sizeof(replayBuffer.hdr.phrase));
		replayBuffer.hdr.phrase[sizeof(replayBuffer.hdr.phrase)-1]='\0';
	}
	else if (deleting)
		memset(replayBuffer.hdr.phrase, 0, sizeof(replayBuffer.hdr.phrase));
	else // keep whatever is in the cache for this prompt, we're just replacing the recording.
		memcpy(replayBuffer.hdr.phrase, phraseCache[customPromptNumber-1], CUSTOM_VOICE_PROMPT_PHRASE_LENGTH);
	// update the phrase cache for regular custom voice prompts.
	if (customPromptNumber <= maxCustomVoicePrompts)
	{
		strncpy(phraseCache[customPromptNumber-1], replayBuffer.hdr.phrase, CUSTOM_VOICE_PROMPT_PHRASE_LENGTH);
		if (phrase && (customPromptNumber > highestUsedCustomVoicePromptNumberWithPhrase) && !deleting)
			highestUsedCustomVoicePromptNumberWithPhrase = customPromptNumber;
		else if (deleting && highestUsedCustomVoicePromptNumberWithPhrase == customPromptNumber)
			highestUsedCustomVoicePromptNumberWithPhrase--;
	}
	replayBuffer.hdr.customVPLength=deleting ? 0 : SAFE_MIN(length, (CUSTOM_VOICE_PROMPT_MAX_SIZE-sizeof(replayBuffer.hdr)));
	uint32_t addr=VOICE_PROMPTS_REGION_TOP-(customPromptNumber*CUSTOM_VOICE_PROMPT_MAX_SIZE);
	// normalize the buffer in case wrapping is enabled which will be the case when saving a DMR voice tag.
	if (replayBuffer.allowWrap)
	{
		replayAmbeCircularBuffer_t normalizedData;
		replayAmbeCircularBufferInit(&normalizedData);
		memcpy(&normalizedData.hdr, &replayBuffer.hdr, sizeof(CustomVoicePromptsHeader_t));
		replayAmbeGetData(&replayBuffer, (uint8_t*)&normalizedData.ambeBuffer, normalizedData.hdr.customVPLength);
		return SPI_Flash_write(addr, (uint8_t*)&normalizedData, CUSTOM_VOICE_PROMPT_MAX_SIZE);
	}
	return SPI_Flash_write(addr, (uint8_t*)&replayBuffer, CUSTOM_VOICE_PROMPT_MAX_SIZE);
}

static int GetCustomVoicePromptData(int customPromptNumber)
{
	if (!voicePromptDataIsLoaded) return 0;
	if (customPromptNumber < 1 || customPromptNumber > (maxCustomVoicePrompts+maxDMRVoiceTags)) return 0;
	// custom voice prompts are saved moving downward from the top of the voice prompt area. Each one is a fixed size for ease of modification.
	uint32_t addr=VOICE_PROMPTS_REGION_TOP-(customPromptNumber*CUSTOM_VOICE_PROMPT_MAX_SIZE);
		CustomVoicePromptsHeader_t hdr;
	if (!SPI_Flash_read(addr, (uint8_t*)&hdr, sizeof(hdr)) || !CheckCustomVPSignature(&hdr))
		return 0;
	
	SPI_Flash_read(addr+sizeof(hdr), (uint8_t *)&ambeData, hdr.customVPLength);
	return hdr.customVPLength;
}

void SaveCustomVoicePrompt(int customPromptNumber, char* phrase)
{
	voicePromptsInit();
	if (SaveAMBEBufferAsCustomVoicePrompt(customPromptNumber, phrase))
	{
		voicePromptsAppendInteger(customPromptNumber);
		// When appending a custom prompt, we need to add the VOICE_PROMPT_CUSTOM to it so the code knows it is a custom prompt.
		voicePromptsAppendPrompt(VOICE_PROMPT_CUSTOM+customPromptNumber);
		voicePromptsAppendPrompt(PROMPT_SILENCE);
		voicePromptsAppendLanguageString(&currentLanguage->vp_saved);
	}
	else
	{
		voicePromptsAppendLanguageString(&currentLanguage->list_full);
	}
	voicePromptsPlay();
}

uint8_t GetMaxCustomVoicePrompts()
{
	return maxCustomVoicePrompts;
}

uint8_t GetNextFreeDMRVoiceTagIndex()
{
	for (int i=DMR_VOICE_TAG_BASE; i < DMR_VOICE_TAG_BASE+maxDMRVoiceTags; ++i)
	{
		uint32_t addr=VOICE_PROMPTS_REGION_TOP-((i+1)*CUSTOM_VOICE_PROMPT_MAX_SIZE);
		CustomVoicePromptsHeader_t hdr;
		if (!SPI_Flash_read(addr, (uint8_t*)&hdr, sizeof(hdr)) || !CheckCustomVPSignature(&hdr))
			return i+1;
	}
	return 0;
}
void DeleteDMRVoiceTag(int dmrVoiceTagNumber)
{
	if (!voicePromptDataIsLoaded) return;
	if (dmrVoiceTagNumber <= DMR_VOICE_TAG_BASE || dmrVoiceTagNumber > DMR_VOICE_TAG_BASE+maxDMRVoiceTags)
		return;
	// custom voice prompts are saved moving downward from the top of the voice prompt area. Each one is a fixed size for ease of changing.
	memset(&replayBuffer.hdr, 0, sizeof(replayBuffer.hdr));
	uint32_t addr=VOICE_PROMPTS_REGION_TOP-(dmrVoiceTagNumber*CUSTOM_VOICE_PROMPT_MAX_SIZE);
	SPI_Flash_write(addr, (uint8_t*)&replayBuffer.hdr, sizeof(replayBuffer.hdr));
}
