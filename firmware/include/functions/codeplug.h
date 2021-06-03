/*
 * Copyright (C) 2019      Kai Ludwig, DG4KLU
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
#ifndef _OPENGD77_CODEPLUG_H_
#define _OPENGD77_CODEPLUG_H_

#include <stdint.h>
#include <stdbool.h>

extern const int CODEPLUG_ADDR_CHANNEL_HEADER_EEPROM;

extern const int CODEPLUG_MAX_VARIABLE_SQUELCH;
extern const int CODEPLUG_MIN_VARIABLE_SQUELCH;
extern const int CODEPLUG_MIN_PER_CHANNEL_POWER;

extern const int VFO_FREQ_STEP_TABLE[8];

#define CODEPLUG_ADDR_QUICKKEYS                                0x7524 // LSB,HSB
#define CODEPLUG_QUICKKEYS_SIZE                                    10



// Zone
#define CODEPLUG_ZONE_DATA_ORIGINAL_STRUCT_SIZE                    48 // 16 channels per zone
#define CODEPLUG_ZONE_DATA_OPENGD77_STRUCT_SIZE                   176 // 80 channels per zone
#define ZONE_DATA_STRUCT_SIZE                                     184

// Channel
#define CODEPLUG_CHANNEL_DATA_STRUCT_SIZE                          56
#define CHANNEL_DATA_STRUCT_SIZE                                   57
#define CODEPLUG_CHANNEL_FLAG4_OFFSET                              51

// RXGroup
#define CODEPLUG_RXGROUP_DATA_STRUCT_SIZE                          80

// Contact
#define CODEPLUG_CONTACT_DATA_SIZE                                 24
#define CONTACT_DATA_STRUCT_SIZE                                   28

// DTMF Contact
#define CODEPLUG_DTMF_CONTACT_DATA_STRUCT_SIZE                     32

// General Settings
#define CODEPLUG_GENERAL_SETTINGS_DATA_STRUCT_SIZE                 40

// DTMF Signalling
#define CODEPLUG_SIGNALLING_DTMF_DATA_STRUCT_SIZE                 120
#define SIGNALLING_DTMF_DURATIONS_DATA_STRUCT_SIZE                  5

#define CODEPLUG_CONTACTS_MIN                                       1
#define CODEPLUG_CONTACTS_MAX                                    1024

#define CODEPLUG_RX_GROUPLIST_MAX                                  76

#define CODEPLUG_CHANNELS_MIN                                       1
#define CODEPLUG_CHANNELS_MAX                                    1024
#define CODEPLUG_CHANNELS_BANKS_MAX                                 8
#define CODEPLUG_CHANNELS_PER_BANK                                128

#define CODEPLUG_DTMF_CONTACTS_MIN                                  1
#define CODEPLUG_DTMF_CONTACTS_MAX                                 32

#define CODEPLUG_CSS_TONE_NONE                                 0xFFFF
typedef enum
{
	CSS_TYPE_NONE          = (1 << 0),
	CSS_TYPE_CTCSS         = (1 << 1),
	CSS_TYPE_DCS           = (1 << 15),
	CSS_TYPE_DCS_INVERTED  = (1 << 14),
	CSS_TYPE_DCS_MASK      = (CSS_TYPE_DCS | CSS_TYPE_DCS_INVERTED)
} CodeplugCSSTypes_t;


#define CODEPLUG_CHANNEL_FLAG_ALL_SKIP                           0x10
#define CODEPLUG_CHANNEL_FLAG_ZONE_SKIP                          0x20
#define CODEPLUG_CHANNEL_IS_FLAG_SET(c, f)                      ((((c)->flag4 & (f)) != 0x0))

extern int codeplugChannelsPerZone;


enum CONTACT_CALLTYPE_SELECT { CONTACT_CALLTYPE_TG = 0, CONTACT_CALLTYPE_PC, CONTACT_CALLTYPE_ALL };

typedef enum
{
     CHANNEL_VFO_A = 0,
     CHANNEL_VFO_B,
     CHANNEL_CHANNEL
} Channel_t;

// AllChannels zone indexNumber is -1
#define CODEPLUG_ZONE_IS_ALLCHANNELS(z) ((z).NOT_IN_CODEPLUGDATA_indexNumber < 0)


typedef struct
{
	char		name[16];
	uint16_t	channels[80];// 16 for the original codeplug, but set this to  80 to allow for the new codeplug zones format
	int			NOT_IN_CODEPLUGDATA_numChannelsInZone;// This property is not part of the codeplug data, its initialised by the code
	int			NOT_IN_CODEPLUGDATA_highestIndex; // Highest index in the array of channels
	int			NOT_IN_CODEPLUGDATA_indexNumber;// This property is not part of the codeplug data, its initialised by the code. Index > 0 are real zone. -1 indicates the virtual "All channels" zone
} struct_codeplugZone_t;

typedef struct
{
	char name[16];
	uint32_t rxFreq;
	uint32_t txFreq;
	uint8_t chMode;
	uint8_t libreDMR_Power;
	uint8_t txRefFreq;
	uint8_t tot;
	uint8_t totRekey;
	uint8_t admitCriteria;
	uint8_t rssiThreshold;
	uint8_t scanList;
	uint16_t rxTone;
	uint16_t txTone;
	uint8_t voiceEmphasis;
	uint8_t txSignaling;
	uint8_t LibreDMR_flag1; // was unmuteRule. 0x80: Optional DMRID sets.
	uint8_t rxSignaling;    // +--
	uint8_t artsInterval;   // | These 3 bytes were repurposed for optional DMRID
	uint8_t encrypt;        // +--
	uint8_t rxColor;
	uint8_t rxGroupList;
	uint8_t txColor;
	uint8_t emgSystem;
	uint16_t contact;
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;// bits... 0x20 = DisableAllLeds
	uint8_t flag4;// bits... 0x80 = Power, 0x40 = Vox, 0x20 = ZoneSkip (AutoScan), 0x10 = AllSkip (LoneWoker), 0x08 = AllowTalkaround, 0x04 = OnlyRx, 0x02 = Channel width, 0x01 = Squelch
	uint16_t VFOoffsetFreq;
	uint8_t VFOflag5;// upper 4 bits are the step frequency 2.5,5,6.25,10,12.5,25,30,50kHz
	uint8_t sql;// Does not seem to be used in the official firmware and seems to be always set to 0
	uint8_t NOT_IN_CODEPLUG_flag; // bit 0x01 = vfo channel
} struct_codeplugChannel_t;

typedef struct
{
	char name[16];
	uint16_t contacts[32];
	int	NOT_IN_CODEPLUG_numTGsInGroup;// NOT IN THE
	uint32_t NOT_IN_CODEPLUG_contactsTG[32];
} struct_codeplugRxGroup_t;

typedef struct
{
	char 		name[16];
	uint32_t 	tgNumber;
	uint8_t		callType;
	uint8_t		callRxTone;
	uint8_t		ringStyle;
	uint8_t		reserve1; // TS override: bit0 1 = no TS override, bit1 + 1: timeslot override value
	int         NOT_IN_CODEPLUGDATA_indexNumber;
} struct_codeplugContact_t;

typedef struct
{
	char       name[16];
	uint8_t    code[16];
} struct_codeplugDTMFContact_t;

typedef struct
{
	char 		radioName[8];// callsign
	uint32_t	radioId;// DMD Id
	uint8_t		libreDMRCodeplugVersion;
	uint8_t		reserve[3];// was 4 bytes of reserved data but we use the first byte as the codeplug version
	uint8_t		arsInitDly;
	uint8_t		txPreambleDur;
	uint8_t		monitorType;
	uint8_t		voxSense;
	uint8_t		rxLowBatt;
	uint8_t		callAlertDur;
	uint8_t		respTmr;
	uint8_t		reminderTmr;
	uint8_t		grpHang;
	uint8_t		privateHang;
	uint8_t		flag1;
	uint8_t		flag2;
	uint8_t		flag3;
	uint8_t		flag4;
	uint8_t		reserver2[2];
	char 		prgPwd[8];
} struct_codeplugGeneralSettings_t;

typedef struct
{
	 char		selfId[8];
	 char		killCode[16];
	 char		wakeCode[16];
	 uint8_t	delimiter;
	 uint8_t	groupCode;
	 uint8_t	decodeResp;
	 uint8_t	autoResetTimer;
	 uint8_t	flag1;
	 uint8_t	reserve1[3];
	 char		pttidUpCode[30];
	 uint16_t	reserve2;
	 char		pttidDownCode[30];
	 uint16_t	reserve3;
	 uint8_t	respHoldTime;
	 uint8_t	decTime;
	 uint8_t	fstDigitDly;
	 uint8_t	fstDur;
	 uint8_t	otherDur;
	 uint8_t	rate;
	 uint8_t	flag2;
	 uint8_t	reserve4;
} struct_codeplugSignalling_DTMF_t;


typedef struct
{
	uint8_t 	fstDigitDly;	// First Digit Delay (unit: hundreds of ms)
	uint8_t 	fstDur;			// First Digit Duration (unit: hundreds of ms)
	uint8_t 	otherDur;		// * and # Duration (unit: hundreds of ms)
	uint8_t 	rate;			// DTMF Rate: 1 .. 10 digits / second
	uint8_t		libreDMR_Tail;	// TX Tail duration (unit: hundreds of ms)
} struct_codeplugSignalling_DTMFDurations_t;

typedef struct
{
	uint16_t	minUHFFreq;
	uint16_t	maxUHFFreq;
	uint16_t	minVHFFreq;
	uint16_t	maxVHFFreq;
	uint8_t		lastPrgTime[6];
	uint16_t	reserve2;
	uint8_t		model[8];
	uint8_t		sn[16];
	uint8_t		cpsSwVer[8];
	uint8_t		hardwareVer[8];
	uint8_t		firmwareVer[8];
	uint8_t		dspFwVer[24];
	uint8_t		reserve3[8];
} struct_codeplugDeviceInfo_t;


typedef enum
{
	CODEPLUG_CUSTOM_DATA_TYPE_NONE = 0,
	CODEPLUG_CUSTOM_DATA_TYPE_IMAGE,
	CODEPLUG_CUSTOM_DATA_TYPE_BEEP
} codeplugCustomDataType_t;

/*
 * deprecated. Use our own non volatile storage instead
 *
void codeplugZoneGetSelected(int *selectedZone,int *selectedChannel);
void codeplugZoneSetSelected(int selectedZone,int selectedChannel);
 */
