/*
 * Copyright (C) 2019-2021 Roger Clark, VK3KYY / G4KYF
 *                         Daniel Caujolle-Bert, F1RMB
 *
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

#include "io/display.h"
#include "hardware/UC1701.h"
#if defined(LANGUAGE_BUILD_JAPANESE)
#include "hardware/UC1701_charset_JA.h"
#else
#include "hardware/UC1701_charset.h"
#endif
#include "functions/settings.h"
#include "user_interface/uiLocalisation.h"
#include "user_interface/menuSystem.h"
#include "utils.h"

// number representing the maximum angle (e.g. if 100, then if you pass in start=0 and end=50, you get a half circle)
// this can be changed with setArcParams function at runtime
#define DEFAULT_ARC_ANGLE_MAX 360.0f
// rotational offset in degrees defining position of value 0 (-90 will put it at the top of circle)
// this can be changed with setAngleOffset function at runtime
#define DEFAULT_ANGLE_OFFSET -90.0f
static float _arcAngleMax = DEFAULT_ARC_ANGLE_MAX;
static float _angleOffset = DEFAULT_ANGLE_OFFSET;
#define DEG_TO_RAD  0.017453292519943295769236907684886f
#define RAD_TO_DEG 57.295779513082320876798154814105f

extern bool headerRowIsDirty;


static __attribute__((section(".data.$RAM2"))) uint8_t screenBufData[((DISPLAY_SIZE_X * DISPLAY_SIZE_Y) >> 3)];
uint8_t *screenBuf = screenBufData;

//#define DISPLAY_CHECK_BOUNDS

#ifdef DISPLAY_CHECK_BOUNDS
static const uint8_t *screenBufEnd = screenBuf + sizeof(screenBuf);
#endif

int16_t displaySetPixel(int16_t x, int16_t y, bool color)
{
	int16_t i;

	i = ((y >> 3) << 7) + x;
	if (i < 0 || i > 1023)
	{
		return -1;// off the screen
	}

	if (color)
	{
		screenBuf[i] |= (0x1 << (y & 7));
	}
	else
	{
		screenBuf[i] &= ~(0x1 << (y & 7));
	}
	return 0;
}

void displayRenderWithoutNotification(void)
{
	displayRenderRows(0, DISPLAY_NUMBER_OF_ROWS);
	headerRowIsDirty = false;
}

void displayRender(void)
{
	if (uiNotificationIsVisible())
	{
		uiNotificationRefresh();
	}
	else
	{
		displayRenderRows(0, DISPLAY_NUMBER_OF_ROWS);
	}
	headerRowIsDirty = false;
}

//#define DISPLAY_CHECK_BOUNDS
#ifdef DISPLAY_CHECK_BOUNDS
static inline bool checkWritePos(uint8_t * writePos)
{
	if (writePos < screenBuf || writePos > screenBufEnd)
	{
#if defined(USING_EXTERNAL_DEBUGGER)
		SEGGER_RTT_printf(0,"Display buffer error\n");
#endif
		return false;
	}
	return true;
}
#endif

int displayPrintCore(int16_t x, int16_t y, const char *szMsg, ucFont_t fontSize, ucTextAlign_t alignment, bool isInverted)
{
#if ! defined(PLATFORM_GD77S)
	int16_t sLen;
	uint8_t *currentCharData;
	int16_t charWidthPixels;
	int16_t charHeightPixels;
	int16_t bytesPerChar;
	int16_t startCode;
	int16_t endCode;
	uint8_t *currentFont;
	uint8_t *writePos;
	uint8_t *readPos;

    sLen = strlen(szMsg);

    switch(fontSize)
    {
#if defined(PLATFORM_RD5R)
       	case FONT_SIZE_1:
    		currentFont = (uint8_t *) font_6x8;
    		break;
    	case FONT_SIZE_1_BOLD:
			currentFont = (uint8_t *) font_6x8_bold;
    		break;
    	case FONT_SIZE_2:
    		currentFont = (uint8_t *) font_8x8;//font_8x8;
    		break;
    	case FONT_SIZE_3:
    		currentFont = (uint8_t *) font_8x8;//font_8x16;
			break;
    	case FONT_SIZE_4:
    		currentFont = (uint8_t *) font_8x16;// font_16x32;
			break;
#else
    	case FONT_SIZE_1:
    		currentFont = (uint8_t *) font_6x8;
    		break;
    	case FONT_SIZE_1_BOLD:
			currentFont = (uint8_t *) font_6x8_bold;
    		break;
    	case FONT_SIZE_2:
    		currentFont = (uint8_t *) font_8x8;
    		break;
    	case FONT_SIZE_3:
    		currentFont = (uint8_t *) font_8x16;
			break;
    	case FONT_SIZE_4:
    		currentFont = (uint8_t *) font_16x32;
			break;
#endif
    	default:
    		return -2;// Invalid font selected
    		break;
    }

    startCode   		= currentFont[2];  // get first defined character
    endCode 	  		= currentFont[3];  // get last defined character
    charWidthPixels   	= currentFont[4];  // width in pixel of one char
    charHeightPixels  	= currentFont[5];  // page count per char
    bytesPerChar 		= currentFont[7];  // bytes per char

    if ((charWidthPixels*sLen) + x > DISPLAY_SIZE_X)
	{
    	sLen = (DISPLAY_SIZE_X - x) / charWidthPixels;
	}

	if (sLen < 0)
	{
		return -1;
	}

	switch(alignment)
	{
		case TEXT_ALIGN_LEFT:
			// left aligned, do nothing.
			break;
		case TEXT_ALIGN_CENTER:
			x = (DISPLAY_SIZE_X - (charWidthPixels * sLen)) >> 1;
			break;
		case TEXT_ALIGN_RIGHT:
			x = DISPLAY_SIZE_X - (charWidthPixels * sLen);
			break;
	}

	for (int16_t i = 0; i < sLen; i++)
	{
		uint32_t charOffset = (szMsg[i] - startCode);

		// End boundary checking.
		if (charOffset > endCode)
		{
			charOffset = ('?' - startCode); // Substitute unsupported ASCII code by a question mark
		}

		currentCharData = (uint8_t *)&currentFont[8 + (charOffset * bytesPerChar)];

		for(int16_t row = 0; row < charHeightPixels / 8 ; row++)
		{
			readPos = (currentCharData + row * charWidthPixels);
			writePos = (screenBuf + x + (i * charWidthPixels) + ((y >> 3) + row) * DISPLAY_SIZE_X) ;

			if ((y & 0x07) == 0)
			{
				// y position is aligned to a row
				for(int16_t p = 0; p < charWidthPixels; p++)
				{
					if (isInverted)
					{
#ifdef DISPLAY_CHECK_BOUNDS
						checkWritePos(writePos);
#endif
						*writePos++ &= ~(*readPos++);
					}
					else
					{
#ifdef DISPLAY_CHECK_BOUNDS
						checkWritePos(writePos);
#endif
						*writePos++ |= *readPos++;
					}
				}
			}
			else
			{
				int16_t shiftNum = y & 0x07;
				// y position is NOT aligned to a row

				for(int16_t p = 0; p < charWidthPixels; p++)
				{
					if (isInverted)
					{
#ifdef DISPLAY_CHECK_BOUNDS
						checkWritePos(writePos);
#endif
						*writePos++ &= ~((*readPos++) << shiftNum);
					}
					else
					{
#ifdef DISPLAY_CHECK_BOUNDS
						checkWritePos(writePos);
#endif
						*writePos++ |= ((*readPos++) << shiftNum);
					}
				}

				readPos = (currentCharData + row * charWidthPixels);
				writePos = (screenBuf + x + (i * charWidthPixels) + ((y >> 3) + row + 1) * DISPLAY_SIZE_X) ;

				for(int16_t p = 0; p < charWidthPixels; p++)
				{
					if (isInverted)
					{
#ifdef DISPLAY_CHECK_BOUNDS
						checkWritePos(writePos);
#endif
						*writePos++ &= ~((*readPos++) >> (8 - shiftNum));
					}
					else
					{
#ifdef DISPLAY_CHECK_BOUNDS
						checkWritePos(writePos);
#endif
						*writePos++ |= ((*readPos++) >> (8 - shiftNum));
					}
				}
			}
		}
	}
#endif // ! PLATFORM_GD77S
	return 0;
}

void displayClearBuf(void)
{
	memset(screenBuf, 0x00, ((DISPLAY_SIZE_X * DISPLAY_SIZE_Y) >> 3));
}

void displayClearRows(int16_t startRow, int16_t endRow, bool isInverted)
{
	// Boundaries
	if (((startRow < 0) || (endRow < 0)) || ((startRow > 8) || (endRow > 8)) || (startRow == endRow))
		return;

	if (endRow < startRow)
	{
		SAFE_SWAP(startRow, endRow);
	}

	// memset would be faster than ucFillRect
	//ucFillRect(0, (startRow * 8), 128, (8 * (endRow - startRow)), true);
    memset(screenBuf + (DISPLAY_SIZE_X * startRow), (isInverted ? 0xFF : 0x00), (DISPLAY_SIZE_X * (endRow - startRow)));
}

void displayPrintCentered(uint8_t y,const char *text, ucFont_t fontSize)
{
	displayPrintCore(0, y, text, fontSize, TEXT_ALIGN_CENTER, false);
}

void displayPrintAt(uint8_t x, uint8_t y, const char *text, ucFont_t fontSize)
{
	displayPrintCore(x, y, text, fontSize, TEXT_ALIGN_LEFT, false);
}

// Bresenham's algorithm - thx wikpedia
void displayDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color)
{
	bool steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep)
	{
		SAFE_SWAP(x0, y0);
		SAFE_SWAP(x1, y1);
	}

	if (x0 > x1)
	{
		SAFE_SWAP(x0, x1);
		SAFE_SWAP(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx >> 1;
	int16_t ystep;

	ystep = ((y0 < y1) ? 1 : -1);

	for (; x0 <= x1; x0++)
	{
		if (steep)
		{
			displaySetPixel(y0, x0, color);
		}
		else
		{
			displaySetPixel(x0, y0, color);
		}

		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

void displayDrawFastVLine(int16_t x, int16_t y, int16_t h, bool color)
{
	displayFillRect(x, y, 1, h, !color);
}

void displayDrawFastHLine(int16_t x, int16_t y, int16_t w, bool color)
{
	displayFillRect(x, y, w, 1, !color);
}

// Draw a circle outline
void displayDrawCircle(int16_t x0, int16_t y0, int16_t r, bool color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	displaySetPixel(x0    , y0 + r, color);
	displaySetPixel(x0    , y0 - r, color);
	displaySetPixel(x0 + r, y0    , color);
	displaySetPixel(x0 - r, y0    , color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		displaySetPixel(x0 + x, y0 + y, color);
		displaySetPixel(x0 - x, y0 + y, color);
		displaySetPixel(x0 + x, y0 - y, color);
		displaySetPixel(x0 - x, y0 - y, color);
		displaySetPixel(x0 + y, y0 + x, color);
		displaySetPixel(x0 - y, y0 + x, color);
		displaySetPixel(x0 + y, y0 - x, color);
		displaySetPixel(x0 - y, y0 - x, color);
	}
}

void displayDrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, bool color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}

		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (cornername & 0x4)
		{
			displaySetPixel(x0 + x, y0 + y, color);
			displaySetPixel(x0 + y, y0 + x, color);
		}

		if (cornername & 0x2)
		{
			displaySetPixel(x0 + x, y0 - y, color);
			displaySetPixel(x0 + y, y0 - x, color);
		}

		if (cornername & 0x8)
		{
			displaySetPixel(x0 - y, y0 + x, color);
			displaySetPixel(x0 - x, y0 + y, color);
		}

		if (cornername & 0x1)
		{
			displaySetPixel(x0 - y, y0 - x, color);
			displaySetPixel(x0 - x, y0 - y, color);
		}
	}
}

/*
 * Used to do circles and roundrects
 */
void displayFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, bool color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}

		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (cornername & 0x1)
		{
			displayDrawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			displayDrawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}

		if (cornername & 0x2)
		{
			displayDrawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
			displayDrawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}

void displayFillCircle(int16_t x0, int16_t y0, int16_t r, bool color)
{
	displayDrawFastVLine(x0, y0 - r, 2 * r + 1, color);
	displayFillCircleHelper(x0, y0, r, 3, 0, color);
}

/*
 * ***** Arc related functions *****
 */
static float cosDegrees(float angle)
{
	return cos(angle * DEG_TO_RAD);
}

static float sinDegrees(float angle)
{
	return sin(angle * DEG_TO_RAD);
}

/*
 * DrawArc function thanks to Jnmattern and his Arc_2.0 (https://github.com/Jnmattern)
 */
void displayFillArcOffsetted(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t thickness, float start, float end, bool color)
{
	int16_t xmin = 65535, xmax = -32767, ymin = 32767, ymax = -32767;
	float cosStart, sinStart, cosEnd, sinEnd;
	float r, t;
	float startAngle, endAngle;

	startAngle = (start / _arcAngleMax) * 360.0f;	// 252
	endAngle = (end / _arcAngleMax) * 360.0f;		// 807

	while (startAngle < 0.0f)
	{
		startAngle += 360.0f;
	}
	while (endAngle < 0.0f)
	{
		endAngle += 360.0f;
	}
	while (startAngle > 360.0f)
	{
		startAngle -= 360.0f;
	}
	while (endAngle > 360.0f)
	{
		endAngle -= 360.0f;
	}

	if (startAngle > endAngle)
	{
		displayFillArcOffsetted(cx, cy, radius, thickness, ((startAngle) / 360.0f) * _arcAngleMax, _arcAngleMax, color);
		displayFillArcOffsetted(cx, cy, radius, thickness, 0, ((endAngle) / 360.0f) * _arcAngleMax, color);
	}
	else
	{
		// Calculate bounding box for the arc to be drawn
		cosStart = cosDegrees(startAngle);
		sinStart = sinDegrees(startAngle);
		cosEnd = cosDegrees(endAngle);
		sinEnd = sinDegrees(endAngle);

		r = radius;
		// Point 1: radius & startAngle
		t = r * cosStart;
		if (t < xmin)
		{
			xmin = t;
		}
		if (t > xmax)
		{
			xmax = t;
		}
		t = r * sinStart;
		if (t < ymin)
		{
			ymin = t;
		}
		if (t > ymax)
		{
			ymax = t;
		}

		// Point 2: radius & endAngle
		t = r * cosEnd;
		if (t < xmin)
		{
			xmin = t;
		}
		if (t > xmax)
		{
			xmax = t;
		}
		t = r * sinEnd;
		if (t < ymin)
		{
			ymin = t;
		}
		if (t > ymax)
		{
			ymax = t;
		}

		r = radius - thickness;
		// Point 3: radius-thickness & startAngle
		t = r * cosStart;
		if (t < xmin)
		{
			xmin = t;
		}
		if (t > xmax)
		{
			xmax = t;
		}
		t = r * sinStart;
		if (t < ymin)
		{
			ymin = t;
		}
		if (t > ymax)
		{
			ymax = t;
		}

		// Point 4: radius-thickness & endAngle
		t = r * cosEnd;
		if (t < xmin)
		{
			xmin = t;
		}
		if (t > xmax)
		{
			xmax = t;
		}
		t = r * sinEnd;
		if (t < ymin)
		{
			ymin = t;
		}
		if (t > ymax)
		{
			ymax = t;
		}

		// Corrections if arc crosses X or Y axis
		if ((startAngle < 90.0f) && (endAngle > 90.0f))
		{
			ymax = radius;
		}

		if ((startAngle < 180.0f) && (endAngle > 180.0f))
		{
			xmin = -radius;
		}

		if ((startAngle < 270.0f) && (endAngle > 270.0f))
		{
			ymin = -radius;
		}

		// Slopes for the two sides of the arc
		float sslope = cosStart / sinStart;
		float eslope = cosEnd / sinEnd;

		if (endAngle == 360.0f) eslope = -1000000.0f;

		int ir2 = (radius - thickness) * (radius - thickness);
		int or2 = radius * radius;

		for (int x = xmin; x <= xmax; x++)
		{
			bool y1StartFound = false, y2StartFound = false;
			bool y1EndFound = false, y2EndSearching = false;
			int y1s = 0, y1e = 0, y2s = 0;
			for (int y = ymin; y <= ymax; y++)
			{
				int x2 = x * x;
				int y2 = y * y;

				if (
					(x2 + y2 < or2 && x2 + y2 >= ir2) && (
					(y > 0.0f && startAngle < 180.0f && x <= y * sslope) ||
					(y < 0.0f && startAngle > 180.0f && x >= y * sslope) ||
					(y < 0.0f && startAngle <= 180.0f) ||
					(y == 0.0f && startAngle <= 180.0f && x < 0.0f) ||
					(y == 0.0f && startAngle == 0.0f && x > 0.0f)
					) && (
					(y > 0.0f && endAngle < 180.0f && x >= y * eslope) ||
					(y < 0.0f && endAngle > 180.0f && x <= y * eslope) ||
					(y > 0.0f && endAngle >= 180.0f) ||
					(y == 0.0f && endAngle >= 180.0f && x < 0.0f) ||
					(y == 0.0f && startAngle == 0.0f && x > 0.0f)))
				{
					if (!y1StartFound)	//start of the higher line found
					{
						y1StartFound = true;
						y1s = y;
					}
					else if (y1EndFound && !y2StartFound) //start of the lower line found
					{
						y2StartFound = true;
						//drawPixel_cont(cx+x, cy+y, ILI9341_BLUE);
						y2s = y;
						y += y1e - y1s - 1;	// calculate the most probable end of the lower line (in most cases the length of lower line is equal to length of upper line), in the next loop we will validate if the end of line is really there
						if (y > ymax - 1) // the most probable end of line 2 is beyond ymax so line 2 must be shorter, thus continue with pixel by pixel search
						{
							y = y2s;	// reset y and continue with pixel by pixel search
							y2EndSearching = true;
						}
					}
					else if (y2StartFound && !y2EndSearching)
					{
						// we validated that the probable end of the lower line has a pixel, continue with pixel by pixel search, in most cases next loop with confirm the end of lower line as it will not find a valid pixel
						y2EndSearching = true;
					}
				}
				else
				{
					if (y1StartFound && !y1EndFound) //higher line end found
					{
						y1EndFound = true;
						y1e = y - 1;
						displayDrawFastVLine(cx + x, cy + y1s, y - y1s, color);
						if (y < 0)
						{
							y = abs(y); // skip the empty middle
						}
						else
							break;
					}
					else if (y2StartFound)
					{
						if (y2EndSearching)
						{
							// we found the end of the lower line after pixel by pixel search
							displayDrawFastVLine(cx + x, cy + y2s, y - y2s, color);
							y2EndSearching = false;
							break;
						}
						else
						{
							// the expected end of the lower line is not there so the lower line must be shorter
							y = y2s;	// put the y back to the lower line start and go pixel by pixel to find the end
							y2EndSearching = true;
						}
					}
				}
			}
			if (y1StartFound && !y1EndFound)
			{
				y1e = ymax;
				displayDrawFastVLine(cx + x, cy + y1s, y1e - y1s + 1, color);
			}
			else if (y2StartFound && y2EndSearching)	// we found start of lower line but we are still searching for the end
			{										// which we haven't found in the loop so the last pixel in a column must be the end
				displayDrawFastVLine(cx + x, cy + y2s, ymax - y2s + 1, color);
			}
		}
	}
}

