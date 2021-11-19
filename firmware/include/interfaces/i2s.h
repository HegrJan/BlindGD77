/*
 * Copyright (C) 2019      Kai Ludwig, DG4KLU
 * Copyright (C) 2019-2020 Roger Clark, VK3KYY / G4KYF
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

#ifndef _OPENGD77_I2S_H_
#define _OPENGD77_I2S_H_

#include "fsl_port.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"

#include "functions/sound.h"

#define NUM_I2S_BUFFERS 6


#if defined(PLATFORM_GD77) || defined(PLATFORM_GD77S)

// I2S to C6000 (I2S)
// OUT/ON  A16 - I2S FS to C6000
// OUT/OFF A14 - I2S CK to C6000
// OUT/ON  A12 - I2S RX to C6000
// IN      A15 - I2S TX to C6000
#define Port_I2S_FS_C6000    PORTA
#define GPIO_I2S_FS_C6000    GPIOA
#define Pin_I2S_FS_C6000     16
#define Port_I2S_CK_C6000    PORTA
#define GPIO_I2S_CK_C6000    GPIOA
#define Pin_I2S_CK_C6000     14
#define Port_I2S_RX_C6000    PORTA
#define GPIO_I2S_RX_C6000    GPIOA
#define Pin_I2S_RX_C6000     12
#define Port_I2S_TX_C6000    PORTA
#define GPIO_I2S_TX_C6000    GPIOA
#define Pin_I2S_TX_C6000     15

#elif defined(PLATFORM_DM1801) || defined(PLATFORM_DM1801A)

// I2S to C6000 (I2S)
// OUT/ON  A16 - I2S FS to C6000
// OUT/OFF A14 - I2S CK to C6000
// OUT/ON  A12 - I2S RX to C6000
// IN      A15 - I2S TX to C6000
#define Port_I2S_FS_C6000    PORTA
#define GPIO_I2S_FS_C6000    GPIOA
#define Pin_I2S_FS_C6000     16
#define Port_I2S_CK_C6000    PORTA
#define GPIO_I2S_CK_C6000    GPIOA
#define Pin_I2S_CK_C6000     14
#define Port_I2S_RX_C6000    PORTA
#define GPIO_I2S_RX_C6000    GPIOA
#define Pin_I2S_RX_C6000     12
#define Port_I2S_TX_C6000    PORTA
#define GPIO_I2S_TX_C6000    GPIOA
#define Pin_I2S_TX_C6000     15

#elif defined(PLATFORM_RD5R)

// I2S to C6000 (I2S)
// OUT/ON  A16 - I2S FS to C6000
// OUT/OFF A14 - I2S CK to C6000
// OUT/ON  A12 - I2S RX to C6000
// IN      A15 - I2S TX to C6000
#define Port_I2S_FS_C6000    PORTA
#define GPIO_I2S_FS_C6000    GPIOA
#define Pin_I2S_FS_C6000     16
#define Port_I2S_CK_C6000    PORTA
#define GPIO_I2S_CK_C6000    GPIOA
#define Pin_I2S_CK_C6000     14
#define Port_I2S_RX_C6000    PORTA
#define GPIO_I2S_RX_C6000    GPIOA
#define Pin_I2S_RX_C6000     12
#define Port_I2S_TX_C6000    PORTA
#define GPIO_I2S_TX_C6000    GPIOA
#define Pin_I2S_TX_C6000     15

#endif


extern volatile bool g_TX_SAI_in_use;
extern sai_edma_handle_t g_SAI_TX_Handle;
extern sai_edma_handle_t g_SAI_RX_Handle;

void init_I2S(void);
void setup_I2S(void);
void I2SReset(void);
void I2STerminateTransfers(void);
void I2STransferReceive(uint8_t *buff,size_t bufferLen);
bool I2STransferTransmit(uint8_t *buff,size_t bufferLen);

#endif /* _OPENGD77_I2S_H_ */
