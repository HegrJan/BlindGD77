/*
 * Copyright (C) 2019      Kai Ludwig, DG4KLU
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *                         Colin, G4EML
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

#ifndef _OPENGD77_TRX_H_
#define _OPENGD77_TRX_H_

#include "functions/sound.h"
#include "interfaces/i2c.h"
#include "functions/calibration.h"
#include "functions/codeplug.h"

typedef struct
{
	int calTableMinFreq;
	int minFreq;
	int maxFreq;
} frequencyHardwareBand_t;

typedef struct
{
	int minFreq;
	int maxFreq;
} frequencyBand_t;

typedef struct trxFrequency
{
	int rxFreq;
	int txFreq;
} trxFrequency_t;

enum RADIO_MODE { RADIO_MODE_NONE, RADIO_MODE_ANALOG, RADIO_MODE_DIGITAL };
enum DMR_ADMIT_CRITERIA { ADMIT_CRITERIA_ALWAYS, ADMIT_CRITERIA_CHANNEL_FREE, ADMIT_CRITERIA_COLOR_CODE };
enum DMR_MODE { DMR_MODE_AUTO, DMR_MODE_DMO, DMR_MODE_RMO, DMR_MODE_SFR };
enum RADIO_FREQUENCY_BAND_NAMES { RADIO_BAND_VHF = 0, RADIO_BAND_220MHz, RADIO_BAND_UHF, RADIO_BANDS_TOTAL_NUM };
enum TRX_FREQ_BAND { TRX_RX_FREQ_BAND = 0, TRX_TX_FREQ_BAND };

extern const frequencyHardwareBand_t 	RADIO_HARDWARE_FREQUENCY_BANDS[RADIO_BANDS_TOTAL_NUM];
extern const frequencyBand_t			DEFAULT_USER_FREQUENCY_BANDS[RADIO_BANDS_TOTAL_NUM];
extern frequencyBand_t					USER_FREQUENCY_BANDS[RADIO_BANDS_TOTAL_NUM];

extern const uint8_t TRX_NUM_CTCSS;
extern const uint16_t TRX_CTCSSTones[];

extern const uint16_t TRX_DCS_TONE;
extern const uint8_t TRX_NUM_DCS;
extern const uint16_t TRX_DCSCodes[];

extern int trxDMRModeRx;
extern int trxDMRModeTx;

extern volatile bool trxTransmissionEnabled;
extern volatile bool trxIsTransmitting;
extern uint32_t trxTalkGroupOrPcId;
extern uint32_t trxDMRID;
extern volatile uint8_t trxRxSignal;
extern volatile uint8_t trxRxNoise;
extern volatile uint8_t trxTxVox;
extern volatile uint8_t trxTxMic;
extern calibrationPowerValues_t trxPowerSettings;
extern int trxCurrentBand[2];
extern volatile bool txPAEnabled;
extern volatile bool trxDMRSynchronisedRSSIReadPending;

void I2C_AT1846_set_register_with_mask(uint8_t reg, uint16_t mask, uint16_t value, uint8_t shift);

bool trxCarrierDetected(void);
bool trxCheckDigitalSquelch(void);
bool trxCheckAnalogSquelch(void);
void trxResetSquelchesState(void);
int trxGetMode(void);
bool trxGetBandwidthIs25kHz(void);
int trxGetFrequency(void);
void trxSetModeAndBandwidth(int mode, bool bandwidthIs25kHz);
void trxSetFrequency(int fRx,int fTx, int dmrMode);
void trxSetRX(void);
void trxSetTX(void);
void trxRxAndTxOff(bool critical);
void trxRxOn(bool critical);
void trxActivateRx(bool critical);
void trxActivateTx(bool critical);
void trxSetPowerFromLevel(int powerLevel);
void trxUpdate_PA_DAC_Drive(void);
uint16_t trxGetPA_DAC_Drive(void);
int trxGetPowerLevel(void);
void trxCalcBandAndFrequencyOffset(CalibrationBand_t *calibrationBand, uint32_t *freq_offset);
void trxSetDMRColourCode(int colourCode);
int trxGetDMRColourCode(void);
int trxGetDMRTimeSlot(void);
void trxSetDMRTimeSlot(int timeslot, bool resync);
void trxSetTxCSS(uint16_t tone);
void trxSetRxCSS(uint16_t tone);
bool trxCheckCSSFlag(uint16_t tone);
bool trxCheckFrequencyInAmateurBand(int tmp_frequency);
int trxGetBandFromFrequency(int frequency);
int trxGetNextOrPrevBandFromFrequency(int frequency, bool nextBand);
void trxReadVoxAndMicStrength(void);
void trxPostponeReadRSSIAndNoise(uint32_t msOverride);
void trxReadRSSIAndNoise(bool force);
uint8_t trxGetCalibrationVoiceGainTx(void);
void trxSelectVoiceChannel(uint8_t channel);
void trxSetTone1(int toneFreq);
void trxSetTone2(int toneFreq);
void trxSetDTMF(int code);
void trxUpdateTsForCurrentChannelWithSpecifiedContact(struct_codeplugContact_t *contactData);
void trxSetMicGainFM(uint8_t gain);

void trxEnableTransmission(void);
void trxDisableTransmission(void);
bool trxPowerUpDownRxAndC6000(bool powerUp, bool includeC6000);
uint8_t trxGetAnalogFilterLevel();
void trxSetAnalogFilterLevel(uint8_t newFilterLevel);
void trxTerminateCheckAnalogSquelch(void);

#endif /* _OPENGD77_TRX_H_ */