void displayFillArc(uint16_t x, uint16_t y, uint16_t radius, uint16_t thickness, float start, float end, bool color)
{
	if (start == 0.0f && end == _arcAngleMax)
	{
		displayFillArcOffsetted(x, y, radius, thickness, 0, _arcAngleMax, color);
	}
	else
	{
		displayFillArcOffsetted(x, y, radius, thickness, start + (_angleOffset / 360.0f)*_arcAngleMax, end + (_angleOffset / 360.0f)*_arcAngleMax, color);
	}
}
/*
 * ***** End of Arc related functions *****
 */

void displayDrawEllipse(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color)
{
	int16_t a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1; /* values of diameter */
	long dx = 4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a; /* error increment */
	long err = dx + dy + b1 * a * a, e2; /* error of 1.step */

	if (x0 > x1)
	{
		x0 = x1;
		x1 += a;
	} /* if called with swapped points */
	if (y0 > y1)
	{
		y0 = y1; /* .. exchange them */
	}
	y0 += (b + 1) >> 1; /* starting pixel */
	y1 = y0 - b1;
	a *= 8 * a;
	b1 = 8 * b * b;

	do {
		displaySetPixel(x1, y0, color); /*   I. Quadrant */
		displaySetPixel(x0, y0, color); /*  II. Quadrant */
		displaySetPixel(x0, y1, color); /* III. Quadrant */
		displaySetPixel(x1, y1, color); /*  IV. Quadrant */
		e2 = 2 * err;
		if (e2 >= dx)
		{
			x0++;
			x1--;
			err += dx += b1;
		} /* x step */
		if (e2 <= dy)
		{
			y0++;
			y1--;
			err += dy += a;
		}  /* y step */
	} while (x0 <= x1);

	while (y0 - y1 < b) /* too early stop of flat ellipses a=1 */
	{
		displaySetPixel(x0 - 1, ++y0, color); /* -> complete tip of ellipse */
		displaySetPixel(x0 - 1, --y1, color);
	}
}

