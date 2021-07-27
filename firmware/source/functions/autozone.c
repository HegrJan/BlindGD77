/*
 * Copyright (C) 2021
 * Joseph Stephen VK7JS
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
#include <stdio.h>
#include "functions/autozone.h"
#include "utils.h"
#include "functions/settings.h"
#include "functions/sound.h"

static struct_AutoZoneParams_t* autoZone = &nonVolatileSettings.autoZone;

uint16_t AutoZoneGetTotalChannels()
{
	return autoZone->totalChannels;
}

bool AutoZoneIsValid()
{
	if ((autoZone->flags & AutoZoneEnabled)==0) // turned off.
	return false;
	if (autoZone->name[0]==0xff || autoZone->name[0]==0)
		return false;
	if (autoZone->endFrequency <= autoZone->startFrequency)
		return false;
	if (autoZone->channelSpacing==0)
		return false;
	
	return true;	
}

bool AutoZoneIsCurrentZone(int zoneIndex)
{
	return zoneIndex == -2; // allchannels is -1, AutoZones are -2, -3, etc.
}

static void ApplyUHFCBRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	// Channels 22, 23, 61, 62 & 63 are rx only.
	switch (index)
	{
		case 22:
		case 23:
		case 61:
		case 62:
		case 63:
			channelBuf->flag4 |= 0x04;// rx only. 
			break;
		default:
			channelBuf->flag4 &=~0x04;// turn off rx only flag in case it was on due to visiting channels 22, 23, etc. 
			break;
	}
	if ((index > 8 && index < 41) || (index > 48)) // force simplex since repeaters are only allowed on 1-8 and 41-48.
	{
		channelBuf->txFreq=channelBuf->rxFreq;
		autoZone->flags&=~(AutoZoneDuplexEnabled|AutoZoneDuplexAvailable);
	}
	else
		autoZone->flags |=AutoZoneDuplexAvailable;
}

static void ApplyMarineRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	// Need to force simplex on non duplex channels.
	bool duplexAllowed=(index >=1 && index <=7) 
	|| (index >=18 && index <=26) 
	|| (index >=29 && index <=35) // Physical indices 29 to 35 correspond to named channels 60 to 66.
	|| (index >=47 && index <=55); // physical channels 47 to 55 correspond to named channels 78 to 86.
	if (duplexAllowed)
	{
		autoZone->flags |=AutoZoneDuplexAvailable;
	}
	else
	{	// Force simplex.
		channelBuf->rxFreq=channelBuf->txFreq;
		autoZone->flags&=~(AutoZoneDuplexEnabled|AutoZoneDuplexAvailable);
	}
}

static  void ApplyGMRSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	// duplex is not allowed on interstitials
	if (index < 9)
		autoZone->flags|=AutoZoneDuplexAvailable;
	else
		autoZone->flags&=~AutoZoneDuplexAvailable;
}

/*
Apply specific hacks, e.g. channels 22, 23, 61-63 in UHF CB in Australian band are RX only.
*/
 void AutoZoneApplyChannelRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	switch (autoZone->type)
	{
		case 	AutoZone_AU_UHFCB:
		ApplyUHFCBRestrictions(index, channelBuf);
		break;
		case AutoZone_MRN:
			ApplyMarineRestrictions(index, channelBuf);
			break;
		case AutoZone_NOAA:
			channelBuf->flag4|=0x04; // RX only.
			break;
		case AutoZone_GMRS:
			ApplyGMRSRestrictions(index, channelBuf);
			break;
		default:
			return;
	}	
}	

