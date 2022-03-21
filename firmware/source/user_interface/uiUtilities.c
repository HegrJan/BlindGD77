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

#include <math.h>
#include "hardware/EEPROM.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/editHandler.h"
#include "user_interface/uiLocalisation.h"
#include "hardware/HR-C6000.h"
#include "functions/settings.h"
#include "hardware/SPI_Flash.h"
#include "functions/ticks.h"
#include "functions/trx.h"
#include "functions/autozone.h"
//see sonic_lite.h
#include "functions/sonic_lite.h"
#include "functions/rxPowerSaving.h"
static const uint8_t DECOMPRESS_LUT[64] = { ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '.' };
static uint8_t customVoicePromptIndex=0xff;
static uint8_t customVoicePromptIndexToSave=0xff;

static __attribute__((section(".data.$RAM2"))) LinkItem_t callsList[NUM_LASTHEARD_STORED];

static uint32_t dmrIdDataArea_1_Size;
static const uint32_t DMRID_HEADER_LENGTH = 0x0C;
static const uint32_t DMRID_MEMORY_LOCATION_1 = 0x30000;
static uint32_t dmrIDDatabaseMemoryLocation2 = 0xB8000;

static dmrIDsCache_t dmrIDsCache;
static voicePromptItem_t voicePromptSequenceState = PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ;
static uint32_t lastTG = 0;

volatile uint32_t lastID = 0;// This needs to be volatile as lastHeardClearLastID() is called from an ISR
LinkItem_t *LinkHead = callsList;
static uint32_t lastHeardNeedsAnnouncementTimer=-1; //reset is -1.
static uint32_t lastHeardUpdateTime=0;
// Try and avoid triggering speaking of last heard if reception breaks up, wait 750 ms after end of reception.
#define LAST_HEARD_TIMER_TIMEOUT 750

static void announceChannelNameOrVFOFrequency(bool voicePromptWasPlaying, bool announceVFOName);
static void dmrDbTextDecode(uint8_t *decompressedBufOut, uint8_t *compressedBufIn, int compressedSize);

DECLARE_SMETER_ARRAY(rssiMeterHeaderBar, DISPLAY_SIZE_X);

uint32_t DMRID_IdLength = 4U;

// Set TS manual override
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
// ts: 1, 2, TS_NO_OVERRIDE
void tsSetManualOverride(Channel_t chan, int8_t ts)
{
	uint16_t tsOverride = nonVolatileSettings.tsManualOverride;

	// Clear TS override for given channel
	tsOverride &= ~(0x03 << (2 * ((int8_t)chan)));
	if (ts != TS_NO_OVERRIDE)
	{
		// Set TS override for given channel
		tsOverride |= (ts << (2 * ((int8_t)chan)));
	}

	settingsSet(nonVolatileSettings.tsManualOverride, tsOverride);
}

// Set TS manual override from contact TS override
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
// contact: apply TS override from contact setting
void tsSetFromContactOverride(Channel_t chan, struct_codeplugContact_t *contact)
{
	if ((contact->reserve1 & 0x01) == 0x00)
	{
		tsSetManualOverride(chan, (((contact->reserve1 & 0x02) >> 1) + 1));
	}
	else
	{
		tsSetManualOverride(chan, TS_NO_OVERRIDE);
	}
}

// Get TS override value
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
// returns (TS + 1, 0 no override)
int8_t tsGetManualOverride(Channel_t chan)
{
	return (nonVolatileSettings.tsManualOverride >> (2 * (int8_t)chan)) & 0x03;
}

// Get manually overridden TS, if any, from currentChannelData
// returns (TS + 1, 0 no override)
int8_t tsGetManualOverrideFromCurrentChannel(void)
{
	Channel_t chan = (((currentChannelData->NOT_IN_CODEPLUG_flag & 0x01) == 0x01) ?
			(((currentChannelData->NOT_IN_CODEPLUG_flag & 0x02) == 0x02) ? CHANNEL_VFO_B : CHANNEL_VFO_A) : CHANNEL_CHANNEL);

	return tsGetManualOverride(chan);
}

// Check if TS is overrode
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
// returns true on overrode for the specified channel
bool tsIsManualOverridden(Channel_t chan)
{
	return (nonVolatileSettings.tsManualOverride & (0x03 << (2 * ((int8_t)chan))));
}

// Keep track of an override when the selected contact has a TS override set
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
void tsSetContactHasBeenOverriden(Channel_t chan, bool isOverriden)
{
	uint16_t tsOverride = nonVolatileSettings.tsManualOverride;

	if (isOverriden)
	{
		tsOverride |= (1 << ((3 * 2) + ((int8_t)chan)));
	}
	else
	{
		tsOverride &= ~(1 << ((3 * 2) + ((int8_t)chan)));
	}

	settingsSet(nonVolatileSettings.tsManualOverride, tsOverride);
}

// Get TS override status, of a selected contact which have a TS override set
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
bool tsIsContactHasBeenOverriddenFromCurrentChannel(void)
{
	Channel_t chan = (((currentChannelData->NOT_IN_CODEPLUG_flag & 0x01) == 0x01) ?
			(((currentChannelData->NOT_IN_CODEPLUG_flag & 0x02) == 0x02) ? CHANNEL_VFO_B : CHANNEL_VFO_A) : CHANNEL_CHANNEL);

	return tsIsContactHasBeenOverridden(chan);
}

// Get manual TS override of the selected contact (which has a TS override set)
// chan: CHANNEL_VFO_A, CHANNEL_VFO_B, CHANNEL_CHANNEL
bool tsIsContactHasBeenOverridden(Channel_t chan)
{
	return (nonVolatileSettings.tsManualOverride >> ((3 * 2) + (int8_t)chan)) & 0x01;
}

bool isQSODataAvailableForCurrentTalker(void)
{
	LinkItem_t *item = NULL;
	uint32_t rxID = HRC6000GetReceivedSrcId();

	// We're in digital mode, RXing, and current talker is already at the top of last heard list,
	// hence immediately display complete contact/TG info on screen
	if ((trxTransmissionEnabled == false) && ((trxGetMode() == RADIO_MODE_DIGITAL) && (rxID != 0) && (HRC6000GetReceivedTgOrPcId() != 0)) &&
			(getAudioAmpStatus() & AUDIO_AMP_MODE_RF)
			&& checkTalkGroupFilter() &&
			(((item = lastheardFindInList(rxID)) != NULL) && (item == LinkHead)))
	{
		return true;
	}

	return false;
}


int alignFrequencyToStep(int freq, int step)
{
	int r = freq % step;

	return (r ? freq + (step - r) : freq);
}

/*
 * Remove space at the end of the array, and return pointer to first non space character
 */
char *chomp(char *str)
{
	char *sp = str, *ep = str;

	while (*ep != '\0')
	{
		ep++;
	}

	// Spaces at the end
	while (ep > str)
	{
		if (*ep == '\0')
		{
		}
		else if (*ep == ' ')
		{
			*ep = '\0';
		}
		else
		{
			break;
		}

		ep--;
	}

	// Spaces at the beginning
	while (*sp == ' ')
	{
		sp++;
	}

	return sp;
}

int32_t getFirstSpacePos(char *str)
{
	char *p = str;

	while(*p != '\0')
	{
		if (*p == ' ')
		{
			return (p - str);
		}

		p++;
	}

	return -1;
}

void lastheardInitList(void)
{
	LinkHead = callsList;

	for(int i = 0; i < NUM_LASTHEARD_STORED; i++)
	{
		callsList[i].id = 0;
		callsList[i].talkGroupOrPcId = 0;
		callsList[i].contact[0] = 0;
		callsList[i].talkgroup[0] = 0;
		callsList[i].talkerAlias[0] = 0;
		callsList[i].locator[0] = 0;
		callsList[i].time = 0;

		if (i == 0)
		{
			callsList[i].prev = NULL;
		}
		else
		{
			callsList[i].prev = &callsList[i - 1];
		}

		if (i < (NUM_LASTHEARD_STORED - 1))
		{
			callsList[i].next = &callsList[i + 1];
		}
		else
		{
			callsList[i].next = NULL;
		}
	}
}

LinkItem_t *lastheardFindInList(uint32_t id)
{
	LinkItem_t *item = LinkHead;

	while (item->next != NULL)
	{
		if (item->id == id)
		{
			// found it
			return item;
		}
		item = item->next;
	}
	return NULL;
}

static uint8_t *coordsToMaidenhead(double longitude, double latitude)
{
	static uint8_t maidenhead[15];
	double l, l2;
	uint8_t c;

	l = longitude;

	for (uint8_t i = 0; i < 2; i++)
	{
		l = l / ((i == 0) ? 20.0 : 10.0) + 9.0;
		c = (uint8_t) l;
		maidenhead[0 + i] = c + 'A';
		l2 = c;
		l -= l2;
		l *= 10.0;
		c = (uint8_t) l;
		maidenhead[2 + i] = c + '0';
		l2 = c;
		l -= l2;
		l *= 24.0;
		c = (uint8_t) l;
		maidenhead[4 + i] = c + 'A';

#if 0
		if (extended)
		{
			l2 = c;
			l -= l2;
			l *= 10.0;
			c = (uint8_t) l;
			maidenhead[6 + i] = c + '0';
			l2 = c;
			l -= l2;
			l *= 24.0;
			c = (uint8_t) l;
			maidenhead[8 + i] = c + (extended ? 'A' : 'a');
			l2 = c;
			l -= l2;
			l *= 10.0;
			c = (uint8_t) l;
			maidenhead[10 + i] = c + '0';
			l2 = c;
			l -= l2;
			l *= 24.0;
			c = (uint8_t) l;
			maidenhead[12 + i] = c + (extended ? 'A' : 'a');
		}
#endif

		l = latitude;
	}

#if 0
	maidenhead[extended ? 14 : 6] = '\0';
#else
	maidenhead[6] = '\0';
#endif

	return &maidenhead[0];
}

static uint8_t *decodeGPSPosition(uint8_t *data)
{
#if 0
	uint8_t errorI = (data[2U] & 0x0E) >> 1U;
	const char* error;
	switch (errorI) {
	case 0U:
		error = "< 2m";
		break;
	case 1U:
		error = "< 20m";
		break;
	case 2U:
		error = "< 200m";
		break;
	case 3U:
		error = "< 2km";
		break;
	case 4U:
		error = "< 20km";
		break;
	case 5U:
		error = "< 200km";
		break;
	case 6U:
		error = "> 200km";
		break;
	default:
		error = "not known";
		break;
	}
#endif

	int32_t longitudeI = ((data[2U] & 0x01U) << 31) | (data[3U] << 23) | (data[4U] << 15) | (data[5U] << 7);
	longitudeI >>= 7;

	int32_t latitudeI = (data[6U] << 24) | (data[7U] << 16) | (data[8U] << 8);
	latitudeI >>= 8;

	float longitude = 360.0F / 33554432.0F;	// 360/2^25 steps
	float latitude  = 180.0F / 16777216.0F;	// 180/2^24 steps

	longitude *= (float)longitudeI;
	latitude  *= (float)latitudeI;

	return (coordsToMaidenhead(longitude, latitude));
}

static uint8_t *decodeTA(uint8_t *TA)
{
	uint8_t *b;
	uint8_t c;
	int8_t j;
	uint8_t i, t1, t2;
	static uint8_t buffer[32];
	uint8_t *talkerAlias = TA;
	uint8_t TAformat = (talkerAlias[0] >> 6U) & 0x03U;
	uint8_t TAsize   = (talkerAlias[0] >> 1U) & 0x1FU;

	switch (TAformat)
	{
	case 0U:		// 7 bit
	memset(&buffer, 0, sizeof(buffer));
	b = &talkerAlias[0];
	t1 = 0U; t2 = 0U; c = 0U;

	for (i = 0U; (i < 32U) && (t2 < TAsize); i++)
	{
		for (j = 7; j >= 0; j--)
		{
			c = (c << 1U) | (b[i] >> j);

			if (++t1 == 7U)
			{
				if (i > 0U)
				{
					buffer[t2++] = c & 0x7FU;
				}

				t1 = 0U;
				c = 0U;
			}
		}
	}
	buffer[TAsize] = 0;
	break;

	case 1U:		// ISO 8 bit
	case 2U:		// UTF8
		memcpy(&buffer, talkerAlias + 1U, sizeof(buffer));
		break;

	case 3U:		// UTF16 poor man's conversion
		t2=0;
		memset(&buffer, 0, sizeof(buffer));
		for (i = 0U; (i < 15U) && (t2 < TAsize); i++)
		{
			if (talkerAlias[2U * i + 1U] == 0)
			{
				buffer[t2++] = talkerAlias[2U * i + 2U];
			}
			else
			{
				buffer[t2++] = '?';
			}
		}
		buffer[TAsize] = 0;
		break;
	}

	return &buffer[0];
}

void lastHeardClearLastID(void)
{
	lastID = 0;
}

static void updateLHItem(LinkItem_t *item)
{
	static const int bufferLen = 33; // displayChannelNameOrRxFrequency() use 6x8 font
	char buffer[bufferLen];// buffer passed to the DMR ID lookup function, needs to be large enough to hold worst case text length that is returned. Currently 16+1
	dmrIdDataStruct_t currentRec;

	if ((item->talkGroupOrPcId >> 24) == PC_CALL_FLAG)
	{
		// Its a Private call
		switch (nonVolatileSettings.contactDisplayPriority)
		{
		case CONTACT_DISPLAY_PRIO_CC_DB_TA:
		case CONTACT_DISPLAY_PRIO_TA_CC_DB:
			if (contactIDLookup(item->id, CONTACT_CALLTYPE_PC, buffer) == true)
			{
				snprintf(item->contact, SCREEN_LINE_BUFFER_SIZE, "%s", buffer);
			}
			else
			{
				dmrIDLookup(item->id, &currentRec);
				snprintf(item->contact, SCREEN_LINE_BUFFER_SIZE, "%s", currentRec.text);
			}
			break;

		case CONTACT_DISPLAY_PRIO_DB_CC_TA:
		case CONTACT_DISPLAY_PRIO_TA_DB_CC:
			if (dmrIDLookup(item->id, &currentRec) == true)
			{
				snprintf(item->contact, SCREEN_LINE_BUFFER_SIZE, "%s", currentRec.text);
			}
			else
			{
				if (contactIDLookup(item->id, CONTACT_CALLTYPE_PC, buffer) == true)
				{
					snprintf(item->contact, SCREEN_LINE_BUFFER_SIZE, "%s", buffer);
				}
				else
				{
					snprintf(item->contact, SCREEN_LINE_BUFFER_SIZE, "%s", currentRec.text);
				}
			}
			break;
		}

		if (item->talkGroupOrPcId != (trxDMRID | (PC_CALL_FLAG << 24)))
		{
			if (contactIDLookup(item->talkGroupOrPcId & 0x00FFFFFF, CONTACT_CALLTYPE_PC, buffer) == true)
			{
				snprintf(item->talkgroup, SCREEN_LINE_BUFFER_SIZE, "%s", buffer);
			}
			else
			{
				dmrIDLookup(item->talkGroupOrPcId & 0x00FFFFFF, &currentRec);
				snprintf(item->talkgroup, SCREEN_LINE_BUFFER_SIZE, "%s", currentRec.text);
			}
		}
	}
	else
	{
		// TalkGroup
		if (contactIDLookup(item->talkGroupOrPcId, CONTACT_CALLTYPE_TG, buffer) == true)
		{
			snprintf(item->talkgroup, SCREEN_LINE_BUFFER_SIZE, "%s", buffer);
		}
		else
		{
			snprintf(item->talkgroup, SCREEN_LINE_BUFFER_SIZE, "%s %u", currentLanguage->tg, (item->talkGroupOrPcId & 0x00FFFFFF));
		}

		switch (nonVolatileSettings.contactDisplayPriority)
		{
		case CONTACT_DISPLAY_PRIO_CC_DB_TA:
		case CONTACT_DISPLAY_PRIO_TA_CC_DB:
			if (contactIDLookup(item->id, CONTACT_CALLTYPE_PC, buffer) == true)
			{
				snprintf(item->contact, MAX_DMR_ID_CONTACT_TEXT_LENGTH, "%s", buffer);
			}
			else
			{
				dmrIDLookup((item->id & 0x00FFFFFF), &currentRec);
				snprintf(item->contact, MAX_DMR_ID_CONTACT_TEXT_LENGTH, "%s", currentRec.text);
			}
			break;

		case CONTACT_DISPLAY_PRIO_DB_CC_TA:
		case CONTACT_DISPLAY_PRIO_TA_DB_CC:
			if (dmrIDLookup((item->id & 0x00FFFFFF), &currentRec) == true)
			{
				snprintf(item->contact, MAX_DMR_ID_CONTACT_TEXT_LENGTH, "%s", currentRec.text);
			}
			else
			{
				if (contactIDLookup(item->id, CONTACT_CALLTYPE_PC, buffer) == true)
				{
					snprintf(item->contact, MAX_DMR_ID_CONTACT_TEXT_LENGTH, "%s", buffer);
				}
				else
				{
					snprintf(item->contact, MAX_DMR_ID_CONTACT_TEXT_LENGTH, "%s", currentRec.text);
				}
			}
			break;
		}
	}
}