/*
 * Draw a triangle
 */
void displayDrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool color)
{
	displayDrawLine(x0, y0, x1, y1, color);
	displayDrawLine(x1, y1, x2, y2, color);
	displayDrawLine(x2, y2, x0, y0, color);
}

/*
 * Fill a triangle
 */
void displayFillTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool color)
{
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1)
	{
		SAFE_SWAP(y0, y1); SAFE_SWAP(x0, x1);
	}
	if (y1 > y2)
	{
		SAFE_SWAP(y2, y1); SAFE_SWAP(x2, x1);
	}
	if (y0 > y1)
	{
		SAFE_SWAP(y0, y1); SAFE_SWAP(x0, x1);
	}

	if(y0 == y2) // Handle awkward all-on-same-line case as its own thing
	{
		a = b = x0;
		if(x1 < a)
		{
			a = x1;
		}
		else if(x1 > b)
		{
			b = x1;
		}

		if(x2 < a)
		{
			a = x2;
		}
		else if(x2 > b)
		{
			b = x2;
		}

		displayDrawFastHLine(a, y0, b - a + 1, color);
		return;
	}

	int16_t dx01 = x1 - x0,
			dy01 = y1 - y0,
			dx02 = x2 - x0,
			dy02 = y2 - y0,
			dx12 = x2 - x1,
			dy12 = y2 - y1,
			sa   = 0,
			sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if(y1 == y2) last = y1;   // Include y1 scanline
	else         last = y1-1; // Skip it

	for(y = y0; y <= last; y++)
	{
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b)
		{
			SAFE_SWAP(a,b);
		}

		displayDrawFastHLine(a, y, b - a + 1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);

	for(; y <= y2; y++)
	{
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b)
		{
			SAFE_SWAP(a,b);
		}

		displayDrawFastHLine(a, y, b - a + 1, color);
	}
}