static void InitializeAU_UHFCB()
{
	strcpy(autoZone->name, "AU UHF CB");
	autoZone->flags=AutoZoneEnabled|AutoZoneInterleaveChannels | AutoZoneOffsetDirectionPlus | AutoZoneNarrow;
	autoZone->type=AutoZone_AU_UHFCB;
	autoZone->startFrequency=47642500; // mHz of first channel
	autoZone->endFrequency=47740000; // mHz of last channel (not including interleaving, channelspacing will be added to this to get absolute end).
	autoZone->channelSpacing=2500; // kHz channel step x 100 (so for narrow we can divide by 2 without using float).
	autoZone->repeaterOffset=750; // kHz.
	autoZone->priorityChannelIndex=5;// ch5
	autoZone->curChannelIndex=1;
	autoZone->rxTone=autoZone->txTone=CODEPLUG_CSS_TONE_NONE;
	uint16_t adjacentChannelSpacing=autoZone->channelSpacing;
	if (autoZone->flags & AutoZoneInterleaveChannels)
		adjacentChannelSpacing/=2; // for the purposes of calculating the number of channels.
	uint16_t totalChannels =  ((autoZone->endFrequency+autoZone->channelSpacing) - autoZone->startFrequency)/adjacentChannelSpacing;
	if (totalChannels > codeplugChannelsPerZone)
		totalChannels=codeplugChannelsPerZone; // max 80.
	autoZone->totalChannels=totalChannels;
}

static void InitializeMarine()
{
	strcpy(autoZone->name, "MRN");
	autoZone->flags=AutoZoneEnabled|AutoZoneInterleaveChannels | AutoZoneInterleavingStartsPrior | AutoZoneHasBaseIndex | AutoZoneSimplexUsesTXFrequency | AutoZoneDuplexAvailable;

	autoZone->type=AutoZone_MRN;
	autoZone->startFrequency=16065000; // mHz of first channel
	autoZone->endFrequency=16200000; // mHz of last channel (not including interleaving, channelspacing will be added to this to get absolute end).
	autoZone->channelSpacing=5000; // kHz channel step x 100 (so for narrow we can divide by 2 without using float).
	autoZone->repeaterOffset=4600; // kHz. Relevant for ch 01-07 and ch18-28, and 60-66, and 78-86
	autoZone->priorityChannelIndex=16;// ch5
	autoZone->curChannelIndex=1;
	autoZone->rxTone=autoZone->txTone=CODEPLUG_CSS_TONE_NONE;
	uint16_t adjacentChannelSpacing=autoZone->channelSpacing;
	if (autoZone->flags & AutoZoneInterleaveChannels)
		adjacentChannelSpacing/=2; // for the purposes of calculating the number of channels.
	uint16_t totalChannels =  ((autoZone->endFrequency+autoZone->channelSpacing) - autoZone->startFrequency)/adjacentChannelSpacing;
	if (totalChannels > codeplugChannelsPerZone)
		totalChannels=codeplugChannelsPerZone; // max 80.
	autoZone->totalChannels=totalChannels;
	autoZone->baseChannelNumberStart=1;
	autoZone->interleaveChannelNumberStart=60;
}

static void InitializeNOAA()
{
//162.400, 162.425, 162.450, 162.475, 162.500, 162.525, and 162.550	
	strcpy(autoZone->name, "NOAA");
	autoZone->flags=AutoZoneEnabled;
	autoZone->type=AutoZone_NOAA;
	autoZone->startFrequency=16240000; // mHz of first channel
	autoZone->endFrequency=16255000; // mHz of last channel (not including interleaving, channelspacing will be added to this to get absolute end).
	autoZone->channelSpacing=2500; // kHz channel step x 100 (so for narrow we can divide by 2 without using float).
	autoZone->curChannelIndex=1;
	autoZone->rxTone=autoZone->txTone=CODEPLUG_CSS_TONE_NONE;
	uint16_t totalChannels =  ((autoZone->endFrequency+autoZone->channelSpacing) - autoZone->startFrequency)/autoZone->channelSpacing;
	autoZone->totalChannels=totalChannels;
}
		
static void InitializeGMRS()
{// 462.55 through 462.725 25 kHz steps.
	strcpy(autoZone->name, "GMRS");
	autoZone->flags=AutoZoneEnabled | AutoZoneOffsetDirectionPlus | AutoZoneInterleaveChannels | AutoZoneNarrow | AutoZoneHasBaseIndex | AutoZoneHasBankAtOffset;
	autoZone->repeaterOffset=5000; // kHz.
	autoZone->type=AutoZone_GMRS;
	autoZone->startFrequency=46255000; // mHz of first channel
	autoZone->endFrequency=46272500; // mHz of last channel (not including interleaving, channelspacing will be added to this to get absolute end).
	autoZone->channelSpacing=2500; // kHz channel step x 100 (so for narrow we can divide by 2 without using float).
	autoZone->curChannelIndex=1;
	autoZone->rxTone=autoZone->txTone=CODEPLUG_CSS_TONE_NONE;
	autoZone->totalChannels= 15; // in each bank.
	autoZone->baseChannelNumberStart=15;
	autoZone->interleaveChannelNumberStart=1; // Because index is added to base, so physical channel 9 should be 1.
	autoZone->offsetBankChannelNumberStart=23;
	autoZone->offsetBankInterleavedChannelNumberStart=8;
}