bool lastHeardListUpdate(uint8_t *dmrDataBuffer, bool forceOnHotspot)
{
	static uint8_t bufferTA[32];
	static uint8_t blocksTA = 0x00;
	bool retVal = false;
	uint32_t talkGroupOrPcId = (dmrDataBuffer[0] << 24) + (dmrDataBuffer[3] << 16) + (dmrDataBuffer[4] << 8) + (dmrDataBuffer[5] << 0);
	static bool overrideTA = false;

	if ((HRC6000GetReceivedTgOrPcId() != 0) || forceOnHotspot)
	{
		if (dmrDataBuffer[0] == TG_CALL_FLAG || dmrDataBuffer[0] == PC_CALL_FLAG)
		{
			uint32_t id = (dmrDataBuffer[6] << 16) + (dmrDataBuffer[7] << 8) + (dmrDataBuffer[8] << 0);

			if ((id != 0) && (talkGroupOrPcId != 0))
			{
				if (id != lastID)
				{
					memset(bufferTA, 0, 32);// Clear any TA data in TA buffer (used for decode)
					blocksTA = 0x00;
					overrideTA = false;
					lastHeardNeedsAnnouncementTimer = (id !=trxDMRID) ? LAST_HEARD_TIMER_TIMEOUT : -1;
					lastHeardUpdateTime=fw_millis();

					retVal = true;// something has changed
					lastID = id;

					LinkItem_t *item = lastheardFindInList(id);

					// Already in the list
					if (item != NULL)
					{
						if (item->talkGroupOrPcId != talkGroupOrPcId)
						{
							item->talkGroupOrPcId = talkGroupOrPcId; // update the TG in case they changed TG
							updateLHItem(item);
						}

						item->time = fw_millis();
						lastTG = talkGroupOrPcId;

						if (item == LinkHead)
						{
							uiDataGlobal.displayQSOState = QSO_DISPLAY_CALLER_DATA;// flag that the display needs to update
							return true;// already at top of the list
						}
						else
						{
							// not at top of the list
							// Move this item to the top of the list
							LinkItem_t *next = item->next;
							LinkItem_t *prev = item->prev;

							// set the previous item to skip this item and link to 'items' next item.
							prev->next = next;

							if (item->next != NULL)
							{
								// not the last in the list
								next->prev = prev;// backwards link the next item to the item before us in the list.
							}

							item->next = LinkHead;// link our next item to the item at the head of the list

							LinkHead->prev = item;// backwards link the hold head item to the item moving to the top of the list.

							item->prev = NULL;// change the items prev to NULL now we are at teh top of the list
							LinkHead = item;// Change the global for the head of the link to the item that is to be at the top of the list.
							if (item->talkGroupOrPcId != 0)
							{
								uiDataGlobal.displayQSOState = QSO_DISPLAY_CALLER_DATA;// flag that the display needs to update
							}
						}
					}
					else
					{
						// Not in the list
						item = LinkHead;// setup to traverse the list from the top.

						if (uiDataGlobal.lastHeardCount < NUM_LASTHEARD_STORED)
						{
							uiDataGlobal.lastHeardCount++;
						}

						// need to use the last item in the list as the new item at the top of the list.
						// find last item in the list
						while(item->next != NULL)
						{
							item = item->next;
						}
						//item is now the last

						(item->prev)->next = NULL;// make the previous item the last

						LinkHead->prev = item;// set the current head item to back reference this item.
						item->next = LinkHead;// set this items next to the current head
						LinkHead = item;// Make this item the new head

						item->id = id;
						item->talkGroupOrPcId = talkGroupOrPcId;
						item->time = fw_millis();
						item->receivedTS = (dmrMonitorCapturedTS != -1) ? dmrMonitorCapturedTS : trxGetDMRTimeSlot();
						lastTG = talkGroupOrPcId;

						memset(item->contact, 0, sizeof(item->contact)); // Clear contact's datas
						memset(item->talkgroup, 0, sizeof(item->talkgroup));
						memset(item->talkerAlias, 0, sizeof(item->talkerAlias));
						memset(item->locator, 0, sizeof(item->locator));

						updateLHItem(item);

						if (item->talkGroupOrPcId != 0)
						{
							uiDataGlobal.displayQSOState = QSO_DISPLAY_CALLER_DATA;// flag that the display needs to update
						}
					}
				}
				else // update TG even if the DMRID did not change
				{
					LinkItem_t *item = lastheardFindInList(id);

					if (lastTG != talkGroupOrPcId)
					{
						if (item != NULL)
						{
							// Already in the list
							item->talkGroupOrPcId = talkGroupOrPcId;// update the TG in case they changed TG
							updateLHItem(item);
							item->time = fw_millis();
						}

						lastTG = talkGroupOrPcId;
						memset(bufferTA, 0, 32);// Clear any TA data in TA buffer (used for decode)
						blocksTA = 0x00;
						overrideTA = false;
						retVal = true;// something has changed
					}

					item->receivedTS = (dmrMonitorCapturedTS != -1) ? dmrMonitorCapturedTS : trxGetDMRTimeSlot();// Always update this in case the TS changed.
				}
			}
		}
		else
		{
			// Data contains the Talker Alias Data
			uint8_t blockID = (forceOnHotspot ? dmrDataBuffer[0] : DMR_frame_buffer[0]);

			if (blockID >= 4)
			{
				blockID -= 4; // shift the blockID for maths reasons

				if (blockID < 4) // ID 0x04..0x07: TA
				{

					// Already stored first byte in block TA Header has changed, lets clear other blocks too
					if ((blockID == 0) && ((blocksTA & (1 << blockID)) != 0) &&
							(bufferTA[0] != (forceOnHotspot ? dmrDataBuffer[2] : DMR_frame_buffer[2])))
					{
						blocksTA &= ~(1 << 0);

						// Clear all other blocks if they're already stored
						if ((blocksTA & (1 << 1)) != 0)
						{
							blocksTA &= ~(1 << 1);
							memset(bufferTA + 7, 0, 7); // Clear 2nd TA block
						}
						if ((blocksTA & (1 << 2)) != 0)
						{
							blocksTA &= ~(1 << 2);
							memset(bufferTA + 14, 0, 7); // Clear 3rd TA block
						}
						if ((blocksTA & (1 << 3)) != 0)
						{
							blocksTA &= ~(1 << 3);
							memset(bufferTA + 21, 0, 7); // Clear 4th TA block
						}
						overrideTA = true;
					}

					// We don't already have this TA block
					if ((blocksTA & (1 << blockID)) == 0)
					{
						static const uint8_t blockLen = 7;
						uint32_t blockOffset = blockID * blockLen;

						blocksTA |= (1 << blockID);

						if ((blockOffset + blockLen) < sizeof(bufferTA))
						{
							memcpy(bufferTA + blockOffset, (void *)(forceOnHotspot ? &dmrDataBuffer[2] : &DMR_frame_buffer[2]), blockLen);

							// Format and length infos are available, we can decode now
							if (bufferTA[0] != 0x0)
							{
								uint8_t *decodedTA;

								if ((decodedTA = decodeTA(&bufferTA[0])) != NULL)
								{
									// TAs doesn't match, update contact and screen.
									if (overrideTA || (strlen((const char *)decodedTA) > strlen((const char *)&LinkHead->talkerAlias)))
									{
										memcpy(&LinkHead->talkerAlias, decodedTA, 31);// Brandmeister seems to send callsign as 6 chars only

										if ((blocksTA & (1 << 1)) != 0) // we already received the 2nd TA block, check for 'DMR ID:'
										{
											char *p = NULL;

											// Get rid of 'DMR ID:xxxxxxx' part of the TA, sent by BM
											if (((p = strstr(&LinkHead->talkerAlias[0], "DMR ID:")) != NULL) || ((p = strstr(&LinkHead->talkerAlias[0], "DMR I")) != NULL))
											{
												*p = 0;
											}
										}

										overrideTA = false;
										uiDataGlobal.displayQSOState = QSO_DISPLAY_CALLER_DATA;
									}
								}
							}
						}
					}
				}
				else if (blockID == 4) // ID 0x08: GPS
				{
					uint8_t *locator = decodeGPSPosition((uint8_t *)(forceOnHotspot ? &dmrDataBuffer[0] : &DMR_frame_buffer[0]));

					if (strncmp((char *)&LinkHead->locator, (char *)locator, 7) != 0)
					{
						memcpy(&LinkHead->locator, locator, 7);
						uiDataGlobal.displayQSOState = QSO_DISPLAY_CALLER_DATA_UPDATE;
					}
				}
			}
		}
	}

	return retVal;
}


static void dmrIDReadContactInFlash(uint32_t contactOffset, uint8_t *data, uint32_t len)
{
	uint32_t address;
	if (contactOffset > dmrIdDataArea_1_Size)
	{
		address = dmrIDDatabaseMemoryLocation2 + contactOffset - dmrIdDataArea_1_Size;
	}
	else
	{
		address = DMRID_MEMORY_LOCATION_1 + DMRID_HEADER_LENGTH + contactOffset;
	}
	SPI_Flash_read(address, data, len);
}


void dmrIDCacheInit(void)
{
	uint8_t headerBuf[32];

	memset(&dmrIDsCache, 0, sizeof(dmrIDsCache_t));
	memset(&headerBuf, 0, sizeof(headerBuf));

	SPI_Flash_read(DMRID_MEMORY_LOCATION_1, headerBuf, DMRID_HEADER_LENGTH);

	if ((headerBuf[0] != 'I') || (headerBuf[1] != 'D') )
	{
		return;
	}

	if (headerBuf[2] == 'N' || headerBuf[2] == 'n')
	{
		DMRID_IdLength = 3U;// default is 4
		if (headerBuf[2] == 'n')
		{
			dmrIDDatabaseMemoryLocation2 = VOICE_PROMPTS_FLASH_HEADER_ADDRESS; // overwrite the VP
		}
	}

	dmrIDsCache.contactLength = (uint8_t)headerBuf[3] - 0x4a;
	// Check that data in DMR ID DB does not have a larger record size than the code has
	if (dmrIDsCache.contactLength > sizeof(dmrIdDataStruct_t))
	{
		return;
	}

	dmrIDsCache.entries = ((uint32_t)headerBuf[8] | (uint32_t)headerBuf[9] << 8 | (uint32_t)headerBuf[10] << 16 | (uint32_t)headerBuf[11] << 24);

	dmrIdDataArea_1_Size = (dmrIDsCache.contactLength * ((0x40000 - DMRID_HEADER_LENGTH) / dmrIDsCache.contactLength));// Size of number of complete DMR ID records

	if (dmrIDsCache.entries > 0)
	{
		dmrIdDataStruct_t dmrIDContact;

		// Set Min and Max IDs boundaries
		// First available ID

		dmrIDContact.id = 0;
		dmrIDReadContactInFlash(0, (uint8_t *)&dmrIDContact, DMRID_IdLength);
		dmrIDsCache.slices[0] = dmrIDContact.id;

		// Last available ID
		dmrIDContact.id = 0;
		dmrIDReadContactInFlash((dmrIDsCache.contactLength * (dmrIDsCache.entries - 1)), (uint8_t *)&dmrIDContact, DMRID_IdLength);
		dmrIDsCache.slices[ID_SLICES - 1] = dmrIDContact.id;

		if (dmrIDsCache.entries > MIN_ENTRIES_BEFORE_USING_SLICES)
		{
			dmrIDsCache.IDsPerSlice = dmrIDsCache.entries / (ID_SLICES - 1);

			for (uint8_t i = 0; i < (ID_SLICES - 2); i++)
			{
				dmrIDContact.id = 0;
				dmrIDReadContactInFlash((dmrIDsCache.contactLength * ((dmrIDsCache.IDsPerSlice * i) + dmrIDsCache.IDsPerSlice)), (uint8_t *)&dmrIDContact, DMRID_IdLength);
				dmrIDsCache.slices[i + 1] = dmrIDContact.id;
			}
		}
	}
}
static void dmrDbTextDecode(uint8_t *decompressedBufOut, uint8_t *compressedBufIn, int compressedSize)
{
	uint8_t *outPtr = decompressedBufOut;
	uint8_t cb1, cb2, cb3;
	int d = 0;
	do
	{
		cb1 = compressedBufIn[d++];
		*outPtr++ = DECOMPRESS_LUT[cb1 >> 2];//A
		if (d == compressedSize)
		{
			break;
		}
		cb2 = compressedBufIn[d++];
		*outPtr++ = DECOMPRESS_LUT[((cb1 & 0x03) << 4) + (cb2 >> 4)];//B
		if (d == compressedSize)
		{
			break;
		}
		cb3 = compressedBufIn[d++];
		*outPtr++ = DECOMPRESS_LUT[((cb2 & 0x0F) << 2) + (cb3 >> 6)];//C
		*outPtr++ = DECOMPRESS_LUT[cb3 & 0x3F];//D

	} while (d < compressedSize);

	// algorithm can result in a extra space at the end of the decompressed string
	// so trim the string
	uint32_t l = (outPtr - decompressedBufOut);
	if (l)
	{
		uint8_t *p = ((decompressedBufOut + l) - 1);
		while ((p >= decompressedBufOut) && (*p == ' '))
		{
			*p-- = 0;
		}
	}
}

/********* DMR ID db contact encoder.
// This should return a value between 0 and 63, 0xff means char not found.
static uint8_t GetLUTIndexForChar(char ch)
{
	for (uint8_t index=0; index < 64; ++index)
	{
		if (ch==DECOMPRESS_LUT[index])
			return index;
	}
	return 0xff;
}

// dmrDbTextEncode does the opposite of dmrDbTextDecode
// I.e. it encodes each char in the input sequence as 6-bits and compresses 4 chars into 3 byte sequences. 
static uint8_t dmrDbTextEncode(uint8_t *compressedBufOut, uint8_t compressedBufSize, uint8_t *decompressedBufIn, uint8_t decompressedSize)
{
	uint8_t compressedCharSequenceIndex=0;
	uint8_t* outPtr=compressedBufOut;
	memset(compressedBufOut, 0, compressedBufSize);
	
	for (uint8_t i=0; i < decompressedSize; ++i)
	{
		uint8_t lutIndex=GetLUTIndexForChar(decompressedBufIn[i]);
		if (lutIndex==0xff)
		continue;
				
		compressedCharSequenceIndex++;
		switch (compressedCharSequenceIndex)
		{
			case 1:
				*outPtr=lutIndex<<2; // move 6 bits to highest pos of first byte of output sequence.
				break;
			case 2:
				*outPtr++|=((lutIndex&0x30)>>4); // move highest 2 bits of next char to lowest pos of first byte to combine with 6 bits which we already have.
				*outPtr=((lutIndex&0xf)<<4); // move last 4 bits of 2nd char to hiest pos of 2nd byte.
				break;
			case 3:
				*outPtr++|=((lutIndex>>2)); // move first four bits of third char to low nibble of 2nd byte of compressed output.
				*outPtr=(lutIndex<<6); // move last two bits of third char to highest pos of 3rd byte of output.
				break;
			case 4:
				*outPtr++|=lutIndex; // move 4th char to lowest six bits of third byte of output sequence.
				compressedCharSequenceIndex=0;// start the next sequence.
				break;
		}// switch
		if (((outPtr-compressedBufOut)==compressedBufSize) && (compressedCharSequenceIndex!=3)) // let the fourth char be written if possible since we won't overflow the buffer.
		break;
	}// for
	return SAFE_MIN((outPtr-compressedBufOut)+1, compressedBufSize); // number of bytes written to the compressed buffer.
}	
*/