int codeplugZonesGetCount(void);
bool codeplugZoneGetDataForNumber(int indexNum,struct_codeplugZone_t *returnBuf);
uint32_t codeplugChannelGetOptionalDMRID(struct_codeplugChannel_t *channelBuf);
void codeplugChannelSetOptionalDMRID(uint32_t dmrID, struct_codeplugChannel_t *channelBuf);
void codeplugChannelGetDataWithOffsetAndLengthForIndex(int index, struct_codeplugChannel_t *channelBuf, uint8_t offset, int length);
void codeplugChannelGetDataForIndex(int index, struct_codeplugChannel_t *channelBuf);
void codeplugUtilConvertBufToString(char *codeplugBuf,char *outBuf,int len);
void codeplugUtilConvertStringToBuf(char *inBuf,char *outBuf,int len);
uint32_t byteSwap32(uint32_t n);
uint32_t bcd2int(uint32_t i);
uint32_t int2bcd(uint32_t i);
uint16_t codeplugCSSToInt(uint16_t css);
uint16_t codeplugIntToCSS(uint16_t i);

bool codeplugRxGroupGetDataForIndex(int index, struct_codeplugRxGroup_t *rxGroupBuf);

bool codeplugContactGetDataForIndex(int index, struct_codeplugContact_t *contact);
bool codeplugDTMFContactGetDataForIndex(int index, struct_codeplugDTMFContact_t *contact);

