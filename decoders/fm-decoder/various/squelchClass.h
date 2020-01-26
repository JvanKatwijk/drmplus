#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair programming
 *
 *    This file is part of the DRM+ Decoder
 *
 *    DRM+ Decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ Decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SQUELCHCLASS__
#define	__SQUELCHCLASS__

#include	"iir-filters.h"
//
//	just a simple class to include elementary squelch handling
//	The basic idea is that when there is no signal, the noise
//	in the upper bands will be roughly as high as in the lowerbands.
//	Measurement shows that the average amplitude of the noise in the
//	upper band is roughly 0.6 times that of the lower part.
//	If the average signal value of the upper part is larger
//	than factor times the average signal value of the lower part,
//	where factor is a value between 0 .. 1, set by the user.
#define	SQUELCH_HYSTERESIS	0.01

class	squelch {
private:
	int16_t		squelchThreshold;
	int32_t		keyFrequency;
	int32_t		holdPeriod;
	int32_t		sampleRate;
	bool		squelchSuppress;
	int32_t		squelchCount;
	float		Average_High;
	float		Average_Low;
	HighPassIIR	squelchHighpass;
	LowPassIIR	squelchLowpass;
public:
	squelch (int32_t	squelchThreshold,
	         int32_t	keyFrequency,
	         int32_t	bufsize,
	         int32_t	sampleRate):
	               squelchHighpass (20,
	                                keyFrequency - 100,
	                                sampleRate, 
	                                S_CHEBYSHEV),
	               squelchLowpass  (20,
	                                keyFrequency,
	                                sampleRate,
	                                S_CHEBYSHEV) {
	this	-> squelchThreshold	= squelchThreshold;
	this	-> keyFrequency		= keyFrequency;
	this	-> holdPeriod		= bufsize;
	this	-> sampleRate		= sampleRate;

	squelchSuppress			= false;
	squelchCount			= 0;
	Average_High			= 0;
	Average_Low			= 0;
}

	~squelch (void) {
}

void		setSquelchLevel (int n) {
	squelchThreshold = n;
}

static inline
float	 decayingAverage (float old, float input, float weight) {
	if (weight <= 1)
	   return input;
	return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}

std::complex<float>	do_squelch (std::complex<float>	soundSample) {
float		val_1;
float		val_2;

	val_1	= abs (squelchHighpass. Pass (soundSample));
	val_2	= abs (squelchLowpass.       Pass (soundSample));

	Average_High	= decayingAverage (Average_High,
	                                   val_1, sampleRate / 100);
	Average_Low	= decayingAverage (Average_Low,
	                                   val_2, sampleRate / 100);

	if (++squelchCount < holdPeriod) {	// use current squelch state
	   if (squelchSuppress)
	      return std::complex<float> (0.001, 0.001);
	   else
	      return soundSample;
	}

	squelchCount = 0;
//	o.k. looking for a new squelch state
	if (squelchThreshold == 0)  	// force squelch if zero
	   squelchSuppress = true;
	else	// recompute 
	if (Average_High < Average_Low * squelchThreshold / 100.0 - SQUELCH_HYSTERESIS)
	   squelchSuppress = false;
	else
	if (Average_High >= Average_Low * squelchThreshold / 100.0 + SQUELCH_HYSTERESIS)
	   squelchSuppress = true;
//	else just keep old squelchSuppress value
	
	return squelchSuppress ?
	           std::complex<float>(0.001, 0.001) :
	           soundSample;
}
};

#endif