bool dmrIDLookup(uint32_t targetId, dmrIdDataStruct_t *foundRecord)
{
	uint32_t targetIdBCD;

	if (DMRID_IdLength == 4U)
	{
		targetIdBCD = int2bcd(targetId);
	}
	else
	{
		targetIdBCD = targetId;
	}


	if ((dmrIDsCache.entries > 0) && (targetIdBCD >= dmrIDsCache.slices[0]) && (targetIdBCD <= dmrIDsCache.slices[ID_SLICES - 1]))
	{
		uint32_t startPos = 0;
		uint32_t endPos = dmrIDsCache.entries - 1;
		uint32_t curPos;

		// Contact's text length == (dmrIDsCache.contactLength - DMRID_IdLength) aren't NULL terminated,
		// so clearing the whole destination array is mandatory
		memset(foundRecord->text, 0, sizeof(foundRecord->text));

		uint8_t compressedBuf[MAX_DMR_ID_CONTACT_TEXT_LENGTH];// worst case length with no compression

		if (dmrIDsCache.entries > MIN_ENTRIES_BEFORE_USING_SLICES) // Use slices
		{
			for (uint8_t i = 0; i < ID_SLICES - 1; i++)
			{
				// Check if ID is in slices boundaries, with a special case for the last slice as [ID_SLICES - 1] is the last ID
				if ((targetIdBCD >= dmrIDsCache.slices[i]) &&
						((i == ID_SLICES - 2) ? (targetIdBCD <= dmrIDsCache.slices[i + 1]) : (targetIdBCD < dmrIDsCache.slices[i + 1])))
				{
					// targetID is the min slice limit, don't go further
					if (targetIdBCD == dmrIDsCache.slices[i])
					{
						foundRecord->id = dmrIDsCache.slices[i];

						dmrIDReadContactInFlash((dmrIDsCache.contactLength * (dmrIDsCache.IDsPerSlice * i)) + DMRID_IdLength, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
						if (DMRID_IdLength == 3U)
						{
							dmrDbTextDecode((uint8_t *)foundRecord->text, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
						}
						else
						{
							memcpy((uint8_t *)foundRecord->text, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
						}
						return true;
					}

					startPos = dmrIDsCache.IDsPerSlice * i;
					endPos = (i == ID_SLICES - 2) ? (dmrIDsCache.entries - 1) : dmrIDsCache.IDsPerSlice * (i + 1);

					break;
				}
			}
		}
		else // Not enough contact to use slices
		{
			bool isMin;

			// Check if targetID is equal to the first or the last in the IDs list
			if ((isMin = (targetIdBCD == dmrIDsCache.slices[0])) || (targetIdBCD == dmrIDsCache.slices[ID_SLICES - 1]))
			{
				foundRecord->id = dmrIDsCache.slices[(isMin ? 0 : (ID_SLICES - 1))];

				dmrIDReadContactInFlash((dmrIDsCache.contactLength * (dmrIDsCache.IDsPerSlice * (isMin ? 0 : (ID_SLICES - 1)))) + DMRID_IdLength, (uint8_t *) &compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));

				if (DMRID_IdLength == 3U)
				{
					dmrDbTextDecode((uint8_t *)foundRecord->text, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
				}
				else
				{
					memcpy((uint8_t *)foundRecord->text, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
				}

				return true;
			}
		}

		// Look for the ID now
		while (startPos <= endPos)
		{
			curPos = (startPos + endPos) >> 1;

			foundRecord->id = 0;
			dmrIDReadContactInFlash((dmrIDsCache.contactLength * curPos), (uint8_t *)foundRecord, DMRID_IdLength);

			if (foundRecord->id < targetIdBCD)
			{
				startPos = curPos + 1;
			}
			else if (foundRecord->id > targetIdBCD)
			{
				endPos = curPos - 1;
			}
			else
			{
				dmrIDReadContactInFlash((dmrIDsCache.contactLength * curPos) + DMRID_IdLength, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));

				if (DMRID_IdLength == 3U)
				{
					dmrDbTextDecode((uint8_t *)foundRecord->text, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
				}
				else
				{
					memcpy((uint8_t *)foundRecord->text, (uint8_t *)&compressedBuf, (dmrIDsCache.contactLength - DMRID_IdLength));
				}

				return true;
			}
		}
	}

	snprintf(foundRecord->text, MAX_DMR_ID_CONTACT_TEXT_LENGTH, "ID:%d", targetId);
	return false;
}

bool contactIDLookup(uint32_t id, uint32_t calltype, char *buffer)
{
	struct_codeplugContact_t contact;
	int8_t manTS = tsGetManualOverrideFromCurrentChannel();

	int contactIndex = codeplugContactIndexByTGorPC((id & 0x00FFFFFF), calltype, &contact,
			((monitorModeData.isEnabled && (dmrMonitorCapturedTS != -1)) ? (dmrMonitorCapturedTS + 1) :
			(manTS ? manTS : (trxGetDMRTimeSlot() + 1))));

	if (contactIndex != -1)
	{
		codeplugUtilConvertBufToString(contact.name, buffer, 16);
		return true;
	}

	return false;
}

static void displayChannelNameOrRxFrequency(char *buffer, size_t maxLen)
{
	if (menuSystemGetCurrentMenuNumber() == UI_CHANNEL_MODE)
	{
		codeplugUtilConvertBufToString(currentChannelData->name, buffer, 16);
	}
	else
	{
		int val_before_dp = currentChannelData->rxFreq / 100000;
		int val_after_dp = currentChannelData->rxFreq - val_before_dp * 100000;
		snprintf(buffer, maxLen, "%d.%05d MHz", val_before_dp, val_after_dp);
	}

	uiUtilityDisplayInformation(buffer, DISPLAY_INFO_ZONE, -1);
}

static void displaySplitOrSpanText(uint8_t y, char *text)
{
	if (text != NULL)
	{
		uint8_t len = strlen(text);

		if (len == 0)
		{
			return;
		}
		else if (len <= 16)
		{
			ucPrintCentered(y, text, FONT_SIZE_3);
		}
		else
		{
			uint8_t nLines = len / 21 + (((len % 21) != 0) ? 1 : 0);

			if (nLines > 2)
			{
				nLines = 2;
				len = 42; // 2 lines max.
			}

			if (nLines > 1)
			{
				char buffer[MAX_DMR_ID_CONTACT_TEXT_LENGTH]; // to fit 2 * 21 chars + NULL, but needs larger

				memcpy(buffer, text, len + 1);
				buffer[len + 1] = 0;

				char *p = buffer + 20;

				// Find a space backward
				while ((*p != ' ') && (p > buffer))
				{
					p--;
				}

				uint8_t rest = (uint8_t)((buffer + strlen(buffer)) - p) - ((*p == ' ') ? 1 : 0);

				// rest is too long, just split the line in two chunks
				if (rest > 21)
				{
					char c = buffer[21];

					buffer[21] = 0;

					ucPrintCentered(y, chomp(buffer), FONT_SIZE_1); // 2 pixels are saved, could center

					buffer[21] = c;

					ucPrintCentered(y + 8, chomp(buffer + 21), FONT_SIZE_1);
				}
				else
				{
					*p = 0;

					ucPrintCentered(y, chomp(buffer), FONT_SIZE_1);
					ucPrintCentered(y + 8, chomp(p + 1), FONT_SIZE_1);
				}
			}
			else // One line of 21 chars max
			{
				ucPrintCentered(y
#if ! defined(PLATFORM_RD5R)
						+ 4
#endif
						, text, FONT_SIZE_1);
			}
		}
	}
}


/*
 * Try to extract callsign and extra text from TA or DMR ID data, then display that on
 * two lines, if possible.
 * We don't care if extra text is larger than 16 chars, ucPrint*() functions cut them.
 *.
 */
static void displayContactTextInfos(char *text, size_t maxLen, bool isFromTalkerAlias)
{
	// Max for TalkerAlias is 37: TA 27 (in 7bit format) + ' [' + 6 (Maidenhead)  + ']' + NULL
	// Max for DMRID Database is MAX_DMR_ID_CONTACT_TEXT_LENGTH (50 + NULL)
	char buffer[MAX_DMR_ID_CONTACT_TEXT_LENGTH];

	if (strlen(text) >= 5 && isFromTalkerAlias) // if it's Talker Alias and there is more text than just the callsign, split across 2 lines
	{
		char    *pbuf;
		int32_t  cpos;

		// User prefers to not span the TA info over two lines, check it that could fit
		if ((nonVolatileSettings.splitContact == SPLIT_CONTACT_SINGLE_LINE_ONLY) ||
				((nonVolatileSettings.splitContact == SPLIT_CONTACT_AUTO) && (strlen(text) <= 16)))
		{
			memcpy(buffer, text, 16);
			buffer[16] = 0;

			uiUtilityDisplayInformation(chomp(buffer), DISPLAY_INFO_CHANNEL, -1);
			displayChannelNameOrRxFrequency(buffer, (sizeof(buffer) / sizeof(buffer[0])));
			return;
		}

		if ((cpos = getFirstSpacePos(text)) != -1)
		{
			// Callsign found
			memcpy(buffer, text, cpos);
			buffer[cpos] = 0;

			uiUtilityDisplayInformation(chomp(buffer), DISPLAY_INFO_CHANNEL, -1);

			memcpy(buffer, text + (cpos + 1), (maxLen - (cpos + 1)));
			buffer[(maxLen - (cpos + 1))] = 0;

			pbuf = chomp(buffer);

			if (strlen(pbuf))
			{
				displaySplitOrSpanText(DISPLAY_Y_POS_CHANNEL_SECOND_LINE, pbuf);
			}
			else
			{
				displayChannelNameOrRxFrequency(buffer, (sizeof(buffer) / sizeof(buffer[0])));
			}
		}
		else
		{
			// No space found, use a chainsaw
			memcpy(buffer, text, 16);
			buffer[16] = 0;

			uiUtilityDisplayInformation(chomp(buffer), DISPLAY_INFO_CHANNEL, -1);

			memcpy(buffer, text + 16, (maxLen - 16));
			buffer[(strlen(text) - 16)] = 0;

			pbuf = chomp(buffer);

			if (strlen(pbuf))
			{
				displaySplitOrSpanText(DISPLAY_Y_POS_CHANNEL_SECOND_LINE, pbuf);
			}
			else
			{
				displayChannelNameOrRxFrequency(buffer, (sizeof(buffer) / sizeof(buffer[0])));
			}
		}
	}
	else
	{
		memcpy(buffer, text, 16);
		buffer[16] = 0;

		uiUtilityDisplayInformation(chomp(buffer), DISPLAY_INFO_CHANNEL, -1);
		displayChannelNameOrRxFrequency(buffer, (sizeof(buffer) / sizeof(buffer[0])));
	}
}

void uiUtilityDisplayInformation(const char *str, displayInformation_t line, int8_t yOverride)
{
	bool inverted = false;

	switch (line)
	{
	case DISPLAY_INFO_CONTACT_INVERTED:
#if defined(PLATFORM_RD5R)
		ucFillRect(0, DISPLAY_Y_POS_CONTACT + 1, DISPLAY_SIZE_X, MENU_ENTRY_HEIGHT, false);
#else
		ucClearRows(2, 4, true);
#endif
		inverted = true;
	case DISPLAY_INFO_CONTACT:
		ucPrintCore(0, ((yOverride == -1) ? (DISPLAY_Y_POS_CONTACT + V_OFFSET) : yOverride), str, FONT_SIZE_3, TEXT_ALIGN_CENTER, inverted);
		break;

	case DISPLAY_INFO_CONTACT_OVERRIDE_FRAME:
		ucDrawRect(0, ((yOverride == -1) ? DISPLAY_Y_POS_CONTACT : yOverride), DISPLAY_SIZE_X, OVERRIDE_FRAME_HEIGHT, true);
		break;

	case DISPLAY_INFO_CHANNEL:
		ucPrintCentered(((yOverride == -1) ? DISPLAY_Y_POS_CHANNEL_FIRST_LINE : yOverride), str, FONT_SIZE_3);
		break;

	case DISPLAY_INFO_SQUELCH:
	{
		static const int xbar = 74; // 128 - (51 /* max squelch px */ + 3);
		int sLen = (strlen(str) * 8);

		// Center squelch word between col0 and bargraph, if possible.
		ucPrintAt(0 + ((sLen) < xbar - 2 ? (((xbar - 2) - (sLen)) >> 1) : 0), DISPLAY_Y_POS_SQUELCH_BAR, str, FONT_SIZE_3);

		int bargraph = 1 + ((currentChannelData->sql - 1) * 5) / 2;
		ucDrawRect(xbar - 2, DISPLAY_Y_POS_SQUELCH_BAR, 55, SQUELCH_BAR_H + 4, true);
		ucFillRect(xbar, DISPLAY_Y_POS_SQUELCH_BAR + 2, bargraph, SQUELCH_BAR_H, false);
	}
	break;

	case DISPLAY_INFO_TONE_AND_SQUELCH:
	{
		static const int bufLen = 24;
		char buf[bufLen];
		char *p = buf;

		if (trxGetMode() == RADIO_MODE_ANALOG)
		{
			CodeplugCSSTypes_t type = codeplugGetCSSType(currentChannelData->rxTone);

			p += snprintf(p, bufLen, "Rx:");

			if (type == CSS_TYPE_CTCSS)
			{
				p += snprintf(p, bufLen - (p - buf), "%d.%dHz", currentChannelData->rxTone / 10 , currentChannelData->rxTone % 10);
			}
			else if (type & CSS_TYPE_DCS)
			{
				p += dcsPrintf(p, bufLen - (p - buf), NULL, currentChannelData->rxTone);
			}
			else
			{
				p += snprintf(p, bufLen - (p - buf), "%s", currentLanguage->none);
			}

			p += snprintf(p, bufLen - (p - buf), "|Tx:");

			type = codeplugGetCSSType(currentChannelData->txTone);
			if (type == CSS_TYPE_CTCSS)
			{
				p += snprintf(p, bufLen - (p - buf), "%d.%dHz", currentChannelData->txTone / 10 , currentChannelData->txTone % 10);
			}
			else if (type & CSS_TYPE_DCS)
			{
				p += dcsPrintf(p, bufLen - (p - buf), NULL, currentChannelData->txTone);
			}
			else
			{
				p += snprintf(p, bufLen - (p - buf), "%s", currentLanguage->none);
			}
			ucPrintCentered(DISPLAY_Y_POS_CSS_INFO, buf, FONT_SIZE_1);

			snprintf(buf, bufLen, "SQL:%d%%", 5 * (((currentChannelData->sql == 0) ? nonVolatileSettings.squelchDefaults[trxCurrentBand[TRX_RX_FREQ_BAND]] : currentChannelData->sql)-1));
			ucPrintCentered(DISPLAY_Y_POS_SQL_INFO, buf, FONT_SIZE_1);
		}
	}
	break;

	case DISPLAY_INFO_SQUELCH_CLEAR_AREA:
#if defined(PLATFORM_RD5R)
		ucFillRect(0, DISPLAY_Y_POS_SQUELCH_BAR, DISPLAY_SIZE_X, 9, true);
#else
		ucClearRows(2, 4, false);
#endif
		break;

	case DISPLAY_INFO_TX_TIMER:
	{
		if (encodingCustomVoicePrompt)
		{
			char recBuf[SCREEN_LINE_BUFFER_SIZE];
			snprintf(recBuf, SCREEN_LINE_BUFFER_SIZE, "RVP %s", str);
			ucPrintCentered(DISPLAY_Y_POS_TX_TIMER, recBuf, FONT_SIZE_4);
		}
		else
			ucPrintCentered(DISPLAY_Y_POS_TX_TIMER, str, FONT_SIZE_4);
		break;
	}
	case DISPLAY_INFO_ZONE:
		ucPrintCentered(DISPLAY_Y_POS_ZONE, str, FONT_SIZE_1);
		break;
	}
}

void uiUtilityRenderQSODataAndUpdateScreen(void)
{
	if (isQSODataAvailableForCurrentTalker())
	{
		ucClearBuf();
		uiUtilityRenderHeader(notScanning);
		uiUtilityRenderQSOData();
		ucRender();
	}
}

void uiUtilityRenderQSOData(void)
{
	uiDataGlobal.receivedPcId = 0x00; //reset the received PcId

	/*
	 * Note.
	 * When using Brandmeister reflectors. TalkGroups can be used to select reflectors e.g. TG 4009, and TG 5000 to check the connnection
	 * Under these conditions Brandmeister seems to respond with a message via a private call even if the command was sent as a TalkGroup,
	 * and this caused the Private Message acceptance system to operate.
	 * Brandmeister seems respond on the same ID as the keyed TG, so the code
	 * (LinkHead->id & 0xFFFFFF) != (trxTalkGroupOrPcId & 0xFFFFFF)  is designed to stop the Private call system tiggering in these instances
	 *
	 * FYI. Brandmeister seems to respond with a TG value of the users on ID number,
	 * but I thought it was safer to disregard any PC's from IDs the same as the current TG
	 * rather than testing if the TG is the user's ID, though that may work as well.
	 */
	if (HRC6000GetReceivedTgOrPcId() != 0)
	{
		if ((LinkHead->talkGroupOrPcId >> 24) == PC_CALL_FLAG) // &&  (LinkHead->id & 0xFFFFFF) != (trxTalkGroupOrPcId & 0xFFFFFF))
		{
			// Its a Private call
			ucPrintCentered(16, LinkHead->contact, FONT_SIZE_3);

			ucPrintCentered(DISPLAY_Y_POS_CHANNEL_FIRST_LINE, currentLanguage->private_call, FONT_SIZE_3);

			if (LinkHead->talkGroupOrPcId != (trxDMRID | (PC_CALL_FLAG << 24)))
			{
				uiUtilityDisplayInformation(LinkHead->talkgroup, DISPLAY_INFO_ZONE, -1);
				ucPrintAt(1, DISPLAY_Y_POS_ZONE, "=>", FONT_SIZE_1);
			}
		}
		else
		{
			// Group call
			bool different = (((LinkHead->talkGroupOrPcId & 0xFFFFFF) != trxTalkGroupOrPcId ) ||
					(((trxDMRModeRx != DMR_MODE_DMO) && (dmrMonitorCapturedTS != -1)) && (dmrMonitorCapturedTS != trxGetDMRTimeSlot())) ||
					(trxGetDMRColourCode() != currentChannelData->txColor));

			uiUtilityDisplayInformation(LinkHead->talkgroup, different ? DISPLAY_INFO_CONTACT_INVERTED : DISPLAY_INFO_CONTACT, -1);

			// If voice prompt feedback is enabled. Play a short beep to indicate the inverse video display showing the TG / TS / CC does not match the current Tx config
			if (different && (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1) && settingsIsOptionBitSet(BIT_INDICATE_DMR_RXTXTG_MISMATCH))
			{
				soundSetMelody(MELODY_RX_TGTSCC_WARNING_BEEP);
			}

			switch (nonVolatileSettings.contactDisplayPriority)
			{
			case CONTACT_DISPLAY_PRIO_CC_DB_TA:
			case CONTACT_DISPLAY_PRIO_DB_CC_TA:
				// No contact found in codeplug and DMRIDs, use TA as fallback, if any.
				if ((strncmp(LinkHead->contact, "ID:", 3) == 0) && (LinkHead->talkerAlias[0] != 0x00))
				{
					if (LinkHead->locator[0] != 0)
					{
						char bufferTA[37]; // TA + ' [' + Maidenhead + ']' + NULL

						memset(bufferTA, 0, sizeof(bufferTA));
						snprintf(bufferTA, 37, "%s [%s]", LinkHead->talkerAlias, LinkHead->locator);
						displayContactTextInfos(bufferTA, sizeof(bufferTA), true);
					}
					else
					{
						displayContactTextInfos(LinkHead->talkerAlias, sizeof(LinkHead->talkerAlias), !(nonVolatileSettings.splitContact == SPLIT_CONTACT_SINGLE_LINE_ONLY));
					}
				}
				else
				{
					displayContactTextInfos(LinkHead->contact, sizeof(LinkHead->contact), !(nonVolatileSettings.splitContact == SPLIT_CONTACT_SINGLE_LINE_ONLY));
				}
				break;

			case CONTACT_DISPLAY_PRIO_TA_CC_DB:
			case CONTACT_DISPLAY_PRIO_TA_DB_CC:
				// Talker Alias have the priority here
				if (LinkHead->talkerAlias[0] != 0x00)
				{
					if (LinkHead->locator[0] != 0)
					{
						char bufferTA[37]; // TA + ' [' + Maidenhead + ']' + NULL

						memset(bufferTA, 0, sizeof(bufferTA));
						snprintf(bufferTA, 37, "%s [%s]", LinkHead->talkerAlias, LinkHead->locator);
						displayContactTextInfos(bufferTA, sizeof(bufferTA), true);
					}
					else
					{
						displayContactTextInfos(LinkHead->talkerAlias, sizeof(LinkHead->talkerAlias), !(nonVolatileSettings.splitContact == SPLIT_CONTACT_SINGLE_LINE_ONLY));
					}
				}
				else // No TA, then use the one extracted from Codeplug or DMRIdDB
				{
					displayContactTextInfos(LinkHead->contact, sizeof(LinkHead->contact), !(nonVolatileSettings.splitContact == SPLIT_CONTACT_SINGLE_LINE_ONLY));
				}
				break;
			}
		}
	}
}

void uiUtilityRenderHeader(HeaderScanIndicatorType_t headerScanIndicatorType)
{
	const int MODE_TEXT_X_OFFSET = 1;
	const int FILTER_TEXT_X_OFFSET = 25;
	const int COLOR_CODE_X_POSITION = 84;
	char buffer[SCREEN_LINE_BUFFER_SIZE];
	char scanTypeIndicator[5] = "";
	if (headerScanIndicatorType == vfoDualWatch || headerScanIndicatorType == channelDualWatch)
	{
		strcpy(scanTypeIndicator, "[DW]");
	}
	else if (headerScanIndicatorType == channelPriorityScan)
	{
		strcpy(scanTypeIndicator, "[PR]");
	}

	static bool scanBlinkPhase = true;
	static uint32_t blinkTime = 0;
	int powerLevel = trxGetPowerLevel();
	bool isPerChannelPower = (currentChannelData->libreDMR_Power != 0x00);
	bool scanIsActive = (uiDataGlobal.Scan.active || uiDataGlobal.Scan.toneActive);
	bool batteryIsLow = batteryIsLowWarning();


	if (headerScanIndicatorType == notScanning)
	{
		if (!trxTransmissionEnabled)
		{
			uiUtilityDrawRSSIBarGraph();
		}
		else
		{
			if (trxGetMode() == RADIO_MODE_DIGITAL)
			{
				uiUtilityDrawDMRMicLevelBarGraph();
			}
		}
	}

	if (scanIsActive || batteryIsLow)
	{
		int blinkPeriod = 1000;
		if (scanBlinkPhase)
		{
			blinkPeriod = 500;
		}

		if ((fw_millis() - blinkTime) > blinkPeriod)
		{
			blinkTime = fw_millis();
			scanBlinkPhase = !scanBlinkPhase;
		}
	}
	else
	{
		scanBlinkPhase = false;
	}

	if (headerScanIndicatorType == vfoSweepScan)
	{
		int span = (VFO_SWEEP_SCAN_RANGE_SAMPLE_STEP_TABLE[uiDataGlobal.Scan.sweepStepSizeIndex] * VFO_SWEEP_NUM_SAMPLES) / (VFO_SWEEP_PIXELS_PER_STEP * 100);

		sprintf(buffer, "+/-%dkHz", (span >> 1));
		ucPrintCore(MODE_TEXT_X_OFFSET, DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
	}

	switch(trxGetMode())
	{
	case RADIO_MODE_ANALOG:
		if (headerScanIndicatorType != vfoSweepScan)
		{
			if (*scanTypeIndicator)
			{
				strcpy(buffer, scanTypeIndicator);
			}
			else
			{
				strcpy(buffer, (trxGetBandwidthIs25kHz() ? "FM" : "FMN"));
			}

			ucPrintCore(MODE_TEXT_X_OFFSET, DISPLAY_Y_POS_HEADER, buffer,
					(((nonVolatileSettings.hotspotType != HOTSPOT_TYPE_OFF) && (uiDataGlobal.dmrDisabled == false)) ? FONT_SIZE_1_BOLD : FONT_SIZE_1), TEXT_ALIGN_LEFT, (scanIsActive ? scanBlinkPhase : false));
		}

		if ((monitorModeData.isEnabled == false) && (headerScanIndicatorType != vfoSweepScan) &&
				((currentChannelData->txTone != CODEPLUG_CSS_TONE_NONE) || (currentChannelData->rxTone != CODEPLUG_CSS_TONE_NONE)))
		{
			bool cssTextInverted = (trxGetAnalogFilterLevel() == ANALOG_FILTER_NONE);//(nonVolatileSettings.analogFilterLevel == ANALOG_FILTER_NONE);

			if (currentChannelData->txTone != CODEPLUG_CSS_TONE_NONE)
			{
				strcpy(buffer, ((codeplugGetCSSType(currentChannelData->txTone) & CSS_TYPE_DCS) ? "DT" : "CT"));
			}
			else // tx and/or rx tones are enabled, no need to check for this
			{
				strcpy(buffer, ((codeplugGetCSSType(currentChannelData->rxTone) & CSS_TYPE_DCS) ? "D" : "C"));
			}

			// There is no room to display if rxTone is CTCSS or DCS, when txTone is set.
			if (currentChannelData->rxTone != CODEPLUG_CSS_TONE_NONE)
			{
				strcat(buffer, "R");
			}

			if (cssTextInverted)
			{
				// Inverted rectangle width is fixed size, large enough to fit 3 characters
				ucFillRect((FILTER_TEXT_X_OFFSET - 2), DISPLAY_Y_POS_HEADER - 1, (18 + 3), 9, false);
			}

			// DCS chars are centered in their H space
			ucPrintCore((FILTER_TEXT_X_OFFSET + (9 /* halt of 3 chars */)) - ((strlen(buffer) * 6) >> 1),
					DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_LEFT, cssTextInverted);
		}
		break;

	case RADIO_MODE_DIGITAL:
		if (settingsUsbMode != USB_MODE_HOTSPOT)
		{
			bool contactTSActive = false;
			bool tsManOverride = false;

			if (nonVolatileSettings.extendedInfosOnScreen & (INFO_ON_SCREEN_TS & INFO_ON_SCREEN_BOTH))
			{
				contactTSActive = ((nonVolatileSettings.overrideTG == 0) && ((currentContactData.reserve1 & 0x01) == 0x00));
				tsManOverride = (contactTSActive ? tsIsContactHasBeenOverriddenFromCurrentChannel() : (tsGetManualOverrideFromCurrentChannel() != 0));
			}

			if (headerScanIndicatorType == notScanning)
			{
				if ((scanIsActive ? (scanBlinkPhase == false) : true) && (nonVolatileSettings.dmrDestinationFilter > DMR_DESTINATION_FILTER_NONE))
				{
					ucFillRect(0, DISPLAY_Y_POS_HEADER - 1, 20, 9, false);
				}
			}

			if (headerScanIndicatorType != vfoSweepScan)
			{
				if (scanIsActive ? scanBlinkPhase == false : true)
				{
					bool isInverted = headerScanIndicatorType ? false : (scanBlinkPhase ^ (nonVolatileSettings.dmrDestinationFilter > DMR_DESTINATION_FILTER_NONE));
					ucPrintCore(MODE_TEXT_X_OFFSET, DISPLAY_Y_POS_HEADER, headerScanIndicatorType != notScanning ? scanTypeIndicator : "DMR", ((nonVolatileSettings.hotspotType != HOTSPOT_TYPE_OFF) ? FONT_SIZE_1_BOLD : FONT_SIZE_1), TEXT_ALIGN_LEFT, isInverted);
				}

				if (headerScanIndicatorType == notScanning)
				{
					bool tsInverted = false;

					snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "%s%d",
							((contactTSActive && (monitorModeData.isEnabled == false)) ? "cS" : currentLanguage->ts),
							((monitorModeData.isEnabled && (dmrMonitorCapturedTS != -1))? (dmrMonitorCapturedTS + 1) : trxGetDMRTimeSlot() + 1));

					if (!(nonVolatileSettings.dmrCcTsFilter & DMR_TS_FILTER_PATTERN))
					{
						ucFillRect(FILTER_TEXT_X_OFFSET - 2, DISPLAY_Y_POS_HEADER - 1, 21, 9, false);
						tsInverted = true;
					}
					ucPrintCore(FILTER_TEXT_X_OFFSET, DISPLAY_Y_POS_HEADER, buffer, (tsManOverride ? FONT_SIZE_1_BOLD : FONT_SIZE_1), TEXT_ALIGN_LEFT, tsInverted);
				}
			}
		}
		break;
	}

	// Power
	sprintf(buffer,"%s%s", POWER_LEVELS[powerLevel], POWER_LEVEL_UNITS[powerLevel]);
	if (headerScanIndicatorType == vfoSweepScan) // Need to shift to the right due to sweep span
	{
		ucPrintCore(COLOR_CODE_X_POSITION - 20, DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
	}
	else
	{
		ucPrintCore(0, DISPLAY_Y_POS_HEADER, buffer,
				((isPerChannelPower && (nonVolatileSettings.extendedInfosOnScreen & (INFO_ON_SCREEN_PWR & INFO_ON_SCREEN_BOTH))) ? FONT_SIZE_1_BOLD : FONT_SIZE_1), TEXT_ALIGN_CENTER, false);
	}

	// In hotspot mode the CC is show as part of the rest of the display and in Analog mode the CC is meaningless
	if((headerScanIndicatorType == notScanning) && (((settingsUsbMode == USB_MODE_HOTSPOT) || (trxGetMode() == RADIO_MODE_ANALOG)) == false))
	{
		int ccode = trxGetDMRColourCode();
		bool isNotFilteringCC = !(nonVolatileSettings.dmrCcTsFilter & DMR_CC_FILTER_PATTERN);

		snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "C%d", ccode);

		if (isNotFilteringCC)
		{
			ucFillRect(COLOR_CODE_X_POSITION - 1, DISPLAY_Y_POS_HEADER - 1, 13 + ((ccode > 9) * 6), 9, false);
		}

		ucPrintCore(COLOR_CODE_X_POSITION, DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_LEFT, isNotFilteringCC);
	}

	// Display battery percentage/voltage
	if (nonVolatileSettings.bitfieldOptions & BIT_BATTERY_VOLTAGE_IN_HEADER)
	{
		int volts, mvolts;
		int16_t xV = (DISPLAY_SIZE_X - ((3 * 6) + 3));

		getBatteryVoltage(&volts, &mvolts);

		snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "%1d", volts);
		ucPrintCore(xV, DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_LEFT, ((batteryIsLow ? scanBlinkPhase : false)));

		ucDrawRect(xV + 6, DISPLAY_Y_POS_HEADER + 5, 2, 2, ((batteryIsLow ? !scanBlinkPhase : true)));

		snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "%1dV", mvolts);
		ucPrintCore(xV + 6 + 3, DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_LEFT, ((batteryIsLow ? scanBlinkPhase : false)));
	}
	else
	{
		snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "%d%%", getBatteryPercentage());
		ucPrintCore(0, DISPLAY_Y_POS_HEADER, buffer, FONT_SIZE_1, TEXT_ALIGN_RIGHT, ((batteryIsLow ? scanBlinkPhase : false)));// Display battery percentage at the right
	}
}

void uiUtilityRedrawHeaderOnly(HeaderScanIndicatorType_t headerScanIndicatorType)
{
	if (headerScanIndicatorType == vfoSweepScan)
	{
		ucFillRect(0, 0, DISPLAY_SIZE_X, 9, true);
	}
	else
	{
#if defined(PLATFORM_RD5R)
		ucClearRows(0, 1, false);
#else
		ucClearRows(0, 2, false);
#endif
	}

	uiUtilityRenderHeader(headerScanIndicatorType);
	ucRenderRows(0, 2);
}

int getRSSIdBm(void)
{
	int dBm = 0;

	if (trxCurrentBand[TRX_RX_FREQ_BAND] == RADIO_BAND_UHF)
	{
		// Use fixed point maths to scale the RSSI value to dBm, based on data from VK4JWT and VK7ZJA
		dBm = -151 + trxRxSignal;// Note no the RSSI value on UHF does not need to be scaled like it does on VHF
	}
	else
	{
		// VHF
		// Use fixed point maths to scale the RSSI value to dBm, based on data from VK4JWT and VK7ZJA
		dBm = -164 + ((trxRxSignal * 32) / 27);
	}

	return dBm;
}

static void drawHeaderBar(int *barWidth, int16_t barHeight)
{
	*barWidth = CLAMP(*barWidth, 0, DISPLAY_SIZE_X);

	if (*barWidth)
	{
		ucFillRect(0, DISPLAY_Y_POS_BAR, *barWidth, barHeight, false);
	}

	// Clear the end of the bar area, if needed
	if (*barWidth < DISPLAY_SIZE_X)
	{
		ucFillRect(*barWidth, DISPLAY_Y_POS_BAR, (DISPLAY_SIZE_X - *barWidth), barHeight, true);
	}
}

void uiUtilityDrawRSSIBarGraph(void)
{
	int rssi = getRSSIdBm();

	if ((rssi > SMETER_S9) && (trxGetMode() == RADIO_MODE_ANALOG))
	{
		// In Analog mode, the max RSSI value from the hardware is over S9+60.
		// So scale this to fit in the last 30% of the display
		rssi = ((rssi - SMETER_S9) / 5) + SMETER_S9;

		// in Digital mode. The maximum signal is around S9+10 dB.
		// So no scaling is required, as the full scale value is approximately S9+10dB
	}

	// Scale the entire bar by 2.
	// Because above S9 the values are scaled to 1/5. This results in the signal below S9 being doubled in scale
	// Signals above S9 the scales is compressed to 2/5.
	rssi = (rssi - SMETER_S0) * 2;

	int barWidth = ((rssi * rssiMeterHeaderBarNumUnits) / rssiMeterHeaderBarDivider);

	drawHeaderBar(&barWidth, 4);

#if 0 // Commented for now, maybe an option later.
	int xPos = 0;
	int currentMode = trxGetMode();
	for (uint8_t i = 1; ((i < 10) && (xPos <= barWidth)); i += 2)
	{
		if ((i <= 9) || (currentMode == RADIO_MODE_DIGITAL))
		{
			xPos = rssiMeterHeaderBar[i];
		}
		else
		{
			xPos = ((rssiMeterHeaderBar[i] - rssiMeterHeaderBar[9]) / 5) + rssiMeterHeaderBar[9];
		}
		xPos *= 2;

		ucDrawFastVLine(xPos, (DISPLAY_Y_POS_BAR + 1), 2, false);
	}
#endif
}

void uiUtilityDrawFMMicLevelBarGraph(void)
{
	trxReadVoxAndMicStrength();

	uint8_t micdB = (trxTxMic >> 1); // trxTxMic is in 0.5dB unit, displaying 50dB .. 100dB
	// display from 50dB to 100dB, span over 128pix
	int barWidth = ((uint16_t)(((float)DISPLAY_SIZE_X / 50.0) * ((float)micdB - 50.0)));
	drawHeaderBar(&barWidth, 3);
}

void uiUtilityDrawDMRMicLevelBarGraph(void)
{
	int barWidth = ((uint16_t)(sqrt(micAudioSamplesTotal) * 1.5));
	drawHeaderBar(&barWidth, 3);
}

void setOverrideTGorPC(uint32_t tgOrPc, bool privateCall)
{
	uiDataGlobal.tgBeforePcMode = 0;

	if (privateCall == true)
	{
		// Private Call

		if ((trxTalkGroupOrPcId >> 24) != PC_CALL_FLAG)
		{
			// if the current Tx TG is a TalkGroup then save it so it can be restored after the end of the private call
			uiDataGlobal.tgBeforePcMode = trxTalkGroupOrPcId;
		}

		tgOrPc |= (PC_CALL_FLAG << 24);
	}

	settingsSet(nonVolatileSettings.overrideTG, tgOrPc);
}

void uiUtilityDisplayFrequency(uint8_t y, bool isTX, bool hasFocus, uint32_t frequency, bool displayVFOChannel, bool isScanMode, uint8_t dualWatchVFO)
{
	char buffer[SCREEN_LINE_BUFFER_SIZE];
	int val_before_dp = frequency / 100000;
	int val_after_dp = frequency - val_before_dp * 100000;

	// Focus + direction
	snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "%c%c", ((hasFocus && !isScanMode)? '>' : ' '), (isTX ? 'T' : 'R'));

	ucPrintAt(0, y, buffer, FONT_SIZE_3);
	// VFO
	if (displayVFOChannel)
	{
		ucPrintAt(16, y + VFO_LETTER_Y_OFFSET, (((dualWatchVFO == 0) && (nonVolatileSettings.currentVFONumber == 0)) || (dualWatchVFO == 1)) ? "A" : "B", FONT_SIZE_1);
	}
	// Frequency
	snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "%d.%05d", val_before_dp, val_after_dp);
	ucPrintAt(FREQUENCY_X_POS, y, buffer, FONT_SIZE_3);
	ucPrintAt(DISPLAY_SIZE_X - (3 * 8), y, "MHz", FONT_SIZE_3);
}