uint32_t codeplugGetUserDMRID(void);
void codeplugSetUserDMRID(uint32_t dmrId);
void codeplugGetRadioName(char *buf);
void codeplugGetBootScreenData(char *line1, char *line2, uint8_t *displayType);
void codeplugGetVFO_ChannelData(struct_codeplugChannel_t *vfoBuf, Channel_t VFONumber);
void codeplugSetVFO_ChannelData(struct_codeplugChannel_t *vfoBuf, Channel_t VFONumber);
bool codeplugAllChannelsIndexIsInUse(int index);
void codeplugAllChannelsIndexSetUsed(int index);
bool codeplugChannelSaveDataForIndex(int index, struct_codeplugChannel_t *channelBuf);
CodeplugCSSTypes_t codeplugGetCSSType(uint16_t tone);


int codeplugDTMFContactsGetCount(void);
int codeplugContactsGetCount(uint32_t callType);
int codeplugContactGetDataForNumberInType(int number, uint32_t callType, struct_codeplugContact_t *contact);
int codeplugDTMFContactGetDataForNumber(int number, struct_codeplugDTMFContact_t *contact);
int codeplugContactIndexByTGorPC(uint32_t tgorpc, uint32_t callType, struct_codeplugContact_t *contact, uint8_t optionalTS);
int codeplugContactSaveDataForIndex(int index, struct_codeplugContact_t *contact);
int codeplugContactGetFreeIndex(void);
bool codeplugContactGetRXGroup(int index);
void codeplugInitChannelsPerZone(void);
bool codeplugGetOpenGD77CustomData(codeplugCustomDataType_t dataType, uint8_t *dataBuf);

void codeplugAllChannelsInitCache(void);
void codeplugInitCaches(void);

bool codeplugContactsContainsPC(uint32_t pc);
bool codeplugGetGeneralSettings(struct_codeplugGeneralSettings_t *generalSettingsBuffer);
bool codeplugGetSignallingDTMF(struct_codeplugSignalling_DTMF_t *signallingDTMFBuffer);
bool codeplugGetSignallingDTMFDurations(struct_codeplugSignalling_DTMFDurations_t *signallingDTMFDurationsBuffer);
bool codeplugZoneAddChannelToZoneAndSave(int channelIndex, struct_codeplugZone_t *zoneBuf);
bool codeplugGetDeviceInfo(struct_codeplugDeviceInfo_t *deviceInfoBuffer);

uint16_t codeplugGetQuickkeyFunctionID(char key);
bool codeplugSetQuickkeyFunctionID(char key, uint16_t functionId);

int codeplugGetRepeaterWakeAttempts(void);
int codeplugGetPasswordPin(int32_t *pinCode);
#endif