/*
 * Draw a rounded rectangle
 */
void displayDrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool color)
{
	// smarter version
	displayDrawFastHLine(x + r    , y        , w - 2 * r, color); // Top
	displayDrawFastHLine(x + r    , y + h - 1, w - 2 * r, color); // Bottom
	displayDrawFastVLine(x        , y + r    , h - 2 * r, color); // Left
	displayDrawFastVLine(x + w - 1, y + r    , h - 2 * r, color); // Right
	// draw four corners
	displayDrawCircleHelper(x + r        , y + r        , r, 1, color);
	displayDrawCircleHelper(x + w - r - 1, y + r        , r, 2, color);
	displayDrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	displayDrawCircleHelper(x + r        , y + h - r - 1, r, 8, color);
}

/*
 * Fill a rounded rectangle
 */
void displayFillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool color)
{
	displayFillRect(x + r, y, w - 2 * r, h, !color);

	// draw four corners
	displayFillCircleHelper(x+w-r-1, y + r, r, 1, h - 2 * r - 1, color);
	displayFillCircleHelper(x+r    , y + r, r, 2, h - 2 * r - 1, color);
}

/*
 *
 */
void displayDrawRoundRectWithDropShadow(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool color)
{
	displayFillRoundRect(x + 2, y, w, h, r, color); // Shadow
	displayFillRoundRect(x, y - 2, w, h, r, !color); // Empty box
	displayDrawRoundRect(x, y - 2, w, h, r, color); // Outline
}