size_t dcsPrintf(char *dest, size_t maxLen, char *prefix, uint16_t tone)
{
	return snprintf(dest, maxLen, "%sD%03X%c", (prefix ? prefix : ""), (tone & ~CSS_TYPE_DCS_MASK), ((tone & CSS_TYPE_DCS_INVERTED) ? 'I' : 'N'));
}

void freqEnterReset(void)
{
	memset(uiDataGlobal.FreqEnter.digits, '-', FREQ_ENTER_DIGITS_MAX);
	uiDataGlobal.FreqEnter.index = 0;
}

int freqEnterRead(int startDigit, int endDigit, bool simpleDigits)
{
	int result = 0;

	if (((startDigit >= 0) && (startDigit <= FREQ_ENTER_DIGITS_MAX)) && ((endDigit >= 0) && (endDigit <= FREQ_ENTER_DIGITS_MAX)))
	{
		for (int i = startDigit; i < endDigit; i++)
		{
			result = result * 10;
			if ((uiDataGlobal.FreqEnter.digits[i] >= '0') && (uiDataGlobal.FreqEnter.digits[i] <= '9'))
			{
				result = result + uiDataGlobal.FreqEnter.digits[i] - '0';
			}
			else
			{
				if (simpleDigits) // stop here, simple numbers are wanted
				{
					result /= 10;
					break;
				}
			}
		}
	}

	return result;
}

int getBatteryPercentage(void)
{
	return SAFE_MAX(0, SAFE_MIN(((int)(((averageBatteryVoltage - CUTOFF_VOLTAGE_UPPER_HYST) * 100) / (BATTERY_MAX_VOLTAGE - CUTOFF_VOLTAGE_UPPER_HYST))), 100));
}

void getBatteryVoltage(int *volts, int *mvolts)
{
	*volts = (int)(averageBatteryVoltage / 10);
	*mvolts = (int)(averageBatteryVoltage - (*volts * 10));
}
bool AtMaximumPower()
{
		if (currentChannelData->libreDMR_Power != 0x00)
		return currentChannelData->libreDMR_Power == (MAX_POWER_SETTING_NUM - 1 + CODEPLUG_MIN_PER_CHANNEL_POWER);
	else
		return nonVolatileSettings.txPowerLevel == (MAX_POWER_SETTING_NUM - 1);
}

bool increasePowerLevel(bool allowFullPower, bool goStraightToMaximum)
{
	bool powerHasChanged = false;

	if (currentChannelData->libreDMR_Power != 0x00)
	{
		if (currentChannelData->libreDMR_Power < (MAX_POWER_SETTING_NUM - 1 + CODEPLUG_MIN_PER_CHANNEL_POWER) + (allowFullPower?1:0))
		{
			if (goStraightToMaximum)
				currentChannelData->libreDMR_Power=(MAX_POWER_SETTING_NUM - 1 + CODEPLUG_MIN_PER_CHANNEL_POWER) + (allowFullPower?1:0);
			else
				currentChannelData->libreDMR_Power++;
			trxSetPowerFromLevel(currentChannelData->libreDMR_Power - 1);
			powerHasChanged = true;
		}
	}
	else
	{
		if (nonVolatileSettings.txPowerLevel < (MAX_POWER_SETTING_NUM - 1 + (allowFullPower?1:0)))
		{
			if (goStraightToMaximum)
				settingsSet(nonVolatileSettings.txPowerLevel, (MAX_POWER_SETTING_NUM - 1 + (allowFullPower?1:0)));
			else
				settingsIncrement(nonVolatileSettings.txPowerLevel, 1);
			trxSetPowerFromLevel(nonVolatileSettings.txPowerLevel);
			powerHasChanged = true;
		}
	}
	if (goStraightToMaximum || allowFullPower)
	{
		keyboardReset();
#if !defined(PLATFORM_GD77S)
		sk2Latch =false;
		sk2LatchTimeout=0;
#endif // !defined(PLATFORM_GD77S)
	}
	announceItem(PROMPT_SEQUENCE_POWER, PROMPT_THRESHOLD_3);

	return powerHasChanged;
}

bool decreasePowerLevel(bool goStraightToMinimum)
{
	bool powerHasChanged = false;

	if (currentChannelData->libreDMR_Power != 0x00)
	{
		if (currentChannelData->libreDMR_Power > CODEPLUG_MIN_PER_CHANNEL_POWER)
		{
			if (goStraightToMinimum)	
				currentChannelData->libreDMR_Power=CODEPLUG_MIN_PER_CHANNEL_POWER;
			else
				currentChannelData->libreDMR_Power--;
			trxSetPowerFromLevel(currentChannelData->libreDMR_Power - 1);
			powerHasChanged = true;
		}
	}
	else
	{
		if (nonVolatileSettings.txPowerLevel > 0)
		{
			if (goStraightToMinimum)
				settingsSet(nonVolatileSettings.txPowerLevel, 0);
			else
				settingsDecrement(nonVolatileSettings.txPowerLevel, 1);
			trxSetPowerFromLevel(nonVolatileSettings.txPowerLevel);
			powerHasChanged = true;
		}
	}
	if (goStraightToMinimum)
	{
		keyboardReset();
#if !defined(PLATFORM_GD77S)
		sk2Latch =false;
		sk2LatchTimeout=0;
#endif // !defined(PLATFORM_GD77S)
	}
	announceItem(PROMPT_SEQUENCE_POWER, PROMPT_THRESHOLD_3);

	return powerHasChanged;
}

ANNOUNCE_STATIC void announceRadioMode(bool voicePromptWasPlaying)
{
	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendLanguageString(&currentLanguage->mode);
	}

	voicePromptsAppendPrompt( (trxGetMode() == RADIO_MODE_DIGITAL) ? PROMPT_DMR : PROMPT_FM);
}

ANNOUNCE_STATIC void announceZoneName(bool voicePromptWasPlaying)
{
	char nameBuf[17];

	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendLanguageString(&currentLanguage->zone);
	}

	codeplugUtilConvertBufToString(currentZone.name, nameBuf, 16);

	if (strcmp(nameBuf, currentLanguage->all_channels) == 0)
	{
		voicePromptsAppendLanguageString(&currentLanguage->all_channels);
	}
	else
	{
		int len = strlen(currentLanguage->zone);
		if (strncasecmp(nameBuf, currentLanguage->zone, len)==0)
		{
			voicePromptsAppendLanguageString(&currentLanguage->zone);
			if (strlen(currentZone.name) > len)
			{
				voicePromptsAppendString(currentZone.name+len);
			}
		}
		else
		{
			voicePromptsAppendString(currentZone.name);
		}
	}
}

