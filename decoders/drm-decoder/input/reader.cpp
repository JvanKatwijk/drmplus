#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ decoder
 *
 *    DRM+ decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"reader.h"
#include	<math.h>
//
//	A simple interface class to the ringBuffer
//	The methods are called from the drmdecoder

#define	OFFSET	00

	theReader::theReader (RingBuffer<std::complex<float>> *r,
	                                              int32_t size) {
	ringBuffer		= r;
	this	-> bufSize	= size;
	if ((size & (size - 1)) != 0)
	   bufSize = 8 * 32768;
	this	-> bufMask		= bufSize - 1;
	data			= new std::complex<float> [bufSize];
	memset (data, 0, bufSize * sizeof (std::complex<float>));
	currentIndex		= 0;
	firstFreeCell		= 0;
	running. store (true);

	for (int i = 0; i < 192000; i ++)
	   freqVec [i] = std::complex<float> (cos (2 * M_PI * i / 192000),
	                                      sin (2 * M_PI * i / 192000));
}

	theReader::~theReader		(void) {
	running. store (false);
	delete []	data;
}

static int currentPhase	= 0;
void	theReader::read			(std::complex<float> *t,
	                                 int a, int shifter) {
	waitfor (a);
	for (int i = 0; i < a; i ++) {
	   t [i] = data [(currentIndex + i) & bufMask] *
	                                         freqVec [currentPhase];
	   currentPhase -= shifter;	// shifter in Hz
	   if (currentPhase < 0)
	      currentPhase += 192000;
	   else
	   if (currentPhase >= 192000)
	      currentPhase -= 192000;
	}
	currentIndex = (currentIndex + a) & bufMask;
}

void	theReader::setStart		(int offset) {
	waitfor		(offset);
	shiftBuffer	(offset);
}

void	theReader::stopSign		() {
	running. store (false);
}

uint32_t	theReader::Contents	(void) {
	if (firstFreeCell >= currentIndex)
	   return firstFreeCell - currentIndex;
	return (bufSize - currentIndex) + firstFreeCell;
}

//
void	theReader::waitfor (int32_t amount) {
uint32_t	tobeRead;
int32_t		contents	= Contents ();

	if (contents > amount)
	   return;
	tobeRead	= amount - contents;
	while ((ringBuffer -> GetRingBufferReadAvailable () < tobeRead) &&
	                                  running. load ()) {
	   usleep (1000);
	   if (!running. load ())
	      throw (1);
	}

	if (!running. load ())
	   return;
//	Ok, if the amount of samples to be read fits in a contiguous part
//	one read will suffice, otherwise it will be in two parts

	for (int i = 0; i < tobeRead; i ++) {
	   std::complex<float> temp;
	   ringBuffer -> getDataFromBuffer (&temp, 1);
	   temp = temp * freqVec [freqPos];
	   freqPos -= OFFSET;
	   if (freqPos < 0)
	      freqPos += 192000;
	   if (freqPos >= 192000)
	      freqPos -= 192000;
	   data [firstFreeCell] = temp;
	   firstFreeCell = (firstFreeCell + 1) % bufSize;
	}
}

void	theReader::shiftBuffer (int16_t n) {
	if (n > 0)
	   waitfor (n + 20);
	currentIndex = (currentIndex + n) % bufSize;
}