/*
 * Draw a rectangle
 */
void displayDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color)
{
	displayDrawFastHLine(x        , y        , w, color);
	displayDrawFastHLine(x        , y + h - 1, w, color);
	displayDrawFastVLine(x        , y        , h, color);
	displayDrawFastVLine(x + w - 1, y        , h, color);
}

/*
 * Fill a rectangle
 */
void displayFillRect(int16_t x, int16_t y, int16_t width, int16_t height, bool isInverted)
{
	uint8_t *addPtr;
	int16_t endStripe 	= x + width;
	int16_t startRow 	= y >> 3;
	int16_t endRow 		= ((y + height) >> 3);
	uint8_t bitPatten;
	int16_t shiftNum;

	if (startRow == endRow)
	{
		addPtr = screenBuf + (startRow << 7);
		bitPatten = (0xff >> (8 - (height & 0x07))) << (y & 0x07);
		//bitPatten = bitPatten ;
		for(int16_t stripe = x; stripe < endStripe; stripe++)
		{
			if (isInverted)
			{
				*(addPtr + stripe) &= ~ bitPatten;
			}
			else
			{
				*(addPtr + stripe) |= bitPatten;
			}
		}
	}
	else
	{
		for(int16_t row = startRow; row <= endRow; row++)
		{
			if (row == startRow)
			{
				shiftNum = y & 0x07;
				bitPatten = (0xff << shiftNum);
			}
			else
			{
				if (row == endRow)
				{
					shiftNum = (y + height) & 0x07;
					bitPatten = (0xff >> (8 - shiftNum));
				}
				else
				{
					// middle rows
					bitPatten = 0xff;
				}
			}
			addPtr = screenBuf + (row << 7);
			for(int16_t stripe = x; stripe < endStripe; stripe++)
			{
				if (isInverted)
				{
					*(addPtr + stripe) &= ~ bitPatten;
				}
				else
				{
					*(addPtr + stripe) |= bitPatten;
				}
			}
		}
	}
}