ANNOUNCE_STATIC void announceContactNameTgOrPc(bool voicePromptWasPlaying)
{
	if (nonVolatileSettings.overrideTG == 0)
	{
		if (!voicePromptWasPlaying)
		{
			voicePromptsAppendLanguageString(&currentLanguage->contact);
		}
		char nameBuf[17];
		codeplugUtilConvertBufToString(currentContactData.name, nameBuf, 16);
		voicePromptsAppendString(nameBuf);
	}
	else
	{
		char buf[17];
		itoa(nonVolatileSettings.overrideTG & 0xFFFFFF, buf, 10);
		if ((nonVolatileSettings.overrideTG >> 24) == PC_CALL_FLAG)
		{
			if (!voicePromptWasPlaying)
			{
				voicePromptsAppendLanguageString(&currentLanguage->private_call);
			}
			voicePromptsAppendString("ID");
		}
		else
		{
			if (!voicePromptWasPlaying)
			{
				voicePromptsAppendPrompt(PROMPT_TALKGROUP);
			}
		}
		voicePromptsAppendString(buf);
	}
}

ANNOUNCE_STATIC void announcePowerLevel(bool voicePromptWasPlaying)
{
	int powerLevel = trxGetPowerLevel();

	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendPrompt(PROMPT_POWER);
	}

	if (powerLevel < 9)
	{
		voicePromptsAppendString((char *)POWER_LEVELS[powerLevel]);
		switch(powerLevel)
		{
		case 0://50mW
		case 1://250mW
		case 2://500mW
		case 3://750mW
			voicePromptsAppendPrompt(PROMPT_MILLIWATTS);
			break;
		case 4://1W
			voicePromptsAppendPrompt(PROMPT_WATT);
			break;
		default:
			voicePromptsAppendPrompt(PROMPT_WATTS);
			break;
		}
	}
	else
	{
		voicePromptsAppendLanguageString(&currentLanguage->user_power);
		voicePromptsAppendInteger(nonVolatileSettings.userPower);
	}
}

#if defined(PLATFORM_GD77S)
void announceEcoLevel(bool voicePromptWasPlaying)
{

	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendLanguageString(&currentLanguage->eco_level);
	}

	voicePromptsAppendInteger(nonVolatileSettings.ecoLevel);
}

void announceMicGain(bool announcePrompt, bool announceValue, bool isDigital)
{
	if (announcePrompt)
	{
		if (isDigital)
		{
			voicePromptsAppendLanguageString(&currentLanguage->dmr_mic_gain);
		}
		else
		{
			voicePromptsAppendLanguageString(&currentLanguage->fm_mic_gain);
		}
	}
	if (announceValue)
	{
		if (isDigital)
		{
			char buf[SCREEN_LINE_BUFFER_SIZE];
			snprintf(buf, SCREEN_LINE_BUFFER_SIZE, "%ddB", (nonVolatileSettings.micGainDMR - 11) * 3);

			voicePromptsAppendString(buf);
		}
		else
		{
			voicePromptsAppendInteger(nonVolatileSettings.micGainFM - 16);
		}
	}
}
#endif

ANNOUNCE_STATIC void announceTemperature(bool voicePromptWasPlaying)
{
	char buffer[17];
	int temperature = getTemperature();
	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendLanguageString(&currentLanguage->temperature);
	}
	snprintf(buffer, 17, "%d.%1d", (temperature / 10), (temperature % 10));
	voicePromptsAppendString(buffer);
	voicePromptsAppendLanguageString(&currentLanguage->celcius);
}

ANNOUNCE_STATIC void announceBatteryVoltage(void)
{
	char buffer[17];
	int volts, mvolts;

	voicePromptsAppendLanguageString(&currentLanguage->battery);
	getBatteryVoltage(&volts,  &mvolts);
	snprintf(buffer, 17, " %1d.%1d", volts, mvolts);
	voicePromptsAppendString(buffer);
	voicePromptsAppendPrompt(PROMPT_VOLTS);
}

ANNOUNCE_STATIC void announceBatteryPercentage(void)
{
	voicePromptsAppendLanguageString(&currentLanguage->battery);
	voicePromptsAppendInteger(getBatteryPercentage());
	voicePromptsAppendPrompt(PROMPT_PERCENT);
}

ANNOUNCE_STATIC void announceTS(void)
{
	voicePromptsAppendPrompt(PROMPT_TIMESLOT);
	voicePromptsAppendInteger(trxGetDMRTimeSlot() + 1);
}

ANNOUNCE_STATIC void announceCC(void)
{
	voicePromptsAppendLanguageString(&currentLanguage->colour_code);
	voicePromptsAppendInteger(trxGetDMRColourCode());
}

static bool IsThisThePriorityChannel()
{
	uint16_t channelIndexRelativeToAllChannels=CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : currentZone.channels[nonVolatileSettings.currentChannelIndexInZone];
	
	return (uiDataGlobal.priorityChannelIndex == channelIndexRelativeToAllChannels) || uiDataGlobal.priorityChannelActive;
}

static bool PriorityChannelExistsInCurrentZone()
{
	if (uiDataGlobal.priorityChannelIndex==NO_PRIORITY_CHANNEL)
		return false;

	uint16_t indexRelativeToCurrentZone=NO_PRIORITY_CHANNEL;	
	return codeplugFindAllChannelsIndexInCurrentZone(uiDataGlobal.priorityChannelIndex, &indexRelativeToCurrentZone) && indexRelativeToCurrentZone!=NO_PRIORITY_CHANNEL;
}

ANNOUNCE_STATIC void announceChannelName(bool announceChannelPrompt, bool announceChannelNumberIfDifferentToName)
{
	char voiceBuf[17];
	char voiceBufChNumber[5];
	int channelNumber = CODEPLUG_ZONE_IS_ALLCHANNELS(currentZone) ? nonVolatileSettings.currentChannelIndexInAllZone : (nonVolatileSettings.currentChannelIndexInZone+1);
	codeplugUtilConvertBufToString(currentChannelData->name, voiceBuf, 16);
	// We can't announce  the channel number if this is the priority channel but it does not exist in the current zone, otherwise it won't make sense.
	bool thisIsThePriorityChannel=IsThisThePriorityChannel();
	bool announceChannelNumber=announceChannelNumberIfDifferentToName;
	if (thisIsThePriorityChannel && !PriorityChannelExistsInCurrentZone())
		announceChannelNumber=false;
		
	snprintf(voiceBufChNumber, 5, "%d", channelNumber);
	
	if (announceChannelPrompt)
	{
		if (thisIsThePriorityChannel)
			voicePromptsAppendLanguageString(&currentLanguage->priorityChannel);
		else
			voicePromptsAppendPrompt(PROMPT_CHANNEL);
	}
	// If the number and name differ, then append.
	if (announceChannelNumber && strcmp(voiceBufChNumber, voiceBuf) != 0)
	{
		voicePromptsAppendString(voiceBufChNumber);
		voicePromptsAppendPrompt(PROMPT_SILENCE);
	}
	voicePromptsAppendString(voiceBuf);
}

void removeUnnecessaryZerosFromVoicePrompts(char *str)
{
	const int NUM_DECIMAL_PLACES = 1;
	int len = strlen(str);
	for(int i = len; i > 2; i--)
	{
		if ((str[i - 1] != '0') || (str[i - (NUM_DECIMAL_PLACES + 1)] == '.'))
		{
			str[i] = 0;
			return;
		}
	}
}

static void announceQRG(uint32_t qrg, bool unit)
{
	char buffer[17];
	int val_before_dp = qrg / 100000;
	int val_after_dp = qrg - val_before_dp * 100000;

	snprintf(buffer, 17, "%d.%05d", val_before_dp, val_after_dp);
	removeUnnecessaryZerosFromVoicePrompts(buffer);
	voicePromptsAppendString(buffer);

	if (unit)
	{
		voicePromptsAppendPrompt(PROMPT_MEGAHERTZ);
	}
}

static void announceFrequencyEx(bool announcePrompt, bool announceRx, bool announceTx)
{
	bool duplex = (currentChannelData->txFreq != currentChannelData->rxFreq);

	if (duplex && announcePrompt && announceRx)
	{
		voicePromptsAppendPrompt(PROMPT_RECEIVE);
	}

	if (announceRx)
		announceQRG(currentChannelData->rxFreq, true);

	if (duplex && announceTx)
	{
		if (announcePrompt)
			voicePromptsAppendPrompt(PROMPT_TRANSMIT);
		announceQRG(currentChannelData->txFreq, true);
	}
}

ANNOUNCE_STATIC void announceFrequency(void)
{
	announceFrequencyEx(true, true, true);
	}

ANNOUNCE_STATIC void announceVFOChannelName(void)
{
	voicePromptsAppendPrompt(PROMPT_VFO);
	voicePromptsAppendString((nonVolatileSettings.currentVFONumber == 0) ? "A" : "B");
	voicePromptsAppendPrompt(PROMPT_SILENCE);
}

ANNOUNCE_STATIC void announceVFOAndFrequency(bool announceVFOName)
{
	if (announceVFOName)
	{
		announceVFOChannelName();
	}
	announceFrequency();
}

ANNOUNCE_STATIC void announceSquelchLevel(bool voicePromptWasPlaying)
{
	static const int BUFFER_LEN = 8;
	char buf[BUFFER_LEN];

	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendLanguageString(&currentLanguage->squelch);
	}

	snprintf(buf, BUFFER_LEN, "%d%%", 5 * (((currentChannelData->sql == 0) ? nonVolatileSettings.squelchDefaults[trxCurrentBand[TRX_RX_FREQ_BAND]] : currentChannelData->sql)-1));
	voicePromptsAppendString(buf);
}

void announceChar(char ch)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	char buf[2] = {ch, 0};
	VoicePromptFlags_T flags =settingsIsOptionBitSet(BIT_PHONETIC_SPELL)?vpAnnouncePhoneticRendering:0;

	voicePromptsInit();
	voicePromptsAppendStringEx(buf, vpAnnounceCaps|vpAnnounceSpaceAndSymbols|flags);
	voicePromptsPlay();
}

void buildCSSCodeVoicePrompts(uint16_t tone, CodeplugCSSTypes_t cssType, Direction_t direction, bool announceType)
{
	static const int BUFFER_LEN = 6;
	char buf[BUFFER_LEN];

	switch(direction)
	{
	case DIRECTION_RECEIVE:
		//voicePromptsAppendString("RX");
		voicePromptsAppendPrompt(PROMPT_RECEIVE);
		break;
	case DIRECTION_TRANSMIT:
		//voicePromptsAppendString("TX");
		voicePromptsAppendPrompt(PROMPT_TRANSMIT);
		break;
	default:
		break;
	}

	if (cssType == CSS_TYPE_NONE)
	{
		voicePromptsAppendStringEx("CSS", vpAnnounceCustomPrompts);
		voicePromptsAppendLanguageString(&currentLanguage->none);
	}
	else if (cssType == CSS_TYPE_CTCSS)
	{
		if (announceType)
		{
			//voicePromptsAppendString("CTCSS");
			voicePromptsAppendLanguageString(&currentLanguage->tone);
		}
		snprintf(buf, BUFFER_LEN, "%d.%d", tone / 10, tone % 10);
		voicePromptsAppendString(buf);
		voicePromptsAppendPrompt(PROMPT_HERTZ);
	}
	else if (cssType & CSS_TYPE_DCS)
	{
		if (announceType)
		{
			voicePromptsAppendStringEx("DCS", vpAnnounceCustomPrompts);
		}

		dcsPrintf(buf, BUFFER_LEN, NULL, tone);
		voicePromptsAppendString(buf);
	}
}


void announceCSSCode(uint16_t tone, CodeplugCSSTypes_t cssType, Direction_t direction, bool announceType, audioPromptThreshold_t immediateAnnounceThreshold)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	bool voicePromptWasPlaying = voicePromptsIsPlaying();

	voicePromptsInit();

	buildCSSCodeVoicePrompts(tone, cssType, direction, announceType);

	// Follow-on when voicePromptWasPlaying is enabled on voice prompt level 2 and above
	// Prompts are voiced immediately on voice prompt level 3
	if ((voicePromptWasPlaying && (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_2)) ||
			(nonVolatileSettings.audioPromptMode >= immediateAnnounceThreshold))
	{
		voicePromptsPlay();
	}
}

void playNextSettingSequence(void)
{
	voicePromptSequenceState++;

	if (voicePromptSequenceState == NUM_PROMPT_SEQUENCES)
	{
		voicePromptSequenceState = 0;
	}

	announceItem(voicePromptSequenceState, PROMPT_THRESHOLD_3);
}

static void announceChannelNameOrVFOFrequency(bool voicePromptWasPlaying, bool announceVFOName)
{
	if (menuSystemGetCurrentMenuNumber() == UI_CHANNEL_MODE)
	{
		announceChannelName(!voicePromptWasPlaying, false);
	}
	else
	{
		announceVFOAndFrequency(announceVFOName);
	}
}

static void 		announceBandWidth(bool voicePromptWasPlaying)
{
	if (!voicePromptWasPlaying)
	{
		voicePromptsAppendLanguageString(&currentLanguage->bandwidth);
	}
	bool wide=(currentChannelData->flag4&0x02) ? true : false;
	char buffer[17]="\0";
	if (wide)
		strcpy(buffer, "25");
	else
		strcpy(buffer, "12.5");
	voicePromptsAppendString(buffer);
	voicePromptsAppendPrompt(PROMPT_KILOHERTZ);
}

void announceItemWithInit(bool init, voicePromptItem_t item, audioPromptThreshold_t immediateAnnounceThreshold)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
	{
		return;
	}

	bool lessVerbose =nonVolatileSettings.audioPromptMode <= AUDIO_PROMPT_MODE_VOICE_LEVEL_2;
	// If voice prompts are already playing and further speech is requested, a less verbose sequence is spoken.
	// This is known as follow-on.
	// For example, at level 3, if the name and mode are being spoken,
	// the user would hear "channel" name "mode" fm.
	// If follow-on occurs, they'd just hear name fm.
	// At voice prompt level 2, we always enforce follow-on to reduce verbosity.
	bool voicePromptWasPlaying = voicePromptsIsPlaying() || lessVerbose;

	voicePromptSequenceState = item;

	if (init)
	{
		voicePromptsInit();
	}

	switch(voicePromptSequenceState)
	{
	case PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ:
	case PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ_AND_MODE:
	case PROMPT_SEQUENCE_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE:
	case PROMPT_SEQUENCE_ZONE_NAME_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE:
	case PROMPT_SEQUENCE_ZONE_NAME_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE_AND_TS_AND_CC:
	case PROMPT_SEQUENCE_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE_AND_TS_AND_CC:
	case PROMPT_SEQUENCE_VFO_FREQ_UPDATE:
	case PROMPT_SEQUENCE_VFO_SCAN_RANGE_UPDATE:
	{
		uint32_t lFreq, hFreq;

		if (((nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_2) || (immediateAnnounceThreshold == PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY)) &&
				((voicePromptSequenceState == PROMPT_SEQUENCE_ZONE_NAME_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE) ||
						(voicePromptSequenceState == PROMPT_SEQUENCE_ZONE_NAME_CHANNEL_NAME_AND_CONTACT_OR_VFO_FREQ_AND_MODE_AND_TS_AND_CC)))
		{
			announceZoneName(voicePromptWasPlaying);
		}
		AnnounceLastHeardContact();
		if (voicePromptSequenceState!=PROMPT_SEQUENCE_VFO_SCAN_RANGE_UPDATE)
			announceChannelNameOrVFOFrequency(voicePromptWasPlaying, (voicePromptSequenceState != PROMPT_SEQUENCE_VFO_FREQ_UPDATE));
		if (uiVFOModeFrequencyScanningIsActiveAndEnabled(&lFreq, &hFreq))
		{
			voicePromptsAppendPrompt(PROMPT_SCAN_MODE);
			voicePromptsAppendLanguageString(&currentLanguage->low);
			announceQRG(lFreq, true);
			voicePromptsAppendLanguageString(&currentLanguage->high);
			announceQRG(hFreq, true);
		}
		if (voicePromptSequenceState == PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ)
		{
			break;
		}
		if (voicePromptSequenceState == PROMPT_SEQUENCE_VFO_FREQ_UPDATE)
		{
			announceVFOChannelName();
		}
		if (!lessVerbose)// At level 2, do not say FM or DMR, only say contact which will indicate DMR, no contact will presumably be fm.
			announceRadioMode(voicePromptWasPlaying);
		if (voicePromptSequenceState == PROMPT_SEQUENCE_CHANNEL_NAME_OR_VFO_FREQ_AND_MODE)
		{
			break;
		}
		if (trxGetMode() == RADIO_MODE_DIGITAL)
		{
			announceContactNameTgOrPc(voicePromptWasPlaying);
			if ((nonVolatileSettings.extendedInfosOnScreen & (INFO_ON_SCREEN_TS & INFO_ON_SCREEN_BOTH)) && tsGetManualOverrideFromCurrentChannel())
			{
				voicePromptsAppendPrompt(PROMPT_SILENCE);
				announceTS();
			}
		}
		if ((currentChannelData->libreDMR_Power != 0) && (nonVolatileSettings.extendedInfosOnScreen & (INFO_ON_SCREEN_PWR & INFO_ON_SCREEN_BOTH)))
		{
			voicePromptsAppendPrompt(PROMPT_SILENCE);
			announcePowerLevel(voicePromptWasPlaying);
		}
	}
	break;
	case PROMPT_SEQUENCE_ZONE:
		announceZoneName(voicePromptWasPlaying);
		break;
	case PROMPT_SEQUENCE_ZONE_AND_CHANNEL_NAME:
		announceZoneName(voicePromptWasPlaying);
		if (voicePromptSequenceState==PROMPT_SEQUENCE_ZONE)
			break;
		voicePromptsAppendPrompt(PROMPT_SILENCE);
		announceChannelName(!voicePromptWasPlaying, false);
	//	deliberate fall through to announce mode.
	case PROMPT_SEQUENCE_MODE:
		announceRadioMode(voicePromptWasPlaying);
		break;
	case PROMPT_SEQUENCE_CONTACT_TG_OR_PC:
		announceContactNameTgOrPc(voicePromptWasPlaying);
		break;
	case PROMPT_SEQUENCE_TS:
		announceTS();
		break;
	case PROMPT_SEQUENCE_CC:
		announceCC();
		break;
	case PROMPT_SEQUENCE_POWER:
		announcePowerLevel(voicePromptWasPlaying);
		break;
	case PROMPT_SEQUENCE_BATTERY:
		if (nonVolatileSettings.bitfieldOptions & BIT_BATTERY_VOLTAGE_IN_HEADER)
		{
			announceBatteryVoltage();
		}
		else
		{
			announceBatteryPercentage();
		}
		break;
	case PROMPT_SQUENCE_SQUELCH:
		announceSquelchLevel(voicePromptWasPlaying);
		break;
	case PROMPT_SEQUENCE_TEMPERATURE:
		announceTemperature(voicePromptWasPlaying);
		break;
	case PROMPT_SEQUENCE_VFO_INPUT_RX_FIELD_AND_FREQ:
	{
		voicePromptsAppendPrompt(PROMPT_RECEIVE);
		announceFrequencyEx(false, true, false);
		break;
	}
	case PROMPT_SEQUENCE_VFO_INPUT_TX_FIELD_AND_FREQ:
	{
		voicePromptsAppendPrompt(PROMPT_TRANSMIT);
		announceFrequencyEx(false, false, true);
		break;
	case PROMPT_SEQUENCE_BANDWIDTH:
		announceBandWidth(voicePromptWasPlaying);
		break;
	}
	case PROMPT_SEQUENCE_DIRECTION_TX:
		voicePromptsAppendString("TX");
		break;
	case PROMPT_SEQUENCE_DIRECTION_RX:
		voicePromptsAppendString("RX");
		break;
	case PROMPT_SEQUENCE_CHANNEL_NUMBER_AND_NAME:
		announceChannelName(!voicePromptWasPlaying, true);
		break;
	case PROMPT_SEQUENCE_SCAN_TYPE:
	{
		if (uiDataGlobal.Scan.active)
		{
			voicePromptsAppendLanguageString(&currentLanguage->scan);
			if (menuSystemGetCurrentMenuNumber() == UI_CHANNEL_MODE)
			{
				if (uiDataGlobal.Scan.scanAllZones)
					voicePromptsAppendLanguageString(&currentLanguage->all);
				else
					voicePromptsAppendLanguageString(&currentLanguage->zone);
			}
		}
		break;
	}	
	default:
		break;
	}
	// If PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY, after building up the prompt, save it for speaking later but don't play it now.
	if (immediateAnnounceThreshold==PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY)
		return;

	// Follow-on when voicePromptWasPlaying is enabled on voice prompt level 2 and above
		// Prompts are voiced immediately on voice prompt level 3
	if ((voicePromptWasPlaying && (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_2)) ||
			(nonVolatileSettings.audioPromptMode >= immediateAnnounceThreshold))
	{
		voicePromptsPlay();
	}
}

