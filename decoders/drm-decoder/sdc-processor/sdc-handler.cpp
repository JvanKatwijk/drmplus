#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of drm+-receiver
 *
 *    drm+ receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm+ receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm+ receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"sdc-handler.h"

	sdcHandler::sdcHandler (drmParameters *params,
	                        uint8_t Rx, uint8_t Ry,
	                        int	nr_sdcSamples):
	                            deconvolver (Rx * (2 * nr_sdcSamples - 12) / Ry) {
	this	-> Rx			= Rx;
	this	-> Ry			= Ry;
	this	-> nr_sdcSamples	= nr_sdcSamples;
	this	-> nr_sdcBits		= Rx * ((2 * nr_sdcSamples - 12) / Ry);
	this	-> deconvolveLength	= 6 * (nr_sdcBits + 6);
	this	-> punctureTable	= pt. getPunctureTable (Rx, Ry);
	this	-> residuTable		= pt. getResiduTable   (Rx, Ry, 
	                                                        nr_sdcSamples);
	punctureSize			= 6 * Rx;
}

	sdcHandler::~sdcHandler () {}

int	sdcHandler::get_sdcBits	() {
	return nr_sdcBits;
}

void	sdcHandler::process (metrics *v, uint8_t *sdcBits) {
metrics theBits [deconvolveLength + 10];
int	Cnt, i;

	Cnt	= 0;
//	the regular segment
	for (i = 0; i < 6 * nr_sdcBits; i ++) {
	   if (punctureTable [i % punctureSize] != 0)
	      theBits [i] = v [Cnt ++];
	   else {
	      theBits [i]. rTow0 = 0;
	      theBits [i]. rTow1 = 0;
	   }
	}

	for (i = 0; i < 36; i ++) {
	   if (residuTable [i] != 0)
	      theBits [6 * nr_sdcBits + i] = v [Cnt ++];
	   else {
	      theBits [6 * nr_sdcBits + i]. rTow0 = 0;
	      theBits [6 * nr_sdcBits + i]. rTow1 = 0;
	   }
	}

	deconvolver. deconvolve (theBits, nr_sdcBits, sdcBits);
}

