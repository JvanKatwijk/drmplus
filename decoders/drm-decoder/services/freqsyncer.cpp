#
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

#include	"basics.h"
#include	"freqsyncer.h"
#include	<cstring>
#include	"reader.h"
#include	<math.h>
#include	"radio-constants.h"
#include	"referenceframe.h"

	freqSyncer::freqSyncer	(theReader *myReader) {
	this -> myReader = myReader;
	wordVector	= (std::complex<float> *)
	                       fftwf_malloc (sizeof (fftwf_complex) * Tu_t);

	plan		= fftwf_plan_dft_1d (Tu_t,
                                     reinterpret_cast <fftwf_complex *>(wordVector),
                                     reinterpret_cast <fftwf_complex *>(wordVector),
                                     FFTW_FORWARD, FFTW_ESTIMATE);
	freqVector	= new std::complex<float> [SAMPLE_RATE];
	for (int i = 0; i < SAMPLE_RATE; i ++)
	   freqVector [i] = std::complex<float> (cos (2 * M_PI * i / SAMPLE_RATE),
	                                         sin (2 * M_PI * i / SAMPLE_RATE));
	offsetIndex	= 0;
	theAngle	= 0;
}

	freqSyncer::~freqSyncer	() {
	fftwf_destroy_plan	(plan);
        fftwf_free		(wordVector);
	delete freqVector;
}

int	freqSyncer::sync	(drmParameters *p) {
float	weight [Tu_t];
int	symbol, carrier;
float	sum	= 0;
float	maxSum	= 0;
int	nullBase	= 0;
int	localIndex	= 0;
	memset (weight, 0, Tu_t * sizeof (float));
	for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	   int16_t time_offset_integer =
	      getWord (myReader -> data,
	               myReader -> bufSize,
	               myReader -> currentIndex + localIndex,
	               p -> timeOffset_fractional);
	   for (carrier = 0; carrier < Tu_t; carrier ++) 
	      weight [carrier] += abs (wordVector [carrier]);
	   localIndex += Ts_t;
	}

	sum = 0;
	for (carrier = 0; carrier < Tu_t; carrier ++)
	   weight [carrier] /= symbolsperFrame;

	for (carrier = 0; carrier < nrCarriers; carrier ++)
	   sum += weight [carrier];

	for (carrier = 0; carrier + nrCarriers < Tu_t; carrier ++) {
	   sum -= weight [carrier];
	   sum += weight [carrier + nrCarriers];
	   if (sum > maxSum) {
	      nullBase = carrier - K_min;
	      maxSum = sum;
	   }
	}
	sum = 0;
	int otherIndex = 0;
	for (carrier = Tu_t / 2 - 50; carrier < Tu_t / 2 + 50; carrier ++) {
	   float x = weight [carrier - 106] +
	             weight [carrier - 102] +
	             weight [carrier + 102] +
	             weight [carrier + 106];
	   if (x > sum) {
	      sum = x;
	      otherIndex = carrier;
	   }
	}

//	fprintf (stderr, "nullIndex = %d\n", nullBase + (nrCarriers + 1) / 2);
	p -> nullCarrier	= nullBase - Tu_t / 2 + (nrCarriers + 1) / 2;
	p -> freqOffset_integer = otherIndex - Tu_t / 2;
	fprintf (stderr, "int offset %d (%d)\n",
	                 otherIndex - Tu_t / 2, nullBase - Tu_t / 2);
	return (nrCarriers + 1) / 2 + p -> freqOffset_integer;
}

int16_t freqSyncer::getWord (std::complex<float> *buffer,
                             int32_t            bufSize,
                             int32_t            theIndex,
                             float              offsetFractional) {

int	i;
std::complex<float> temp [Ts_t];
std::complex<float> angle	= std::complex<float> (0, 0);
int	bufMask	= myReader -> bufSize - 1;

	int f	= theIndex & bufMask;
	if (offsetFractional < 0) {
	   offsetFractional = 1 + offsetFractional;
	   f -= 1;
	}

	for (i = 0; i < Ts_t; i ++) {
	   std::complex<float> one = buffer [(f + i) & bufMask];
	   std::complex<float> two = buffer [(f + i + 1) & bufMask];
	   temp [i] = cmul (one, 1 - offsetFractional) +
	              cmul (two, offsetFractional);
	}
	
	for (i = 0; i < Tg_t; i ++)
	   angle += conj (temp [Tu_t + i]) * temp [i];

	theAngle	= 0.9 * theAngle + 0.1 * arg (angle);

	int offset_in_Hz = theAngle / (2 * M_PI) * SAMPLE_RATE / Tu_t;

	for (i = 0; i < Ts_t; i ++) {
	   temp [i] *= freqVector [offsetIndex];
	   offsetIndex += offset_in_Hz;
	   if (offsetIndex < 0)
	      offsetIndex += SAMPLE_RATE;
	   if (offsetIndex >= SAMPLE_RATE)
	      offsetIndex -= SAMPLE_RATE;
	}

	for (i = Tg_t; i < Ts_t; i ++)
	   wordVector [i - Tg_t] = temp [i];

	fftwf_execute (plan);
	for (i = 0; i < Tu_t / 2; i ++) {
	   std::complex<float> w = wordVector [i];
	   wordVector [i] = wordVector [i + Tu_t / 2];
	   wordVector [i + Tu_t / 2] = w;
	}
	return 0;
}