void announceItem(voicePromptItem_t item, audioPromptThreshold_t immediateAnnounceThreshold)
{
	announceItemWithInit(true, item, immediateAnnounceThreshold);
}

void promptsPlayNotAfterTx(void)
{
	if (menuSystemGetPreviouslyPushedMenuNumber() != UI_TX_SCREEN)
	{
		voicePromptsPlay();
	}
}

void uiUtilityBuildTgOrPCDisplayName(char *nameBuf, int bufferLen)
{
	int contactIndex;
	struct_codeplugContact_t contact;
	uint32_t id = (trxTalkGroupOrPcId & 0x00FFFFFF);
	int8_t manTS = tsGetManualOverrideFromCurrentChannel();

	if ((trxTalkGroupOrPcId >> 24) == TG_CALL_FLAG)
	{
		contactIndex = codeplugContactIndexByTGorPC(id, CONTACT_CALLTYPE_TG, &contact, (manTS ? manTS : (trxGetDMRTimeSlot() + 1)));
		if (contactIndex == -1)
		{
			snprintf(nameBuf, bufferLen, "%s %u", currentLanguage->tg, (trxTalkGroupOrPcId & 0x00FFFFFF));
		}
		else
		{
			codeplugUtilConvertBufToString(contact.name, nameBuf, 16);
		}
	}
	else
	{
		contactIndex = codeplugContactIndexByTGorPC(id, CONTACT_CALLTYPE_PC, &contact, (manTS ? manTS : (trxGetDMRTimeSlot() + 1)));
		if (contactIndex == -1)
		{
			dmrIdDataStruct_t currentRec;
			if (dmrIDLookup(id, &currentRec))
			{
				strncpy(nameBuf, currentRec.text, bufferLen);
			}
			else
			{
				// check LastHeard for TA data.
				LinkItem_t *item = lastheardFindInList(id);
				if ((item != NULL) && (strlen(item->talkerAlias) != 0))
				{
					strncpy(nameBuf, item->talkerAlias, bufferLen);
				}
				else
				{
					snprintf(nameBuf, bufferLen, "ID:%u", id);
				}
			}
		}
		else
		{
			codeplugUtilConvertBufToString(contact.name, nameBuf, 16);
		}
	}
}

void acceptPrivateCall(uint32_t id, int timeslot)
{
	uiDataGlobal.PrivateCall.state = PRIVATE_CALL;
	uiDataGlobal.PrivateCall.lastID = (id & 0xffffff);
	uiDataGlobal.receivedPcId = 0x00;

	setOverrideTGorPC(uiDataGlobal.PrivateCall.lastID, true);

#if !defined(PLATFORM_GD77S)
	if (timeslot != trxGetDMRTimeSlot())
	{
		trxSetDMRTimeSlot(timeslot);
		tsSetManualOverride(((menuSystemGetRootMenuNumber() == UI_CHANNEL_MODE) ? CHANNEL_CHANNEL : (CHANNEL_VFO_A + nonVolatileSettings.currentVFONumber)), (timeslot + 1));
	}
#else
	(void)timeslot;
#endif

	announceItem(PROMPT_SEQUENCE_CONTACT_TG_OR_PC,PROMPT_THRESHOLD_3);
}

bool repeatVoicePromptOnSK1(uiEvent_t *ev)
{
	if (BUTTONCHECK_SHORTUP(ev, BUTTON_SK1) && (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0) && (ev->keys.key == 0))
	{
		if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
		{
			if (voicePromptsIsPlaying())
				voicePromptsTerminate();
			else
			{
				if (voicePromptsGetEditMode())
					ReplayDMR();
				else
					voicePromptsPlay();
			}
		}		
	return true;
	}

	return false;
}
/*
static void testCompressDecompress()
{
	char test[16]="\0";
strcpy(test,"vk7js Joseph 50");
	uint8_t compressedBuf[16];
	uint8_t bytes = dmrDbTextEncode((uint8_t*)&compressedBuf, 16, (uint8_t*)&test, 16);
voicePromptsInit();
voicePromptsAppendInteger(bytes);

char output[16]="\0";

dmrDbTextDecode((uint8_t*)&output, (uint8_t*)&compressedBuf, bytes);
voicePromptsAppendStringEx(output,vpAnnounceCustomPrompts);
}
*/
void AnnounceChannelSummary(bool voicePromptWasPlaying, bool announceName)
{
	bool isChannelScreen=menuSystemGetCurrentMenuNumber() == UI_CHANNEL_MODE;
	
	voicePromptsInit();
	announceItemWithInit(false, PROMPT_SEQUENCE_SCAN_TYPE, PROMPT_THRESHOLD_NEVER_PLAY_IMMEDIATELY); // since this function calling does the init and play.
	AnnounceLastHeardContact();
	if (announceName)
	{
		if (isChannelScreen)
			announceChannelName(true, true);
		else
			announceVFOChannelName();
	}

	announceFrequency();
	
	uint32_t lFreq, hFreq;

	if (!isChannelScreen && uiVFOModeFrequencyScanningIsActiveAndEnabled(&lFreq, &hFreq))
	{
		voicePromptsAppendLanguageString(&currentLanguage->low);
		announceQRG(lFreq, true);
		voicePromptsAppendLanguageString(&currentLanguage->high);
		announceQRG(hFreq, true);
	}
	
	announceRadioMode(voicePromptWasPlaying);
	bool narrow=(currentChannelData->flag4&0x02)==0 ? true : false;
	if (narrow && trxGetMode() == RADIO_MODE_ANALOG)
		voicePromptsAppendPrompt(PROMPT_N);

	voicePromptsAppendPrompt(PROMPT_SILENCE);
	
	if (trxGetMode() == RADIO_MODE_DIGITAL)
	{
		announceContactNameTgOrPc(voicePromptsIsPlaying());
		voicePromptsAppendPrompt(PROMPT_SILENCE);
		announceTS();
		voicePromptsAppendPrompt(PROMPT_SILENCE);
		announceCC();
	}
	else
	{
		bool rxAndTxTonesAreTheSame = (currentChannelData->rxTone != CODEPLUG_CSS_TONE_NONE)
		&& (currentChannelData->rxTone ==currentChannelData->txTone);
		if (currentChannelData->rxTone != CODEPLUG_CSS_TONE_NONE)
		{
			bool isCTCSS = codeplugGetCSSType(currentChannelData->rxTone)==CSS_TYPE_CTCSS;

			buildCSSCodeVoicePrompts(currentChannelData->rxTone,
			(isCTCSS ? CSS_TYPE_CTCSS : ((currentChannelData->rxTone & CSS_TYPE_DCS_MASK) ? CSS_TYPE_DCS_INVERTED : CSS_TYPE_DCS)), rxAndTxTonesAreTheSame? DIRECTION_NONE : DIRECTION_RECEIVE, true);
			voicePromptsAppendPrompt(PROMPT_SILENCE);
		}

		if (currentChannelData->txTone != CODEPLUG_CSS_TONE_NONE && !rxAndTxTonesAreTheSame)
		{
			bool isCTCSS = codeplugGetCSSType(currentChannelData->txTone)==CSS_TYPE_CTCSS;

			buildCSSCodeVoicePrompts(currentChannelData->txTone,
			(isCTCSS ? CSS_TYPE_CTCSS : ((currentChannelData->txTone & CSS_TYPE_DCS_MASK ) ? CSS_TYPE_DCS_INVERTED : CSS_TYPE_DCS)), DIRECTION_TRANSMIT, true);
		}
		voicePromptsAppendPrompt(PROMPT_SILENCE);
	}

	voicePromptsAppendPrompt(PROMPT_SILENCE);
	announcePowerLevel(voicePromptWasPlaying);
	if (isChannelScreen)
	{
		if (currentChannelData->libreDMR_Power == 0)
			voicePromptsAppendLanguageString(&currentLanguage->from_master);
	
		voicePromptsAppendPrompt(PROMPT_SILENCE);

		announceZoneName(voicePromptWasPlaying);
	}

#ifdef SONIC_MEM_TRACK
	voicePromptsAppendPrompt(PROMPT_I);
	voicePromptsAppendInteger(sonicGetMaxInput());
	
	voicePromptsAppendPrompt(PROMPT_O);
	voicePromptsAppendInteger( sonicGetMaxOutput());
	// settings struct size.
	voicePromptsAppendPrompt(PROMPT_S);
	voicePromptsAppendInteger(sizeof(nonVolatileSettings));
#endif
	voicePromptsPlay();
}

bool handleMonitorMode(uiEvent_t *ev)
{
	// Time by which a DMR signal should have been decoded, including locking on to a DMR signal on a different CC
	const int DMR_MODE_CC_DETECT_TIME_MS = 375;// WAS:250;// Normally it seems to take about 125mS to detect DMR even if the CC is incorrect.

	if (monitorModeData.isEnabled)
	{
#if defined(PLATFORM_GD77S)
		if ((BUTTONCHECK_DOWN(ev, BUTTON_SK1) == 0) || (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0) || BUTTONCHECK_DOWN(ev, BUTTON_ORANGE) || (ev->events & ROTARY_EVENT))
#else
		if ((BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0) || (ev->keys.key != 0) || BUTTONCHECK_DOWN(ev, BUTTON_SK1)
#if !defined(PLATFORM_RD5R)
				|| BUTTONCHECK_DOWN(ev, BUTTON_ORANGE)
#endif
			)
#endif
		{
			monitorModeData.isEnabled = false;

			// Blindly set mode and BW, as maybe only FM BW has changed (MonitorMode ends to FM 25kHz if no DMR is detected)
			// Anyway, trxSetModeAndBandwidth() won't do anything if the current mode and BW are identical,
			trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4 & 0x02) == 0x02));

			currentChannelData->sql = monitorModeData.savedSquelch;
			nonVolatileSettings.dmrCcTsFilter = monitorModeData.savedDMRCcTsFilter;
			nonVolatileSettings.dmrDestinationFilter = monitorModeData.savedDMRDestinationFilter;

			switch (monitorModeData.savedRadioMode)
			{
				case RADIO_MODE_ANALOG:
					trxSetRxCSS(currentChannelData->rxTone);
					break;
				case RADIO_MODE_DIGITAL:
					trxSetDMRColourCode(monitorModeData.savedDMRCc);
					trxSetDMRTimeSlot(monitorModeData.savedDMRTs);
					init_digital_DMR_RX();
					disableAudioAmp(AUDIO_AMP_MODE_RF);
					break;
			}

			headerRowIsDirty = true;

			if (uiVFOModeSweepScanning(true))
			{
				uiVFOSweepScanModePause(false, false);
			}
			else
			{
				uiDataGlobal.displayQSOState = QSO_DISPLAY_DEFAULT_SCREEN;
				addTimerCallback(uiUtilityRenderQSODataAndUpdateScreen, 2000, menuSystemGetCurrentMenuNumber(), true);
			}

			return true;
		}
	}
	else
	{
#if defined(PLATFORM_GD77S)
		if (BUTTONCHECK_LONGDOWN(ev, BUTTON_SK1) && BUTTONCHECK_LONGDOWN(ev, BUTTON_SK2)
#else
		if (BUTTONCHECK_EXTRALONGDOWN(ev, BUTTON_SK2) && (BUTTONCHECK_DOWN(ev, BUTTON_SK1) == 0) && (BUTTONCHECK_SHORTUP(ev, BUTTON_SK1) == 0)
#endif
				&& (((uiDataGlobal.Scan.toneActive == false) &&
						((uiDataGlobal.Scan.active == false) || (uiDataGlobal.Scan.active && (uiDataGlobal.Scan.state == SCAN_PAUSED))))
						|| uiVFOModeSweepScanning(false))
		)
		{
			if (voicePromptsIsPlaying())
			{
				voicePromptsTerminate();
			}

			if (uiVFOModeSweepScanning(false))
			{
				uiVFOSweepScanModePause(true, true);
			}

			monitorModeData.savedRadioMode = trxGetMode();
			monitorModeData.savedSquelch = currentChannelData->sql;
			monitorModeData.savedDMRCcTsFilter = nonVolatileSettings.dmrCcTsFilter;
			monitorModeData.savedDMRDestinationFilter = nonVolatileSettings.dmrDestinationFilter;
			monitorModeData.savedDMRCc = trxGetDMRColourCode();
			monitorModeData.savedDMRTs = trxGetDMRTimeSlot();

			// Temporary override DMR filtering
			nonVolatileSettings.dmrCcTsFilter = DMR_CCTS_FILTER_NONE;
			nonVolatileSettings.dmrDestinationFilter = DMR_DESTINATION_FILTER_NONE;
			monitorModeData.dmrTimeout = DMR_MODE_CC_DETECT_TIME_MS;

			monitorModeData.dmrIsValid = false;
			monitorModeData.qsoInfoUpdated = true;
			monitorModeData.isEnabled = true;

			// Start with DMR autodetection
			if (monitorModeData.savedRadioMode != RADIO_MODE_DIGITAL)
			{
				trxSetModeAndBandwidth(RADIO_MODE_DIGITAL, false);
			}

			trxSetDMRColourCode(0);
			trxSetDMRTimeSlot(0);
			clearActiveDMRID();
			lastHeardClearLastID();

			init_digital_DMR_RX();
			disableAudioAmp(AUDIO_AMP_MODE_RF);

			headerRowIsDirty = true;
			return true;
		}
	}
	return false;
}

// Helper function that manages the returned value from the codeplug quickkey code
static bool setQuickkeyFunctionID(char key, uint16_t functionId, bool silent)
{
	if (
#if defined(PLATFORM_RD5R)
			// '5' is reserved for torch on RD-5R
			(key != '5') &&
#endif
			codeplugSetQuickkeyFunctionID(key, functionId))
	{
		if (silent == false)
		{
			nextKeyBeepMelody = (int *)MELODY_ACK_BEEP;
		}
		return true;
	}

	nextKeyBeepMelody = (int *)MELODY_ERROR_BEEP;
	return false;
}

void saveQuickkeyMenuIndex(char key, uint8_t menuId, uint8_t entryId, uint8_t function)
{
	uint16_t functionID;

	functionID = QUICKKEY_MENUVALUE(menuId, entryId, function);
	if (setQuickkeyFunctionID(key, functionID, false))
	{
		menuDataGlobal.menuOptionsTimeout = -1;// Flag to indicate that a QuickKey has just been set.
	}
}

void saveQuickkeyMenuLongValue(char key, uint8_t menuId, uint16_t entryId)
{
	uint16_t functionID;

	functionID = QUICKKEY_MENULONGVALUE(menuId, entryId);
	setQuickkeyFunctionID(key, functionID, ((menuId == 0) && (entryId == 0)));
}

void saveQuickkeyContactIndex(char key, uint16_t contactId)
{
	setQuickkeyFunctionID(key, QUICKKEY_CONTACTVALUE(contactId), false);
}

// Returns the index in either the CTCSS or DCS list of the tone (or closest match)
uint8_t cssGetToneIndex(uint16_t tone, CodeplugCSSTypes_t type)
{
	uint16_t *start = NULL;
	uint16_t *end = NULL;

	if (type & CSS_TYPE_CTCSS)
	{
		start = (uint16_t *)TRX_CTCSSTones;
		end = start + (TRX_NUM_CTCSS - 1);
	}
	else if (type & CSS_TYPE_DCS)
	{
		tone &= ~CSS_TYPE_DCS_MASK;
		start = (uint16_t *)TRX_DCSCodes;
		end = start + (TRX_NUM_DCS - 1);
	}

	if (start && end)
	{
		uint16_t *p = start;

		while((p <= end) && (*p < tone))
		{
			p++;
		}

		if (p <= end)
		{
			return (p - start);
		}
	}

	return 0U;
}

uint16_t cssGetToneFromIndex(uint8_t index, CodeplugCSSTypes_t type)
{
	if (type & CSS_TYPE_CTCSS)
	{
		if (index >= TRX_NUM_CTCSS)
		{
			index = 0;
		}
		return TRX_CTCSSTones[index];
	}
	else if (type & CSS_TYPE_DCS)
	{
		if (index >= TRX_NUM_DCS)
		{
			index = 0;
		}
		return (TRX_DCSCodes[index] | type);
	}

	return CODEPLUG_CSS_TONE_NONE;
}

