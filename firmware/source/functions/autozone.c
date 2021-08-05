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

static void ApplyMarineRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);
static void ApplyUHFCBRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);
static  void ApplyGMRSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);
static  void ApplyFRSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);
static void 	ApplyMURSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);
static void 	ApplyNOAARestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);
static void 	ApplyPMR446Restrictions(uint16_t index, struct_codeplugChannel_t *channelBuf);

const struct_AutoZoneParams_t AutoZoneData[AutoZone_TYPE_MAX]=
{// type, name, flags, start freq, end freq, spacing, rpt offset, pri index, totalInBase, baseChn, interChn, offsetChn, offInterChn, func
	{AutoZone_MRN, "MRN", 0x03a5, 16065000, 16200000, 5000, 4600, 16, 56, 1, 60, 0, 0, &ApplyMarineRestrictions},
		{AutoZone_AU_UHFCB, "UHF CB", 0x55, 47642500, 47740000, 2500, 750, 5, 80, 0, 0, 0, 0, &ApplyUHFCBRestrictions},
			{AutoZone_GMRS, "GMRS", 0x04d5, 46255000, 46272500, 2500, 5000, 0, 15, 15, 1, 23, 8, &ApplyGMRSRestrictions},
				{AutoZone_FRS, "FRS",  0x04d5, 46255000, 46272500, 2500, 5000, 0, 15, 15, 1, 23, 8, &ApplyFRSRestrictions},
					{AutoZone_MURS, "MURS", 0x01, 15182000, 15206000, 6000, 0, 0, 5, 0, 0, 0, 0, &ApplyMURSRestrictions},
						{AutoZone_NOAA, "NOAA", 0x01, 16240000, 16255000, 2500, 0, 0, 7, 0, 0, 0, 0, &ApplyNOAARestrictions},
							{AutoZone_PMR446, "PMR446", 0x41,  44600625, 44619375, 1250, 0, 0, 16, 0, 0, 0, 0, &ApplyPMR446Restrictions}
};

static struct_AutoZoneParams_t* autoZone=&nonVolatileSettings.autoZone;

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
	return zoneIndex < -1; // allchannels is -1, AutoZones are -2, -3, etc.
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

static  void ApplyGMRSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	// duplex is not allowed on interstitials
	if (index < 9)
		autoZone->flags|=AutoZoneDuplexAvailable;
	else
		autoZone->flags&=~AutoZoneDuplexAvailable;
	if (index >= 24)
		channelBuf->flag4&=~0x02; // narrow.
	else
		channelBuf->flag4|=0x02; // bits... 0x80 = Power, 0x40 = Vox, 0x20 = ZoneSkip (AutoScan), 0x10 = AllSkip (LoneWoker), 0x08 = AllowTalkaround, 0x04 = OnlyRx, 0x02 = Channel width, 0x01 = Squelch
	if (index >=24)
		channelBuf->libreDMR_Power=3; // max 0.5 watts.
	else
		channelBuf->libreDMR_Power=0; // From Master.
}
	
static  void ApplyFRSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	// duplex is never allowed
	autoZone->flags&=~AutoZoneDuplexAvailable;
	channelBuf->flag4&=~0x02; // narrow.
	// channels 8-14 must use 0.5 w, the rest 2 w.
	if (index >=24)
		channelBuf->libreDMR_Power=3; // max 0.5 watts.
	else
		channelBuf->libreDMR_Power=6; // max 2 watts.
}
	
static void 	ApplyMURSRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	channelBuf->libreDMR_Power=6; // max 2 watts.
	if (index <=3)
		channelBuf->flag4&=~0x02; // clear.
	else
		channelBuf->flag4|=0x02; // bits... 0x80 = Power, 0x40 = Vox, 0x20 = ZoneSkip (AutoScan), 0x10 = AllSkip (LoneWoker), 0x08 = AllowTalkaround, 0x04 = OnlyRx, 0x02 = Channel width, 0x01 = Squelch
}

static void AdjustMURSFrequencies(uint16_t index, uint32_t* rxFreq, uint32_t* txFreq)
{
	if (index==4)
	{
		*rxFreq=15457000;
		*txFreq=*rxFreq;
		return;
	}		
				
	if (index==5)
	{
		*rxFreq=15460000;
		*txFreq=*rxFreq;
		return;
	}		
}

static void 	ApplyNOAARestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	channelBuf->flag4|=0x04; // RX only.
}

