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
#include "dmr_codec/codec.h"
#include "functions/sound.h"
#include "functions/voicePrompts.h"
#include "functions/settings.h"
#include "user_interface/uiLocalisation.h"
#include "functions/rxPowerSaving.h"
#include "functions/sonic_lite.h"
const uint32_t VOICE_PROMPTS_DATA_MAGIC = 0x5056;//'VP'
const uint32_t VOICE_PROMPTS_DATA_VERSION = 0x0006; // Version 6 TOC increased to 320. Added PROMOT_VOX and PROMPT_UNUSED_1 to PROMPT_UNUSED_10
													// Version 5 TOC increased to 300
													// Version 4 does not have unused items
                                                    // Version 3 does not have the PROMPT_TBD items in it
#define VOICE_PROMPTS_TOC_SIZE 320
static void getAmbeData(int offset,int length);

typedef struct
{
	uint32_t magic;
	uint32_t version;
} VoicePromptsDataHeader_t;

const uint32_t VOICE_PROMPTS_FLASH_HEADER_ADDRESS 		= 0x8F400;
const uint32_t VOICE_PROMPTS_FLASH_OLD_HEADER_ADDRESS 	= 0xE0000;
static uint32_t voicePromptsFlashDataAddress;// = VOICE_PROMPTS_FLASH_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t) + sizeof(uint32_t)*VOICE_PROMPTS_TOC_SIZE ;
// 76 x 27 byte ambe frames
#define AMBE_DATA_BUFFER_SIZE  2052
bool voicePromptDataIsLoaded = false;
static bool voicePromptIsActive = false;
static int promptDataPosition = -1;
static int currentPromptLength = -1;

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

void voicePromptsCacheInit(void)
{
	VoicePromptsDataHeader_t header;

	SPI_Flash_read(VOICE_PROMPTS_FLASH_HEADER_ADDRESS,(uint8_t *)&header,sizeof(VoicePromptsDataHeader_t));

	if (voicePromptsCheckMagicAndVersion((uint32_t *)&header))
	{
		voicePromptDataIsLoaded = SPI_Flash_read(VOICE_PROMPTS_FLASH_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t), (uint8_t *)&tableOfContents, sizeof(uint32_t) * VOICE_PROMPTS_TOC_SIZE);
		voicePromptsFlashDataAddress =  VOICE_PROMPTS_FLASH_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t) + sizeof(uint32_t)*VOICE_PROMPTS_TOC_SIZE ;
	}

	if (!voicePromptDataIsLoaded)
	{
		SPI_Flash_read(VOICE_PROMPTS_FLASH_OLD_HEADER_ADDRESS,(uint8_t *)&header,sizeof(VoicePromptsDataHeader_t));
		if (voicePromptsCheckMagicAndVersion((uint32_t *)&header))
		{
			voicePromptDataIsLoaded = SPI_Flash_read(VOICE_PROMPTS_FLASH_OLD_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t), (uint8_t *)&tableOfContents, sizeof(uint32_t) * VOICE_PROMPTS_TOC_SIZE);
			voicePromptsFlashDataAddress =  VOICE_PROMPTS_FLASH_OLD_HEADER_ADDRESS + sizeof(VoicePromptsDataHeader_t) + sizeof(uint32_t)*VOICE_PROMPTS_TOC_SIZE ;
		}
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
			if (voicePromptsCurrentSequence.Pos < (voicePromptsCurrentSequence.Length - 1))
			{
				voicePromptsCurrentSequence.Pos++;
				promptDataPosition = 0;

				int promptNumber = voicePromptsCurrentSequence.Buffer[voicePromptsCurrentSequence.Pos];

				currentPromptLength = tableOfContents[promptNumber + 1] - tableOfContents[promptNumber];

				getAmbeData(tableOfContents[promptNumber], currentPromptLength);
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

void voicePromptsAppendString(char *promptString)
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
		if ((*promptString >= '0') && (*promptString <= '9'))
		{
			voicePromptsAppendPrompt(*promptString - '0' + PROMPT_0);
		}
		else if ((*promptString >= 'A') && (*promptString <= 'Z'))
		{
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
		else
		{
			// otherwise just add silence
			voicePromptsAppendPrompt(PROMPT_SILENCE);
		}

		promptString++;
	}
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
		currentPromptLength = tableOfContents[promptNumber + 1] - tableOfContents[promptNumber];
		getAmbeData(tableOfContents[promptNumber], currentPromptLength);

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