void cssIncrement(uint16_t *tone, uint8_t *index, uint8_t step, CodeplugCSSTypes_t *type, bool loop, bool stayInCSSType)
{
	*index += step;

	if (*type & CSS_TYPE_CTCSS)
	{
		if (*index >= TRX_NUM_CTCSS)
		{
			if (stayInCSSType)
			{
				*index = 0;
			}
			else
			{
				*tone = cssGetToneFromIndex((*index = 0), (*type = CSS_TYPE_DCS));
				return;
			}
		}
		*tone = TRX_CTCSSTones[*index];
	}
	else if (*type & CSS_TYPE_DCS)
	{
		if (*index >= TRX_NUM_DCS)
		{
			if (stayInCSSType)
			{
				*index = 0;
			}
			else
			{
				if (*type & CSS_TYPE_DCS_INVERTED)
				{
					if (loop)
					{
						*tone = cssGetToneFromIndex((*index = 0), (*type = CSS_TYPE_CTCSS));
						return;
					}
					// We are at the end of whole list
					*index = TRX_NUM_DCS - 1;
				}
				else
				{
					*tone = cssGetToneFromIndex((*index = 0), (*type = (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED)));
					return;
				}
			}
		}
		*tone = TRX_DCSCodes[*index] | *type;
	}
	else if (*type & CSS_TYPE_NONE)
	{
		*tone = cssGetToneFromIndex((*index = 0), (*type = CSS_TYPE_CTCSS));
	}
}

void cssDecrement(uint16_t *tone, uint8_t *index, uint8_t step, CodeplugCSSTypes_t *type, bool loop, bool stayInCSSType)
{
	int16_t idx = *index - step;

	if (*type & CSS_TYPE_CTCSS)
	{
		if (idx < 0)
		{
			if (stayInCSSType)
			{
				idx = TRX_NUM_CTCSS - 1;
			}
			else
			{
				if (loop)
				{
					*tone = cssGetToneFromIndex((*index = (TRX_NUM_DCS - 1)), (*type = (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED)));
				}
				else
				{
					*tone = cssGetToneFromIndex((*index = 0), (*type = CSS_TYPE_NONE));
				}
				return;
			}
		}
		*tone = TRX_CTCSSTones[(*index = (uint8_t)idx)];
	}
	else if (*type & CSS_TYPE_DCS)
	{
		if (idx < 0)
		{
			if (stayInCSSType)
			{
				idx = TRX_NUM_DCS - 1;
			}
			else
			{
				if (*type & CSS_TYPE_DCS_INVERTED)
				{
					*tone = cssGetToneFromIndex((*index = (TRX_NUM_DCS - 1)), (*type = CSS_TYPE_DCS));
				}
				else
				{
					*tone = cssGetToneFromIndex((*index = (TRX_NUM_CTCSS - 1)), (*type = CSS_TYPE_CTCSS));
				}
				return;
			}
		}
		*tone = TRX_DCSCodes[(*index = (uint8_t)idx)] | *type;
	}
	else if (*type & CSS_TYPE_NONE)
	{
		if (loop)
		{
			*tone = cssGetToneFromIndex((*index = (TRX_NUM_DCS - 1)), (*type = (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED)));
		}
		else
		{
			*tone = cssGetToneFromIndex((*index = 0), (*type = CSS_TYPE_NONE));
		}
	}
}

bool uiShowQuickKeysChoices(char *buf, const int bufferLen, const char *menuTitle)
{
	bool settingOption = (menuDataGlobal.menuOptionsSetQuickkey != 0) || (menuDataGlobal.menuOptionsTimeout > 0);

	if (menuDataGlobal.menuOptionsSetQuickkey != 0)
	{
		snprintf(buf, bufferLen, "%s %c", currentLanguage->set_quickkey, menuDataGlobal.menuOptionsSetQuickkey);
		menuDisplayTitle(buf);
		ucDrawChoice(CHOICES_OKARROWS, true);

		if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_1)
		{
			voicePromptsInit();
			voicePromptsAppendLanguageString(&currentLanguage->set_quickkey);
			voicePromptsAppendPrompt(PROMPT_0 + (menuDataGlobal.menuOptionsSetQuickkey - '0'));
		}
	}
	else if (settingOption == false)
	{
		menuDisplayTitle(menuTitle);
	}

	return settingOption;
}


// --- DTMF contact list playback ---

static uint32_t dtmfGetToneDuration(uint32_t duration)
{
	bool starOrHash = ((uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr] == 14) || (uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr] == 15));

	/*
	 * https://www.sigidwiki.com/wiki/Dual_Tone_Multi_Frequency_(DTMF):
	 *    Standard Whelen timing is 40ms tone, 20ms space, where standard Motorola rate is 250ms tone, 250ms space.
	 *    Federal Signal ranges from 35ms tone 5ms space to 1000ms tone 1000ms space.
	 *    Genave Superfast rate is 20ms tone 20ms space. Genave claims their decoders can even respond to 20ms tone 5ms space.
	 *
	 *
	 * ETSI: https://www.etsi.org/deliver/etsi_es/201200_201299/20123502/01.01.01_60/es_20123502v010101p.pdf
	 * 4.2.4 Signal timing
	 *    4.2.4.1 Tone duration
	 *      Where the DTMF signalling tone duration is controlled automatically by the transmitter, the duration of any individual
	 *      DTMF tone combination sent shall not be less than 65 ms. The time shall be measured from the time when the tone
	 *      reaches 90 % of its steady-state value, until it has dropped to 90 % of its steady-state value.
	 *
	 *         NOTE: For correct operation of supplementary services such as SCWID (Spontaneous Call Waiting
	 *               Identification) and ADSI (Analogue Display Services Interface), DTMF tone bursts should not be longer
	 *               than 90 ms.
	 *
	 *    4.2.4.2 Pause duration
	 *       Where the DTMF signalling pause duration is controlled automatically by the transmitter the duration of the pause
	 *       between any individual DTMF tone combination shall not be less than 65 ms. The time shall be measured from the time
	 *       when the tone has dropped to 10 % of its steady-state value, until it has risen to 10 % of its steady-state value.
	 *
	 *         NOTE: In order to ensure correct reception of all the digits in a network address sequence, some networks may
	 *               require a sufficient pause after the last DTMF digit signalled and before normal transmission starts.
	 */

	// First digit
	if ((uiDataGlobal.DTMFContactList.poPtr == 0) && (uiDataGlobal.DTMFContactList.durations.fstDur > 0))
	{
		/*
		 * First digit duration:
		 *    - Example 1??š "DTMF rate" is set to 10 digits per second (duration is 50 milliseconds).
		 *        The first digit time is set to 100 milliseconds. Thus, the actual length of the first digit duration is 150 milliseconds.
		 *        However, if the launch starts with a "*" or "#" tone, the intercom will compare the duration with "* and #" and whichever
		 *        is longer for both.
		 *    - Example 2??š "DTMF rate" is set to 10 digits per second (duration is 50 milliseconds).
		 *        The first digit time is set to 100 milliseconds. "* And # tone" is set to 500 milliseconds.
		 *        Thus, the actual length of the first "*" or "#" tone is 550 milliseconds.
		 */
		return ((starOrHash ? (uiDataGlobal.DTMFContactList.durations.otherDur * 100) : (uiDataGlobal.DTMFContactList.durations.fstDur * 100)) + duration);
	}

	/*
	 * '*' '#' Duration:
	 *    - Example 1??š "DTMF rate" is set to 10 digits per second (duration is 50 milliseconds).
	 *        "* And # tone" is set to 500 milliseconds. Thus, the actual length of "* and # sounds" is 550 milliseconds.
	 *        However, if the launch starts with * and # sounds, the intercom compares the duration of the pitch with
	 *        the "first digit time" and uses the longer one of the two.
	 *    - Example 2??š "DTMF rate" is set to 10 digits per second (duration is 50 milliseconds).
	 *        The first digit time is set to 100 milliseconds. "* And # tone" is set to 500 milliseconds.
	 *        Therefore, the actual number of the first digit * or # is 550 milliseconds.
	 */
	return ((starOrHash ? (uiDataGlobal.DTMFContactList.durations.otherDur * 100) : 0) + duration);
}


static void dtmfProcess(void)
{
	if (uiDataGlobal.DTMFContactList.poLen == 0U)
	{
		return;
	}

	if (PITCounter > uiDataGlobal.DTMFContactList.nextPeriod)
	{
		uint32_t duration = (1000 / (uiDataGlobal.DTMFContactList.durations.rate * 2));
		bool pause=uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr] == 0x20;
		if (pause)
		{
			uiDataGlobal.DTMFContactList.inTone=false;
			if (trxTransmissionEnabled)
			{
				trxSelectVoiceChannel(AT1846_VOICE_CHANNEL_NONE);
				trxDisableTransmission();

				trxTransmissionEnabled = false;
				trxSetRX();
				LEDs_PinWrite(GPIO_LEDgreen, Pin_LEDgreen, 0);
			}

			uiDataGlobal.DTMFContactList.nextPeriod = PITCounter + 10000;
			// Keep pausing while carrier is detected, e.g. response from hotspot 
			if (!trxCarrierDetected())
				uiDataGlobal.DTMFContactList.poPtr++;

			return;
		}
		else if (uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr] != 0xFFU)
		{
			// Set voice channel (and tone), accordingly to the next inTone state
			if (uiDataGlobal.DTMFContactList.inTone == false)
			{
				trxSetDTMF(uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr]);
				trxSelectVoiceChannel(AT1846_VOICE_CHANNEL_DTMF);
			}
			else
			{
				trxSelectVoiceChannel(AT1846_VOICE_CHANNEL_NONE);
			}
			
			uiDataGlobal.DTMFContactList.inTone = !uiDataGlobal.DTMFContactList.inTone;
		}
		else
		{
			// Pause after last digit
			if (uiDataGlobal.DTMFContactList.inTone)
			{
				uiDataGlobal.DTMFContactList.inTone = false;
				trxSelectVoiceChannel(AT1846_VOICE_CHANNEL_NONE);
				uiDataGlobal.DTMFContactList.nextPeriod = PITCounter + ((duration + (uiDataGlobal.DTMFContactList.durations.libreDMR_Tail * 100)) * 10U);
				return;
			}
		}

		if (uiDataGlobal.DTMFContactList.inTone)
		{
			// Move forward in the sequence, set tone duration
			uiDataGlobal.DTMFContactList.nextPeriod = PITCounter + (dtmfGetToneDuration(duration) * 10U);
			uiDataGlobal.DTMFContactList.poPtr++;
		}
		else
		{
			// No next character, last iteration pause has already been processed.
			// Move the pointer (offset) beyond the end of the sequence (handled in the next statement)
			if (uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr] == 0xFFU)
			{
				uiDataGlobal.DTMFContactList.poPtr++;
			}
			else
			{
				// Set pause time in-between tone duration
				uiDataGlobal.DTMFContactList.nextPeriod = PITCounter + (duration * 10U);
			}
		}

		if (uiDataGlobal.DTMFContactList.poPtr > uiDataGlobal.DTMFContactList.poLen)
		{
			uiDataGlobal.DTMFContactList.poPtr = 0U;
			uiDataGlobal.DTMFContactList.poLen = 0U;
		}
	}
}

void dtmfSequenceReset(void)
{
	uiDataGlobal.DTMFContactList.poLen = 0U;
	uiDataGlobal.DTMFContactList.poPtr = 0U;
	uiDataGlobal.DTMFContactList.isKeying = false;
}

bool dtmfSequenceIsKeying(void)
{
	return uiDataGlobal.DTMFContactList.isKeying;
}

void dtmfSequencePrepare(uint8_t *seq, bool autoStart)
{
	uint8_t len = 16U;

	dtmfSequenceReset();

	memcpy(uiDataGlobal.DTMFContactList.buffer, seq, 16);
	uiDataGlobal.DTMFContactList.buffer[16] = 0xFFU;

	// non empty
	if (uiDataGlobal.DTMFContactList.buffer[0] != 0xFFU)
	{
		// Find the sequence length
		for (uint8_t i = 0; i < 16; i++)
		{
			if (uiDataGlobal.DTMFContactList.buffer[i] == 0xFFU)
			{
				len = i;
				break;
			}
		}

		uiDataGlobal.DTMFContactList.poLen = len;
		uiDataGlobal.DTMFContactList.isKeying = (autoStart ? (len > 0) : false);
	}
}

void dtmfSequenceStart(void)
{
	if (uiDataGlobal.DTMFContactList.isKeying == false)
	{
		uiDataGlobal.DTMFContactList.isKeying = (uiDataGlobal.DTMFContactList.poLen > 0);
	}
}

void dtmfSequenceStop(void)
{
	uiDataGlobal.DTMFContactList.poLen = 0U;
}

static uint16_t dtmfCTCSSDCSSquelchTail=0;
#ifndef CTCSSDCS_TAIL
#define CTCSSDCS_TAIL 250
#endif

static bool HandleDTMFCTCSSDCSSquelchTail()
{
	if (currentChannelData->txTone == 0xffff) return false;

	if (dtmfCTCSSDCSSquelchTail)
	{
		dtmfCTCSSDCSSquelchTail--;
		if (dtmfCTCSSDCSSquelchTail==0)
			return false; // so the dtmf tx will be reset.
		return true;
	}
	
	bool isDCS = (codeplugGetCSSType(currentChannelData->txTone)& CSS_TYPE_DCS) ? true : false;

	dtmfCTCSSDCSSquelchTail = CTCSSDCS_TAIL;
	// clear whatever tone or DCS code was used so the tail can be txmitted without anything to allow the receiving radio to shut down its rx without a squelch tail.
	// If using DCS, however, send a 136.5 tone instead.
	trxSetTxCSS(isDCS ? 1365 : 0xffff); 		
	return true;
}

void dtmfSequenceTick(bool popPreviousMenuOnEnding)
{
	if (uiDataGlobal.DTMFContactList.isKeying)
	{
		bool pause=uiDataGlobal.DTMFContactList.buffer[uiDataGlobal.DTMFContactList.poPtr] == 0x20;

		if (!trxTransmissionEnabled && !pause)
		{
			rxPowerSavingSetState(ECOPHASE_POWERSAVE_INACTIVE);
			// Start TX DTMF, prepare for ANALOG
			trxSetModeAndBandwidth(RADIO_MODE_ANALOG, ((currentChannelData->flag4 & 0x02) == 0x02));
			trxSetTxCSS(currentChannelData->txTone);

			trxEnableTransmission();

			trxSelectVoiceChannel(AT1846_VOICE_CHANNEL_NONE);
			enableAudioAmp(AUDIO_AMP_MODE_RF);
			GPIO_PinWrite(GPIO_RX_audio_mux, Pin_RX_audio_mux, 1);
			uiDataGlobal.DTMFContactList.inTone = false;
			uiDataGlobal.DTMFContactList.nextPeriod = PITCounter + ((uiDataGlobal.DTMFContactList.durations.fstDigitDly * 100) * 10U); // Sequence preamble
		}

		// DTMF has been TXed, restore DIGITAL/ANALOG
		if (uiDataGlobal.DTMFContactList.poLen == 0U)
		{
			if (HandleDTMFCTCSSDCSSquelchTail())
				return;
			
			trxDisableTransmission();

			if (trxTransmissionEnabled)
			{
				// Stop TXing;
				trxTransmissionEnabled = false;
				trxSetRX();
				LEDs_PinWrite(GPIO_LEDgreen, Pin_LEDgreen, 0);

				trxSelectVoiceChannel(AT1846_VOICE_CHANNEL_MIC);
				disableAudioAmp(AUDIO_AMP_MODE_RF);
				if (currentChannelData->chMode == RADIO_MODE_ANALOG)
				{
					trxSetModeAndBandwidth(currentChannelData->chMode, ((currentChannelData->flag4 & 0x02) == 0x02));
					trxSetTxCSS(currentChannelData->txTone);
				}
				else
				{
					trxSetModeAndBandwidth(currentChannelData->chMode, false);// bandwidth false = 12.5Khz as DMR uses 12.5kHz
					trxSetDMRColourCode(currentChannelData->txColor);
				}
			}

			uiDataGlobal.DTMFContactList.isKeying = false;

			if (popPreviousMenuOnEnding)
			{
				menuSystemPopPreviousMenu();
			}

			return;
		}

		if (uiDataGlobal.DTMFContactList.poLen > 0U)
		{
			dtmfProcess();
		}
	}
}

 bool dtmfConvertCodeToChars(uint8_t *code, char *text, int maxSize)
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
		else if (code[i]==0x20)
			text[j++]='P'; // pause.
	}
	text[j] = 0;

	return true;
}

extern int toupper(int __c);

 bool dtmfConvertCharsToCode(char *text, uint8_t *code, int maxSize)
{// initialize to empty.
	if (!text || !*text || !code)
	{
		return false;
	}

	memset(code, 0xFFU, DTMF_CODE_MAX_LEN);
	for (int i = 0; (i < maxSize) && text[i]; i++)
	{
		bool pause=toupper(text[i])=='P';
		char *symbol = strchr(DTMF_AllowedChars, toupper(text[i]));
		if (!symbol && !pause)
		{
			return false;
		}
		code[i] =pause ? 0x20 : (symbol - DTMF_AllowedChars);
	}

	return true;
}

void resetOriginalSettingsData(void)
{
	originalNonVolatileSettings.magicNumber = 0xDEADBEEF;
}

bool ToggleFMBandwidth(uiEvent_t *ev, struct_codeplugChannel_t* channel)
{
	if (!channel)
		return false;
	
	if (BUTTONCHECK_DOWN(ev, BUTTON_SK1) || BUTTONCHECK_DOWN(ev, BUTTON_SK2))
		return false;
	
	if (!KEYCHECK_SHORTUP(ev->keys, KEY_STAR))
		return false;
	
	if (channel->chMode != RADIO_MODE_ANALOG)
		return false;
	uint8_t mask=channel->flag4;
	if (mask&0x02) // wide, set to narrow.
	mask&=~0x02;
	else // narrow, set to wide.
	mask|=0x02;
	channel->flag4=mask;
	
	trxSetModeAndBandwidth(channel->chMode, ((channel->flag4&0x02)==0x02));
	
	return true;
}
void AdjustTXFreqByRepeaterOffset(uint32_t* rxFreq,uint32_t* txFreq, int repeaterOffsetDirection)
{
	if (repeaterOffsetDirection==0)
	{
		*txFreq=*rxFreq;
		return;	
	}
	int bandType=trxGetBandFromFrequency(*rxFreq);
	bool isVHF=bandType==RADIO_BAND_VHF;
	bool isUHF=bandType==RADIO_BAND_UHF;
	
	uint16_t offset=0;
	if (isVHF)
	{
		offset=nonVolatileSettings.vhfOffset;
	}
	else if (isUHF)
	{
		offset=nonVolatileSettings.uhfOffset;
	}
	if (offset==0)
	{
		return;
	}
	
	uint32_t newFreq=(*rxFreq);
	if (repeaterOffsetDirection== 1)
	{
		newFreq+=(offset*100);
	}
	else
	{
		newFreq-=(offset*100);
	}
	
	if (!trxCheckFrequencyInAmateurBand(newFreq))
	{
			return;
	}
	
	(*txFreq)=newFreq;
}

