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

#
#ifndef	__READER__
#define	__READER__

#include	"ringbuffer.h"
#include	<stdint.h>
#include	<atomic>
#include	<complex>
#include	"basics.h"
#include	<unistd.h>
//
//	For now we have a simple abstraction layer over a ringbuffer
//	that provides a suitable buffer.  It acts as a - more or
//	less regular - array, however, how the data gets in is kept
//	a secret.
class	theReader {
public:
				theReader (RingBuffer<std::complex<float>> *,
	                                   int32_t);
				~theReader (void);
	void			waitfor		(int32_t);
	void			shiftBuffer	(int16_t);
	void			setStart	(int);
	void			read		(std::complex<float> *,
	                                          int, int);
	uint32_t		bufSize;
	uint32_t		bufMask;
	std::complex<float>	*data;
	uint32_t		currentIndex;
	void			stopSign	();
protected:
	uint32_t		Contents	(void);
	uint32_t		firstFreeCell;
	RingBuffer<std::complex<float>> * ringBuffer;
	std::atomic<bool>	running;
	std::complex<float>	freqVec [192000];
	int			freqPos;
};

#endif