/*
 *
 */
void displayDrawRectWithDropShadow(int16_t x, int16_t y, int16_t w, int16_t h, bool color)
{
	displayFillRect(x + 2, y, w, h, !color); // Shadow
	displayFillRect(x, y - 2, w, h, color); // Empty box
	displayDrawRect(x, y - 2, w, h, color); // Outline
}

/*
 * Draw a 1-bit image at the specified (x,y) position.
*/
void displayDrawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, bool color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j = 0; j < h; j++, y++)
    {
        for(int16_t i = 0; i < w; i++)
        {
            if(i & 7)
            {
            	byte <<= 1;
            }
            else
            {
            	byte = *(bitmap + (j * byteWidth + i / 8));
            }

            if(byte & 0x80)
            {
            	displaySetPixel(x + i, y, color);
            }
        }
    }
}

/*
 * Draw XBitMap Files (*.xbm), e.g. exported from GIMP.
*/
void displayDrawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, bool color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j = 0; j < h; j++, y++)
    {
        for(int16_t i = 0; i < w; i++)
        {
            if(i & 7)
            {
            	byte >>= 1;
            }
            else
            {
            	byte = *(bitmap + (j * byteWidth + i / 8));
            }
            // Nearly identical to drawBitmap(), only the bit order
            // is reversed here (left-to-right = LSB to MSB):
            if(byte & 0x01)
            {
            	displaySetPixel(x + i, y, color);
            }
        }
    }
}