static bool AutoZoneCycleRepeaterOffset(menuStatus_t* newMenuStatus)
{
	bool IsAutoZone= AutoZoneIsCurrentZone(currentZone.NOT_IN_CODEPLUGDATA_indexNumber) && AutoZoneIsValid();

	if (!IsAutoZone)
		return false;
	
	voicePromptsInit();
	
	int direction=0;
	if (autoZone.flags & AutoZoneDuplexAvailable)
	{
		direction =(autoZone.flags&AutoZoneOffsetDirectionPlus) ? 1 : -1;
	
		if ((autoZone.flags&AutoZoneDuplexEnabled)==0)
			autoZone.flags|=AutoZoneDuplexEnabled;
		else
			autoZone.flags&=~AutoZoneDuplexEnabled;
	
		if (autoZone.flags & AutoZoneDuplexEnabled)
		{
			if (direction > 0)
				voicePromptsAppendPrompt(PROMPT_PLUS);
			else
				voicePromptsAppendPrompt(PROMPT_MINUS);
		}		
	}
	if ((autoZone.flags & AutoZoneDuplexAvailable)==0 || (autoZone.flags & AutoZoneDuplexEnabled)==0)
	{
		direction=0;
		voicePromptsAppendLanguageString(&currentLanguage->none);
		if (newMenuStatus)
			*newMenuStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
	}
	if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_2)
		voicePromptsPlay();
	AutoZoneGetFrequenciesForIndex(uiDataGlobal.currentSelectedChannelNumber, &currentChannelData->rxFreq, &currentChannelData->txFreq);
	AutoZoneApplyChannelRestrictions(uiDataGlobal.currentSelectedChannelNumber, currentChannelData);
	trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
	return true;
}
	
void CycleRepeaterOffset(menuStatus_t* newMenuStatus)
{
	if (AutoZoneCycleRepeaterOffset(newMenuStatus))
		return;
	
	voicePromptsInit();
	if (uiDataGlobal.repeaterOffsetDirection==0)
	{
		uiDataGlobal.repeaterOffsetDirection=1;
		voicePromptsAppendPrompt(PROMPT_PLUS);
	}
	else if (uiDataGlobal.repeaterOffsetDirection == 1)
	{
		uiDataGlobal.repeaterOffsetDirection=-1;
		voicePromptsAppendPrompt(PROMPT_MINUS);
	}
	else
	{
		uiDataGlobal.repeaterOffsetDirection=0;
		voicePromptsAppendLanguageString(&currentLanguage->none);
		if (newMenuStatus)
		{
			*newMenuStatus |= (MENU_STATUS_LIST_TYPE | MENU_STATUS_FORCE_FIRST);
		}
	}
	if (nonVolatileSettings.audioPromptMode >= AUDIO_PROMPT_MODE_VOICE_LEVEL_2)
		voicePromptsPlay();
	AdjustTXFreqByRepeaterOffset(&currentChannelData->rxFreq, &currentChannelData->txFreq, uiDataGlobal.repeaterOffsetDirection);
	trxSetFrequency(currentChannelData->rxFreq, currentChannelData->txFreq, DMR_MODE_AUTO);
}

bool IsBitSet(uint8_t bits, int whichBit)
{
	if (whichBit < 1 || whichBit > 8)
		return false;
	
	whichBit--; // 1 is bit 0;
	
	uint8_t bit=1<<whichBit;
	return bits & bit ? true : false;
}

 void SetBit(uint8_t* bits, int whichBit, bool set)
{
	if (!bits)
		return;
	if (whichBit < 1 || whichBit > 8)
		return;
	
	whichBit--; // 1 is bit 0;
	uint8_t bit=1<<whichBit;
	if (set)
		*bits|=bit;
	else
		*bits&=~bit;
}

static bool IsLastHeardContactRelevant()
{
	if (!LinkHead) return false;
	if (LinkHead->id==0) return false;
	if (LinkHead->id==trxDMRID) return false; // one's own ID.
	if ((fw_millis() - lastHeardUpdateTime) > 10000) return false; // If it is older than 10 seconds.
	if (trxGetMode()==RADIO_MODE_ANALOG) return false;
	if ((trxDMRModeRx == DMR_MODE_DMO) && ((dmrMonitorCapturedTS !=-1) && (dmrMonitorCapturedTS != trxGetDMRTimeSlot())))
		return false;

	return true;
}

void AnnounceLastHeardContact()
{
	if (!IsLastHeardContactRelevant()) return;
	
	uint8_t offset=0;
	if (strncmp(LinkHead->contact, "ID:", 3)==0 && LinkHead->contact[3])
	{
		offset=3;
	}
	char buffer[MAX_DMR_ID_CONTACT_TEXT_LENGTH]="\0";
	if (LinkHead->talkerAlias[0])
		strcpy(buffer, LinkHead->talkerAlias);
	else if (LinkHead->contact[0])
		strcpy(buffer, LinkHead->contact+offset);
	// terminate at first space so we only read the callsign or first piece of data.
	int endOffset=0;
	while (buffer[endOffset] && buffer[endOffset]!=' ')
	{
		endOffset++;
	}
	if (endOffset)
		buffer[endOffset]='\0';
	if (buffer[0])
		voicePromptsAppendString(buffer);
	else
		voicePromptsAppendInteger(LinkHead->id);
	uint32_t tg = (LinkHead->talkGroupOrPcId & 0xFFFFFF);
	
	if ((trxTalkGroupOrPcId != tg) && (LinkHead->talkgroup[0]))
	{
		voicePromptsAppendString(LinkHead->talkgroup);
	}
}

void AnnounceLastHeardContactIfNeeded()
{
	if (lastHeardNeedsAnnouncementTimer ==-1) return;

	if (!IsLastHeardContactRelevant()) return;
	
	if (voicePromptsIsPlaying()) return;
	
	if (trxIsTransmitting)
	{
		lastHeardNeedsAnnouncementTimer = -1;
		return;
	}
// at this point, if we have detected that the DMR ID has changed, queue it for speaking but do not speak it until reception has finished.
// If we don't queue it, sk1 will still speak the old callsign if pressed.
	if (lastHeardNeedsAnnouncementTimer==LAST_HEARD_TIMER_TIMEOUT)
	{
		voicePromptsInit();
		AnnounceLastHeardContact(); // just queue, do not play.
		lastHeardNeedsAnnouncementTimer--; // so we do  not do it again until it changes.
		return;
	}
	
	if (getAudioAmpStatus() & (AUDIO_AMP_MODE_RF | AUDIO_AMP_MODE_BEEP | AUDIO_AMP_MODE_PROMPT))
	{// wait till reception has finished.
		lastHeardNeedsAnnouncementTimer=LAST_HEARD_TIMER_TIMEOUT-1; // avoid requeueing the DMR ID unless it actually changes.
		lastHeardUpdateTime=fw_millis();
		return;
	}
	
	if (lastHeardNeedsAnnouncementTimer > 0)
	{
		lastHeardNeedsAnnouncementTimer--;
		return; // wait for timer to expire, start counting  after end of transmission.
	}

	lastHeardNeedsAnnouncementTimer=-1; // reset.
	lastHeardUpdateTime=fw_millis(); // start from now because last TX may have been longer than our timeout!
	if ((nonVolatileSettings.audioPromptMode < 1) || (nonVolatileSettings.bitfieldOptions&BIT_ANNOUNCE_LASTHEARD)==0)
		return;// Don't automatically announce it.
	voicePromptsInit();

	AnnounceLastHeardContact();
	
	voicePromptsPlay();
}

bool ScanShouldSkipFrequency(uint32_t freq)
{
	for (int i = 0; i < MAX_ZONE_SCAN_NUISANCE_CHANNELS; i++)
	{
		if (uiDataGlobal.Scan.nuisanceDelete[i] == -1)
		{
			break;
		}
		else
		{
			if(uiDataGlobal.Scan.nuisanceDelete[i] == freq)
			{
				return true;
			}
		}
	}
	return false;	
}
static void PlayAndResetCustomVoicePromptIndex()
{
	if (customVoicePromptIndex==0xff) return;
	
	if (customVoicePromptIndex > GetMaxCustomVoicePrompts())
	{
		nextKeyBeepMelody = (int *)MELODY_ERROR_BEEP;
		customVoicePromptIndex=0xff; // reset 
		return;	
	}

	voicePromptsInit();
	voicePromptsAppendPrompt(VOICE_PROMPT_CUSTOM+customVoicePromptIndex);
	voicePromptsPlay();
	customVoicePromptIndex=0xff;
}
/*
When Contact Details is invoked from SK1+hash, regardless of the fact that the cursor is in an edit, if the user edits the voice tag, we should save it associated with the DMR voice tag and not the custom voice prompt in the edit.
*/
static bool ForceUseOfVoiceTagIndex()
{
	if (contactListContactData.ringStyle ==0) return false;
	
	int currentMenu = menuSystemGetCurrentMenuNumber();
	if (currentMenu==MENU_CONTACT_LIST) return true; // called from DMR contact list.
	
	if (currentMenu!= MENU_CONTACT_DETAILS) return false;
	if (GetDMRContinuousSave()) return false;
	// This is being called from SK1+hash invocation so even though we might be focused on an edit, force the use of the voice tag index rather than any custom prompt.
	return true;
}
			
/*
Handle custom voice prompts.
While SK1 is being held down, keep track of, and combine, the digits being pressed (actually released) until SK1 is released
or the number entered so far is already two digits long, at which time, play the corresponding custom voice prompt.
If the last digit is held down for a long hold, save the corresponding voice prompt.
*/
bool HandleCustomPrompts(uiEvent_t *ev, char* phrase)
{
	if (nonVolatileSettings.audioPromptMode < AUDIO_PROMPT_MODE_VOICE_LEVEL_1) return false;
	
	if ((customVoicePromptIndex < 0xff) && BUTTONCHECK_DOWN(ev, BUTTON_SK1)==0 && (BUTTONCHECK_DOWN(ev, BUTTON_SK2) == 0))
	{// SK1 was released and customVoicePromptIndex has been set so play the corresponding custom voice prompt.
		if (customVoicePromptIndex==0)
			customVoicePromptIndex=10; // shortcut if SK1 plus 0 is quickly pressed and released, play prompt 10.
		PlayAndResetCustomVoicePromptIndex();
		return true;
	}
	//SK1+* save to next available custom voice prompt slot.
	if ((ev->buttons & BUTTON_SK1) &&KEYCHECK_SHORTUP(ev->keys, KEY_STAR))
	{// save to the next available custom voice prompt slot.
		customVoicePromptIndex=GetNextFreeVoicePromptIndex(false);
		SaveCustomVoicePrompt(customVoicePromptIndex, phrase);
		customVoicePromptIndex=0xff; // reset.
		keyboardReset(); // reset the keyboard also.
		return true;
	}
	// SK1+# associate last played DMR with ID and save to contact.
	if ((ev->buttons & BUTTON_SK1) && IsLastHeardContactRelevant() && KEYCHECK_SHORTUP(ev->keys, KEY_HASH))
	{// associate last recorded DMR with last heard ID.
		char phrase[16]="\0";
		snprintf(phrase, 16, "%d", LinkHead->id);
		memset(&contactListContactData, 0, sizeof(contactListContactData));
		int contactIndex=codeplugContactIndexByTGorPC((LinkHead->id & 0x00FFFFFF), CONTACT_CALLTYPE_PC, &contactListContactData, 0);
		uint8_t DMRVTIndex=contactListContactData.ringStyle > 0 ? contactListContactData.ringStyle : GetNextFreeVoicePromptIndex(true);
		SetDMRContinuousSave(false); // This stops incoming ambe frames overwriting the voice tag while we give the user an opportunity to edit it.
		SaveCustomVoicePrompt(DMRVTIndex, phrase);
		uiDataGlobal.currentSelectedContactIndex=contactIndex==-1? codeplugContactGetFreeIndex() : contactIndex;
		if (contactIndex ==-1)
		{
			memset(&contactListContactData, 0, sizeof(contactListContactData));
			contactListContactData.NOT_IN_CODEPLUGDATA_indexNumber=uiDataGlobal.currentSelectedContactIndex;
			contactListContactData.tgNumber=LinkHead->id;
			contactListContactData.callType=CONTACT_CALLTYPE_PC;
			contactListContactData.ringStyle=DMRVTIndex;
		}
		menuSystemPushNewMenu(MENU_CONTACT_DETAILS);
		return true;
	}
	bool IsVoicePromptEditMode=voicePromptsGetEditMode();
	if (IsVoicePromptEditMode)
	{
			if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
		{// Try and determine where to save the edited audio.
			if (ForceUseOfVoiceTagIndex())
				customVoicePromptIndexToSave=contactListContactData.ringStyle;
			if (customVoicePromptIndexToSave!=0xff)
				SaveCustomVoicePrompt(customVoicePromptIndexToSave, 0);
			customVoicePromptIndexToSave=0xff;
			voicePromptsSetEditMode(false);
			keyboardReset();
			ev->keys.key=0;
			ev->buttons=0;
			ev->events |=FUNCTION_EVENT;
			ev->function = FUNC_REDRAW;
			
			return false; // so next menu handler gets called and screen gets updated.
		}
		if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
		{
			voicePromptsAdjustEnd(false, 0, true);
			voicePromptsAdjustEnd(true, 0, true);
			voicePromptsSetEditMode(false);

			customVoicePromptIndex=0xff;
			keyboardReset();
			ev->keys.key=0;
			ev->buttons=0;
			ev->events |=FUNCTION_EVENT;
			ev->function = FUNC_REDRAW;
			
			return false; // so next menu handler gets called and screen is updated.
		}
		if ((ev->keys.key==0) && BUTTONCHECK_SHORTUP(ev, BUTTON_SK1))
		{
			ReplayDMR();
			keyboardReset();
			return true;
		}

		// up/down adjust start.
		// left/right adjust end.
		bool longHoldLeftRight=KEYCHECK_LONGDOWN(ev->keys, KEY_LEFT) || KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_LEFT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_RIGHT);
		bool leftRight=KEYCHECK_SHORTUP(ev->keys, KEY_LEFT) || KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT) || longHoldLeftRight;
		bool longHoldUpDown=KEYCHECK_LONGDOWN(ev->keys, KEY_UP) || KEYCHECK_LONGDOWN(ev->keys, KEY_DOWN) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_UP) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_DOWN);;
		bool upDown=KEYCHECK_SHORTUP(ev->keys, KEY_UP) || KEYCHECK_SHORTUP(ev->keys, KEY_DOWN) || longHoldUpDown;
		bool reverse=KEYCHECK_SHORTUP(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN(ev->keys, KEY_RIGHT) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_RIGHT) || KEYCHECK_SHORTUP(ev->keys, KEY_DOWN) || KEYCHECK_LONGDOWN(ev->keys, KEY_DOWN) || KEYCHECK_LONGDOWN_REPEAT(ev->keys, KEY_DOWN);//right is increasing the clip region from the end, slightly counter intuitive.
		bool longHold=longHoldLeftRight || longHoldUpDown;
		if (leftRight || upDown)
		{
			int step=longHold ? 3 : 1;
			voicePromptsAdjustEnd(upDown, reverse ? -step : step, false);
			return true;
		}
		// * from edit mode will copy the last custom voice prompt spoken back to the edit buffer so it can be edited.
		if (KEYCHECK_SHORTUP(ev->keys, KEY_STAR))
		{
			customVoicePromptIndexToSave=voicePromptsGetLastCustomPromptNumberAnnounced();
			voicePromptsCopyCustomPromptToEditBuffer(customVoicePromptIndexToSave);
			ReplayDMR();
			return true;
		}
		// hash autotrim
		if (KEYCHECK_SHORTUP(ev->keys, KEY_HASH))
		{
			if (BUTTONCHECK_DOWN(ev, BUTTON_SK2))
				AnnounceEditBufferLength();
			else
				voicePromptsEditAutoTrim();
			return true;
		}
	}
	// When contact details was invoked using SK1+# we prohibit DMRContinuousSave to give time for the user to choose to listen to and possibly edit the audio without it being overwritten.
	// As soon as we detect this screen is not the active screen, we re-enable it.
	// We don't automatically enable sound edit mode as it might be confusing to have to press Green twice.
	if (!GetDMRContinuousSave() && (menuSystemGetCurrentMenuNumber() != MENU_CONTACT_DETAILS))
		SetDMRContinuousSave(true);
	// SK1 is not being held down on its own.
	if (((ev->buttons & BUTTON_SK1) && (ev->buttons & BUTTON_SK2)==0)==false) return IsVoicePromptEditMode;
	// SK1+green enter edit mode.
	if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN) && !IsVoicePromptEditMode)
	{
		voicePromptsSetEditMode(true); // so nothing else gets written to circular buffer while we're allowing edits.
		return true;
	}
	
	// No number is going down, coming up or being held.
	if (!KEYCHECK_PRESS_NUMBER(ev->keys) && !KEYCHECK_DOWN_NUMBER(ev->keys) && !KEYCHECK_SHORTUP_NUMBER(ev->keys)) return IsVoicePromptEditMode;
	
	int keyval=menuGetKeypadKeyValueEx(ev, true, false);
	if (keyval > 9) return IsVoicePromptEditMode;
	
	if (KEYCHECK_LONGDOWN_NUMBER(ev->keys))
	{
		// a digit is being held down, save the appropriate custom voice prompt:
		// If customVoicePromptIndex is 0, save prompt 10 shortcut,
		// If it is still reset, set to the current keyval.
		// Otherwise this is the second digit being held down of a series, combine with the prior value and save.
		if (customVoicePromptIndex==0 || customVoicePromptIndex==0xff)
			customVoicePromptIndex=(keyval==0) ? 10 : keyval;
		else
			customVoicePromptIndex=(10*customVoicePromptIndex)+keyval;
		if ((phrase==0) && (menuSystemGetCurrentMenuNumber() ==MENU_CONTACT_DETAILS) && (menuDataGlobal.currentItemIndex ==0 || menuDataGlobal.currentItemIndex==1))
			phrase=GetCurrentEditBuffer();
		SaveCustomVoicePrompt(customVoicePromptIndex, phrase);
		customVoicePromptIndex=0xff; // reset.
		keyboardReset(); // reset the keyboard also.
		return true;
	}
	else if (KEYCHECK_SHORTUP_NUMBER(ev->keys))
	{// digit is being released, either set customVoicePromptIndex or combine with prior value as appropriate.
		if (customVoicePromptIndex==0xff)
		{// just set it to the current digit.
			customVoicePromptIndex=keyval;
			// Announce the digit rather than allowing the default keypad beep 
			//as this digit will either result in a prompt being spoken, if SK1 is released, 
			// or be combined with the prior digit if 2nd in a series with SK1 still held down.
			voicePromptsInit();
			voicePromptsAppendInteger(customVoicePromptIndex);
			voicePromptsPlay();
		}
		else
			customVoicePromptIndex=(10*customVoicePromptIndex)+keyval; // combine.
		if (customVoicePromptIndex > 9)
		{// play straight away since we can't add any more digits.
			PlayAndResetCustomVoicePromptIndex();
		}
		return true;
	}
		
	return false;
}

void ShowEditAudioClipScreen(uint16_t start, uint16_t end)
{
	ucClearBuf();
	menuDisplayTitle("Edit Audio Clip");
	
	char buffer[SCREEN_LINE_BUFFER_SIZE];
	snprintf(buffer, SCREEN_LINE_BUFFER_SIZE, "S %d.%02d - E %d.%02d", start/1000, start%1000, end/1000, end%1000);

	ucPrintCore(0, DISPLAY_Y_POS_MENU_START, buffer, FONT_SIZE_3, TEXT_ALIGN_LEFT, false);
	
	ucRender();
}
