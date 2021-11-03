/*
 * Pcx.cpp
 *
 *  Created on: 02.11.2021
 *      Author: Andreas Volz
 */

#include "endian.h"

// C
#include <stdlib.h>
#include <string.h>

// could stay in cpp files as it's private implementation
struct PCXheader
{
	unsigned char Manufacturer;
	unsigned char Version;
	unsigned char Encoding;
	unsigned char BitsPerPixel;
	short Xmin, Ymin, Xmax, Ymax;
	short HDpi, VDpi;
	unsigned char Colormap[48];
	unsigned char Reserved;
	unsigned char NPlanes;
	short BytesPerLine;
	short PaletteInfo;
	short HscreenSize;
	short VscreenSize;
	unsigned char Filler[54];
};

/**
 **  Convert 8 bit pcx file to raw image
 */
void ConvertPcxToRaw(unsigned char *pcx, unsigned char **raw,
		unsigned char **pal, int *w, int *h)
{
	struct PCXheader pcxh;
	int y;
	int i;
	int count;
	unsigned char *src;
	unsigned char *dest;
	unsigned char ch;

	ch = 0;
	memcpy(&pcxh, pcx, sizeof(struct PCXheader));
	pcxh.Xmin = ConvertLE16(pcxh.Xmin);
	pcxh.Ymin = ConvertLE16(pcxh.Ymin);
	pcxh.Xmax = ConvertLE16(pcxh.Xmax);
	pcxh.Ymax = ConvertLE16(pcxh.Ymax);
	pcxh.BytesPerLine = ConvertLE16(pcxh.BytesPerLine);

	*w = pcxh.Xmax - pcxh.Xmin + 1;
	*h = pcxh.Ymax - pcxh.Ymin + 1;

	*raw = (unsigned char*) malloc(*w * *h);
	src = pcx + sizeof(struct PCXheader);

	for (y = 0; y < *h; ++y)
	{
		count = 0;
		dest = *raw + y * *w;
		for (i = 0; i < *w; ++i)
		{
			if (!count)
			{
				ch = *src++;
				if ((ch & 0xc0) == 0xc0)
				{
					count = ch & 0x3f;
					ch = *src++;
				}
				else
				{
					count = 1;
				}
			}
			dest[i] = ch;
			--count;
		}
	}

	*pal = (unsigned char*) malloc(256 * 3);
	dest = *pal;
	do
	{
		ch = *src++;
	} while (ch != 12);

	for (i = 0; i < 256 * 3; ++i)
	{
		*dest++ = *src++;
	}
}