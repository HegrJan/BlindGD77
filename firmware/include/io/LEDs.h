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

#ifndef _OPENGD77_LEDS_H_
#define _OPENGD77_LEDS_H_

#include <stdint.h>
#include <stdbool.h>
#include "functions/settings.h"

#if ! defined(PLATFORM_GD77S)
extern uint8_t LEDsState[2];
#endif

void LEDsInit(void);

#if defined(PLATFORM_RD5R)
void torchToggle(void);
#endif


static inline void LEDs_PinWrite(GPIO_Type *base, uint32_t pin, uint8_t output)
{
#if ! defined(PLATFORM_GD77S)
	LEDsState[(base == GPIO_LEDred) ? 0 : 1] = output;

	if ((nonVolatileSettings.bitfieldOptions & BIT_ALL_LEDS_DISABLED) == 0)
#endif
	{
		GPIO_PinWrite(base, pin, output);
	}
}

static inline uint32_t LEDs_PinRead(GPIO_Type *base, uint32_t pin)
{
#if ! defined(PLATFORM_GD77S)
	if (nonVolatileSettings.bitfieldOptions & BIT_ALL_LEDS_DISABLED)
	{
		return LEDsState[(base == GPIO_LEDred) ? 0 : 1];
	}
#endif
	return GPIO_PinRead(base, pin);
}



#endif /* _OPENGD77_LEDS_H_ */