void displayDrawChoice(ucChoice_t choice, bool clearRegion)
{
#if defined(PLATFORM_RD5R)
	const uint8_t TEXT_Y = 40;
	const uint8_t FILLRECT_Y = 32;
#else
	const uint8_t TEXT_Y = 49;
	const uint8_t FILLRECT_Y = 48;
#endif
	const uint8_t TEXT_L_CENTER_X = 12;
	const uint8_t TEXT_R_CENTER_X = 115;

	struct
	{
		char *lText;
		char *rText;
	} choices[] =
	{
			{ "OK" 							             , NULL                                        }, // UC1701_CHOICE_OK
			{ (char *)currentLanguage->yes___in_uppercase, (char *)currentLanguage->no___in_uppercase  }, // UC1701_CHOICE_YESNO
			{ NULL						                 , (char *)currentLanguage->DISMISS            },  // UC1701_CHOICE_DISMISS
			{ "OK" 							             , NULL                                        }  // UC1701_CHOICE_OKARROWS
	};
	char *lText = NULL;
	char *rText = NULL;


	const int ucTriangleArrows[2][6] = {
			{ // Down
					(DISPLAY_SIZE_X/2)-12, (TEXT_Y + (FONT_SIZE_3_HEIGHT / 2) - 1),
					(DISPLAY_SIZE_X/2)-4, (TEXT_Y + (FONT_SIZE_3_HEIGHT / 2) - (FONT_SIZE_3_HEIGHT / 4) - 1),
					(DISPLAY_SIZE_X/2)-4, (TEXT_Y + (FONT_SIZE_3_HEIGHT / 2) + (FONT_SIZE_3_HEIGHT / 4) - 1)
			}, // Up
			{
					(DISPLAY_SIZE_X/2)+4, (TEXT_Y + (FONT_SIZE_3_HEIGHT / 2) + (FONT_SIZE_3_HEIGHT / 4) - 1),
					(DISPLAY_SIZE_X/2)+4, (TEXT_Y + (FONT_SIZE_3_HEIGHT / 2) - (FONT_SIZE_3_HEIGHT / 4) - 1),
					(DISPLAY_SIZE_X/2)+12, (TEXT_Y + (FONT_SIZE_3_HEIGHT / 2) - 1)
			}
	};


	if (clearRegion)
	{
		displayFillRect(0, FILLRECT_Y, DISPLAY_SIZE_X, 16, true);
	}

	if (choice >= CHOICES_NUM)
	{
		return;
	}

	lText = choices[choice].lText;
	rText = choices[choice].rText;

	if (lText)
	{
		int16_t x = (TEXT_L_CENTER_X - ((strlen(lText) * 8) >> 1));

		if (x < 2)
		{
			x = 2;
		}
		displayPrintAt(x, TEXT_Y, lText, FONT_SIZE_3);
	}

	if(rText)
	{
		size_t len = (strlen(rText) * 8);
		int16_t x = (TEXT_R_CENTER_X - (len >> 1));

		if ((x + len) > 126)
		{
			x = (126 - len);
		}
		displayPrintAt(x, TEXT_Y, rText, FONT_SIZE_3);
	}

	if (choice == CHOICES_OKARROWS)
	{
		displayFillTriangle(ucTriangleArrows[0][0], ucTriangleArrows[0][1],
				ucTriangleArrows[0][2], ucTriangleArrows[0][3],
				ucTriangleArrows[0][4], ucTriangleArrows[0][5], true);
		displayFillTriangle(ucTriangleArrows[1][0], ucTriangleArrows[1][1],
				ucTriangleArrows[1][2], ucTriangleArrows[1][3],
				ucTriangleArrows[1][4], ucTriangleArrows[1][5], true);
	}
}

uint8_t *displayGetScreenBuffer(void)
{
	return screenBuf;
}

void displayRestorePrimaryScreenBuffer(void)
{
	screenBuf = screenBufData;
}

uint8_t *displayGetPrimaryScreenBuffer(void)
{
	return &screenBufData[0];
}

void displayOverrideScreenBuffer(uint8_t *buffer)
{
	screenBuf = buffer;
}
