/*
 * Copyright (C) 2019      Kai Ludwig, DG4KLU
 * Copyright (C) 2020-2021 Roger Clark, VK3KYY / G4KYF
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

#include "io/buttons.h"
#include "interfaces/pit.h"
#include "functions/settings.h"
#include "usb/usb_com.h"
#include "interfaces/gpio.h"

static uint32_t prevButtonState;
static uint32_t mbuttons;
volatile bool   PTTLocked = false;

#define MBUTTON_PRESSED        (1 << 0)
#define MBUTTON_LONG           (1 << 1)
#define MBUTTON_EXTRA_LONG     (1 << 2)

typedef enum
{
	MBUTTON_ORANGE,
	MBUTTON_SK1,
	MBUTTON_SK2,
	MBUTTON_MAX
} MBUTTON_t;

void buttonsInit(void)
{
	gpioInitButtons();
	mbuttons = BUTTON_NONE;
	prevButtonState = BUTTON_NONE;
}

static bool isMButtonPressed(MBUTTON_t mbutton)
{
     return (((mbuttons >> (mbutton * 3)) & MBUTTON_PRESSED) & MBUTTON_PRESSED);
}

static bool isMButtonLong(MBUTTON_t mbutton)
{
     return (((mbuttons >> (mbutton * 3)) & MBUTTON_LONG) & MBUTTON_LONG);
}

static bool isMButtonExtraLong(MBUTTON_t mbutton)
{
     return (((mbuttons >> (mbutton * 3)) & MBUTTON_EXTRA_LONG) & MBUTTON_EXTRA_LONG);
}

static void setMButtonsStateAndClearLong(uint32_t *buttons, MBUTTON_t mbutton, uint32_t buttonID)
{
	if (*buttons & buttonID)
	{
		mbuttons |= (MBUTTON_PRESSED << (mbutton * 3));
	}
	else
	{
		mbuttons &= ~(MBUTTON_PRESSED << (mbutton * 3));
	}

	taskENTER_CRITICAL();
	switch (mbutton)
	{
		case MBUTTON_SK1:
		case MBUTTON_SK2:
		case MBUTTON_ORANGE:
			timer_mbuttons[mbutton] = (*buttons & buttonID) ? (nonVolatileSettings.keypadTimerLong * 1000) : 0;
			break;

		default:
			break;
	}
	taskEXIT_CRITICAL();

	mbuttons &= ~(MBUTTON_LONG << (mbutton * 3));
	mbuttons &= ~(MBUTTON_EXTRA_LONG << (mbutton * 3));
}

static void checkMButtonState(uint32_t *buttons, MBUTTON_t mbutton, uint32_t buttonID)
{
	if (isMButtonPressed(mbutton) == false)
	{
		setMButtonsStateAndClearLong(buttons, mbutton, buttonID);
	}
}

uint32_t buttonsRead(void)
{
	uint32_t result = BUTTON_NONE;

#if ! defined(PLATFORM_RD5R)
	if (GPIO_PinRead(GPIO_Orange, Pin_Orange) == 0)
	{
		result |= BUTTON_ORANGE;
		checkMButtonState(&result, MBUTTON_ORANGE, BUTTON_ORANGE);
	}
#endif // ! PLATFORM_RD5R

	if (GPIO_PinRead(GPIO_PTT, Pin_PTT) == 0)
	{
		result |= BUTTON_PTT;
	}

	if (GPIO_PinRead(GPIO_SK1, Pin_SK1) == 0)
	{
		result |= BUTTON_SK1;
		checkMButtonState(&result, MBUTTON_SK1, BUTTON_SK1);
	}

	if (GPIO_PinRead(GPIO_SK2, Pin_SK2) == 0)
	{
		result |= BUTTON_SK2;
		checkMButtonState(&result, MBUTTON_SK2, BUTTON_SK2);
	}

	return result;
}

static void checkMButtons(uint32_t *buttons, MBUTTON_t mbutton, uint32_t buttonID, uint32_t buttonShortUp, uint32_t buttonLong, uint32_t buttonExtraLong)
{
	taskENTER_CRITICAL();
	uint32_t tmp_timer_mbutton = timer_mbuttons[mbutton];
	taskEXIT_CRITICAL();

	// Note: Short press are send async

	if ((*buttons & buttonID) && isMButtonPressed(mbutton) && isMButtonLong(mbutton) && (isMButtonExtraLong(mbutton) == false))
	{
		// button is still down
		*buttons |= buttonLong;

		if (tmp_timer_mbutton == 0)
		{
			// Long extra long press
			mbuttons |= (MBUTTON_EXTRA_LONG << (mbutton * 3));

			// Clear LONG and set EXTRA_LONG bits
			*buttons &= ~buttonLong;
			*buttons |= buttonExtraLong;
		}
	}
	else if ((*buttons & buttonID) && isMButtonPressed(mbutton) && isMButtonLong(mbutton) && isMButtonExtraLong(mbutton))
	{
		// button is still down
		*buttons |= buttonLong;
		// Clear LONG and set EXTRA_LONG bits
		*buttons &= ~buttonLong;
		*buttons |= buttonExtraLong;
	}
	else if ((*buttons & buttonID) && isMButtonPressed(mbutton) && (isMButtonLong(mbutton) == false))
	{
		if (tmp_timer_mbutton == 0)
		{
			// Long press
			mbuttons |= (MBUTTON_LONG << (mbutton * 3));

			// Set LONG bit
			*buttons |= buttonLong;

			// Reset the timer for extra long down usage
			taskENTER_CRITICAL();
			timer_mbuttons[mbutton] = (nonVolatileSettings.keypadTimerLong * 1000);
			taskEXIT_CRITICAL();
		}
	}
	else if (((*buttons & buttonID) == 0) && isMButtonPressed(mbutton) && (isMButtonLong(mbutton) == false) && (tmp_timer_mbutton != 0))
	{
		// Short press/release cycle
		mbuttons &= ~(MBUTTON_PRESSED << (mbutton * 3));
		mbuttons &= ~(MBUTTON_LONG << (mbutton * 3));
		mbuttons &= ~(MBUTTON_EXTRA_LONG << (mbutton * 3));

		taskENTER_CRITICAL();
		timer_mbuttons[mbutton] = 0;
		taskEXIT_CRITICAL();

		// Set SHORT press
		*buttons |= buttonShortUp;
		*buttons &= ~buttonLong;
		*buttons &= ~buttonExtraLong;
	}
	else if (((*buttons & buttonID) == 0) && isMButtonPressed(mbutton) && isMButtonLong(mbutton))
	{
		// Button was still down after a long press, now handle release
		mbuttons &= ~(MBUTTON_PRESSED << (mbutton * 3));
		mbuttons &= ~(MBUTTON_LONG << (mbutton * 3));
		mbuttons &= ~(MBUTTON_EXTRA_LONG << (mbutton * 3));

		// Remove LONG and EXTRA_LONG
		*buttons &= ~buttonLong;
		*buttons &= ~buttonExtraLong;
	}
}

void buttonsCheckButtonsEvent(uint32_t *buttons, int *event, bool keyIsDown)
{
	*buttons = buttonsRead();

	if ((*buttons != BUTTON_NONE) || (mbuttons & BUTTON_WAIT_NEW_STATE))
	{
		// A key is down, just leave DOWN bit
		if (keyIsDown)
		{
			mbuttons |= BUTTON_WAIT_NEW_STATE;

			// Clear stored states
			setMButtonsStateAndClearLong(buttons, MBUTTON_SK1, BUTTON_SK1);
			setMButtonsStateAndClearLong(buttons, MBUTTON_SK2, BUTTON_SK2);
#if ! defined(PLATFORM_RD5R)
			setMButtonsStateAndClearLong(buttons, MBUTTON_ORANGE, BUTTON_ORANGE);
#endif

			// Won't send a CHANGE event, as the key turns to be a modifier now
			prevButtonState = *buttons;
			*event = EVENT_BUTTON_NONE;
			return;
		}
		else
		{
			if (mbuttons & BUTTON_WAIT_NEW_STATE)
			{
				if (*buttons != prevButtonState)
				{
					mbuttons &= ~BUTTON_WAIT_NEW_STATE;

					// Clear stored states
					setMButtonsStateAndClearLong(buttons, MBUTTON_SK1, BUTTON_SK1);
					setMButtonsStateAndClearLong(buttons, MBUTTON_SK2, BUTTON_SK2);
#if ! defined(PLATFORM_RD5R)
					setMButtonsStateAndClearLong(buttons, MBUTTON_ORANGE, BUTTON_ORANGE);
#endif
					prevButtonState = *buttons;
					*event = EVENT_BUTTON_CHANGE;
					return;
				}

				*event = EVENT_BUTTON_NONE;
				return;
			}
		}
	}

	// Check state for every single button
#if ! defined(PLATFORM_RD5R)
	checkMButtons(buttons, MBUTTON_ORANGE, BUTTON_ORANGE, BUTTON_ORANGE_SHORT_UP, BUTTON_ORANGE_LONG_DOWN, BUTTON_ORANGE_EXTRA_LONG_DOWN);
#endif // ! PLATFORM_RD5R
	checkMButtons(buttons, MBUTTON_SK1, BUTTON_SK1, BUTTON_SK1_SHORT_UP, BUTTON_SK1_LONG_DOWN, BUTTON_SK1_EXTRA_LONG_DOWN);
	checkMButtons(buttons, MBUTTON_SK2, BUTTON_SK2, BUTTON_SK2_SHORT_UP, BUTTON_SK2_LONG_DOWN, BUTTON_SK2_EXTRA_LONG_DOWN);

	if (prevButtonState != *buttons)
	{
		prevButtonState = *buttons;
		*event = EVENT_BUTTON_CHANGE;
	}
	else
	{
		*event = EVENT_BUTTON_NONE;
	}
}