static void 	ApplyPMR446Restrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	channelBuf->libreDMR_Power=3; // max 0.5 watts.
}

/*
Apply specific hacks, e.g. channels 22, 23, 61-63 in UHF CB in Australian band are RX only.
*/
 void AutoZoneApplyChannelRestrictions(uint16_t index, struct_codeplugChannel_t *channelBuf)
{
	if (autoZone->type <1 || autoZone->type >=AutoZone_TYPE_MAX)
		return;
	if (!AutoZoneData[autoZone->type].ApplyChannelRestrictionsFunc)
		return;
	AutoZoneData[autoZone->type-1].ApplyChannelRestrictionsFunc(index, channelBuf);
}	

void AutoZoneInitialize(AutoZoneType_t type)
{
	if (type >=AutoZone_TYPE_MAX)
		return;
	memcpy(autoZone, &AutoZoneData[type-1], sizeof(struct_AutoZoneParams_t));
}

static uint16_t GetDisplayChannelNumber(uint16_t index)
{
		bool channelIsInBankAtOffset=false;
		if ((autoZone->flags&AutoZoneHasBankAtOffset) && (index > autoZone->totalChannelsInBaseBank) && (index <=(autoZone->totalChannelsInBaseBank*2)))
	{
		index-=autoZone->totalChannelsInBaseBank;
		channelIsInBankAtOffset=true;
	}

	uint16_t channelNumber=index;
	
	if (autoZone->flags &AutoZoneHasBaseIndex)
	{
		uint16_t totalChannelsRoundedUpToEven=autoZone->totalChannelsInBaseBank;
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

static bool 		GetGMRSZoneChannelIndices(struct_codeplugZone_t *returnBuf, bool restrictToFRS)
{
	// physical order is 15-22, 1-7, 23-30, 8-14.
	// remember zone is 0-based, but channels are 1-based
	for (uint16_t channelIndex=1; channelIndex <=7; ++channelIndex)
	{
		returnBuf->channels[channelIndex-1] = channelIndex+8; // interstitial channels in positions 9 to 15 move down to 1 to 7.
	}	
		for (uint16_t channelIndex=8; channelIndex <=14; ++channelIndex)
	{
		returnBuf->channels[channelIndex-1] = channelIndex + 16; // physical channels 24 to 30 move down to 8 to 14.
	}	
	for (uint16_t channelIndex=15; channelIndex <=22; ++channelIndex)
	{
		returnBuf->channels[channelIndex-1] = channelIndex -14; // channels 1 to 8 move to 15 to 22.
	}	
	if (restrictToFRS)
		returnBuf->NOT_IN_CODEPLUGDATA_numChannelsInZone=22;
	else
	{
		for (uint16_t channelIndex=23; channelIndex <= 30; ++channelIndex)
		{
			returnBuf->channels[channelIndex-1] = channelIndex-7; // channels 16 to 23 move to 23 to 30.
		}
		returnBuf->NOT_IN_CODEPLUGDATA_numChannelsInZone=30;
	}
	
	return true;
}

static bool 	AutoZoneGetChannelIndices(struct_codeplugZone_t *returnBuf)
{
	switch (autoZone->type)
	{
		case AutoZone_GMRS:
		case AutoZone_FRS:
			return GetGMRSZoneChannelIndices(returnBuf, autoZone->type==AutoZone_FRS);
			break;
		default:
		break;
	}
	return false;
}

// Given an index, we count bits which are enabled until the bit count equals the index.
// For example, if autozones 1 and 7 are enabled, bits 0 and 6 will be set.
// If the index is for the 2nd enabled autoZone, then we'll find bit 6 i.e. return decimal 32.
static uint8_t AutoZoneGetBitFromIndex(int index)
{
	uint8_t indexCounter=0;
	uint8_t bit=1;
	for (uint8_t bitCounter = 1 ; bitCounter < 8; ++bitCounter)
	{
		if (nonVolatileSettings.autoZonesEnabled&bit)
		{
			indexCounter++;
			if (indexCounter==index)
				return bit;
		}
		bit <<=1;
	}
	
	return 0;
}

// Each bit of nonVolatileSettings.autoZonesEnabled from right to left corresponds to one of the autoZones types.
// So if marine and AU UHF CB are enabled, then bits 0 and 1 will be set.
// This returns the type identifier for the bit passed in.
// Assuming a single bit is set.
static uint8_t AutoZoneGetTypeFromBit(uint8_t bit)
{
	return __builtin_ffs(bit); 
}

bool AutoZoneGetZoneDataForIndex(int zoneNum, struct_codeplugZone_t *returnBuf)
{
	if (zoneNum < codeplugZonesGetRealCount() || zoneNum == (codeplugZonesGetCount() - 1))
		return false;
	uint8_t nthAutoZone=zoneNum-codeplugZonesGetRealCount()+1;
	// look in the nonVolatileSettings.autoZonesEnabled flags and see which autoZone this refers to counting bits from right to left.
	uint8_t autoZoneEnabledBit=AutoZoneGetBitFromIndex( nthAutoZone);
	uint8_t type=AutoZoneGetTypeFromBit(autoZoneEnabledBit);
	if (type < 1 || type >=AutoZone_TYPE_MAX)
		return false;
	if ((type!=autoZone->type) || !AutoZoneIsValid())
		AutoZoneInitialize(type);
	// This is the autogenerated zone.
	int nameLen = SAFE_MIN(((int)sizeof(returnBuf->name)), ((int)strlen(autoZone->name)));
	// Codeplug name is 0xff filled, codeplugUtilConvertBufToString() handles the conversion
	memset(returnBuf->name, 0xff, sizeof(returnBuf->name));
	memcpy(returnBuf->name, autoZone->name, nameLen);
	
		int total=autoZone->totalChannelsInBaseBank;
	if (autoZone->flags&AutoZoneHasBankAtOffset)
		total*=2;
	returnBuf->NOT_IN_CODEPLUGDATA_numChannelsInZone = total;

	if (!AutoZoneGetChannelIndices(returnBuf))
	{// Just use natural order.
		for (uint16_t channelIndex=0; channelIndex < total; ++channelIndex)
		{
			returnBuf->channels[channelIndex]=channelIndex+1;
		}
	}
	
	returnBuf->NOT_IN_CODEPLUGDATA_indexNumber = -1-type;// Set as -2 ... as this is not a real zone. Its the autogenerated zone.
	
	return true;
}

bool AutoZoneGetFrequenciesForIndex(uint16_t index, uint32_t* rxFreq, uint32_t* txFreq)
{
	if (!rxFreq || !txFreq)
		return false;
	if (!AutoZoneIsValid())
		return false;
	
	bool channelIsInBankAtOffset=false;
	if ((autoZone->flags&AutoZoneHasBankAtOffset) && (index > autoZone->totalChannelsInBaseBank) && (index <= (autoZone->totalChannelsInBaseBank*2)))
	{
		index-=autoZone->totalChannelsInBaseBank;
		channelIsInBankAtOffset=true;
	}
	if (index > autoZone->totalChannelsInBaseBank)
		return false;
	
	uint16_t multiplier=(index-1);
	// If interleaved, first half of channels start at startFrequency and the channel step is defined by autoZone->channelSpacing kHz.
	// Second half are offset by autoZone->channelSpacing/2 kHz between the first half of the channels
	// and may start prior.
	uint16_t interleaveOffset=0;
	uint16_t totalChannelsRoundedUpToEven=autoZone->totalChannelsInBaseBank;
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
	// Hack for MURS.
	if (autoZone->type==AutoZone_MURS)
		AdjustMURSFrequencies(index, rxFreq, txFreq);
	return true;
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
	channelBuf->rxTone=CODEPLUG_CSS_TONE_NONE;
	channelBuf->txTone=CODEPLUG_CSS_TONE_NONE;

	if (autoZone->flags & AutoZoneNarrow)
		channelBuf->flag4&=~0x02; // clear.
	else
		channelBuf->flag4|=0x02; // bits... 0x80 = Power, 0x40 = Vox, 0x20 = ZoneSkip (AutoScan), 0x10 = AllSkip (LoneWoker), 0x08 = AllowTalkaround, 0x04 = OnlyRx, 0x02 = Channel width, 0x01 = Squelch
	AutoZoneApplyChannelRestrictions(index, channelBuf);
	return true;
}

bool AutoZoneGetData(AutoZoneType_t type, struct_AutoZoneParams_t* autoZone)
{
	if (type <=AutoZone_NONE || type >=AutoZone_TYPE_MAX)
		return false;
	memcpy(autoZone, &AutoZoneData[type-1], sizeof(struct_AutoZoneParams_t));
	
	return true;
}

uint8_t AutoZoneGetEnabledCount()
{
	return __builtin_popcount(nonVolatileSettings.autoZonesEnabled);
}