void AutoZoneInitialize(AutoZoneType_t type)
{
		memset(autoZone, 0, sizeof(struct_AutoZoneParams_t));
	
	switch (type)
	{
		case	AutoZone_AU_UHFCB:
			InitializeAU_UHFCB();
			break;
		case AutoZone_MRN:
			InitializeMarine();
			break;
		case AutoZone_NOAA:
			InitializeNOAA();
			break;
		case AutoZone_GMRS:
			InitializeGMRS();
			break;
		default:
		break;
	}
}

bool AutoZoneGetZoneDataForIndex(int zoneNum, struct_codeplugZone_t *returnBuf)
{
	if (zoneNum != (codeplugZonesGetCount() - 2) || !AutoZoneIsValid())
		return false;
	
	// This is the autogenerated zone.
	int nameLen = SAFE_MIN(((int)sizeof(returnBuf->name)), ((int)strlen(autoZone->name)));
	// Codeplug name is 0xff filled, codeplugUtilConvertBufToString() handles the conversion
	memset(returnBuf->name, 0xff, sizeof(returnBuf->name));
	memcpy(returnBuf->name, autoZone->name, nameLen);
	int total=autoZone->totalChannels;
	if (autoZone->flags&AutoZoneHasBankAtOffset)
		total*=2;
	for (uint16_t channelIndex=0; channelIndex < total; ++channelIndex)
	{
		returnBuf->channels[channelIndex]=channelIndex+1;
	}
	returnBuf->NOT_IN_CODEPLUGDATA_numChannelsInZone = total;
	returnBuf->NOT_IN_CODEPLUGDATA_highestIndex = total;
	returnBuf->NOT_IN_CODEPLUGDATA_indexNumber = -2;// Set as -2 as this is not a real zone. Its the autogenerated zone.
	
	return true;
}

bool AutoZoneGetFrequenciesForIndex(uint16_t index, uint32_t* rxFreq, uint32_t* txFreq)
{
	if (!rxFreq || !txFreq)
		return false;
	if (!AutoZoneIsValid())
		return false;
	
	bool channelIsInBankAtOffset=false;
	if ((autoZone->flags&AutoZoneHasBankAtOffset) && (index > autoZone->totalChannels) && (index <= (autoZone->totalChannels*2)))
	{
		index-=autoZone->totalChannels;
		channelIsInBankAtOffset=true;
	}
	if (index > autoZone->totalChannels)
		return false;
	
	uint16_t multiplier=(index-1);
	// If interleaved, first half of channels start at startFrequency and the channel step is defined by autoZone->channelSpacing kHz.
	// Second half are offset by autoZone->channelSpacing/2 kHz between the first half of the channels
	// and may start prior.
	uint16_t interleaveOffset=0;
	uint16_t totalChannelsRoundedUpToEven=autoZone->totalChannels;
	if ((totalChannelsRoundedUpToEven%2)==1)
		totalChannelsRoundedUpToEven++; // so when we divide by 2, if odd number, we round up.
	if ((autoZone->flags&AutoZoneInterleaveChannels) && (index > totalChannelsRoundedUpToEven/2))
	{
		interleaveOffset=autoZone->channelSpacing/2;
		multiplier -= (totalChannelsRoundedUpToEven/2);
	}
	if (autoZone->flags&AutoZoneInterleavingStartsPrior)
		*rxFreq = autoZone->startFrequency-interleaveOffset+(multiplier * autoZone->channelSpacing);
	else
		*rxFreq = autoZone->startFrequency+interleaveOffset+(multiplier * autoZone->channelSpacing);
	*txFreq = *rxFreq;
	// adjust by repeater offset if duplex is on.
	if ((autoZone->flags&AutoZoneDuplexEnabled) && (autoZone->repeaterOffset > 0) && !channelIsInBankAtOffset)
	{
		if (autoZone->flags&AutoZoneOffsetDirectionPlus)
			(*txFreq)+=(autoZone->repeaterOffset*100);
		else
			(*txFreq)-=(autoZone->repeaterOffset*100);
	}
	else if (autoZone->flags&(AutoZoneSimplexUsesTXFrequency) || (channelIsInBankAtOffset && (autoZone->flags&AutoZoneHasBankAtOffset)))
	{
		if (autoZone->flags&AutoZoneOffsetDirectionPlus)
			(*txFreq)+=(autoZone->repeaterOffset*100);
		else
			(*txFreq)-=(autoZone->repeaterOffset*100);
		*rxFreq=*txFreq;
	}
	return true;
}

static uint16_t GetDisplayChannelNumber(uint16_t index)
{
		bool channelIsInBankAtOffset=false;
		if ((autoZone->flags&AutoZoneHasBankAtOffset) && (index > autoZone->totalChannels) && (index <=(autoZone->totalChannels*2)))
	{
		index-=autoZone->totalChannels;
		channelIsInBankAtOffset=true;
	}

	uint16_t channelNumber=index;
	
	if (autoZone->flags &AutoZoneHasBaseIndex)
	{
		uint16_t totalChannelsRoundedUpToEven=autoZone->totalChannels;
		if ((totalChannelsRoundedUpToEven%2)==1)
			totalChannelsRoundedUpToEven++; // so when we divide by 2, if odd number, we round up.
		// see which base offset we should use.
		int baseChannelOffset=0;
		if ((autoZone->flags&AutoZoneInterleaveChannels) && (index > totalChannelsRoundedUpToEven/2)) // its in the interleaved group.
		{
			baseChannelOffset=channelIsInBankAtOffset ? autoZone->offsetBankInterleavedChannelNumberStart : autoZone->interleaveChannelNumberStart;
		}
		else
			baseChannelOffset=channelIsInBankAtOffset ? autoZone->offsetBankChannelNumberStart : autoZone->baseChannelNumberStart;
		{
		}
		if ((autoZone->flags&AutoZoneInterleaveChannels) && (index > totalChannelsRoundedUpToEven/2))
			channelNumber=(index - totalChannelsRoundedUpToEven/2) + baseChannelOffset -1;
		else
			channelNumber+= baseChannelOffset -1;
	}
	return channelNumber;
}

static void AutoZoneGetChannelNameForIndex(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	memset(channelBuf->name, 0xff, 16);
	snprintf(channelBuf->name, 16, "%d", GetDisplayChannelNumber(index));
}

bool AutoZoneGetChannelData( uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	if (!channelBuf)
	{
		return false;
	}
	
	uint32_t rxFreq=0;
	uint32_t txFreq=0;
	
	if (!AutoZoneGetFrequenciesForIndex(index, &rxFreq, &txFreq))
	{
		return false;
	}

	// generate the channel name.
	AutoZoneGetChannelNameForIndex(index, channelBuf);
	channelBuf->rxFreq=rxFreq;
	channelBuf->txFreq=txFreq;
	channelBuf->chMode=(autoZone->flags & AutoZoneModeDigital) ? RADIO_MODE_DIGITAL : RADIO_MODE_ANALOG;
	channelBuf->libreDMR_Power=0; // From master unless overridden by restriction.
	channelBuf->rxTone=autoZone->rxTone;
	channelBuf->txTone=autoZone->txTone;

	if (autoZone->flags & AutoZoneNarrow)
		channelBuf->flag4&=~0x02; // clear.
	else
		channelBuf->flag4|=0x02; // bits... 0x80 = Power, 0x40 = Vox, 0x20 = ZoneSkip (AutoScan), 0x10 = AllSkip (LoneWoker), 0x08 = AllowTalkaround, 0x04 = OnlyRx, 0x02 = Channel width, 0x01 = Squelch
	AutoZoneApplyChannelRestrictions(index, channelBuf);
	return true;
}

