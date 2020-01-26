#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
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

#include	<stdlib.h>
#include	<stdio.h>
#include	"rds-decoder.h"
#include	"fm-decoder.h"
#include	"iir-filters.h"
#include	"sincos.h"

const float	RDS_BITCLK_HZ =	1187.5;
/*
 *	RDS is a bpsk-like signal, with a baudrate 1187.5
 *	on a carrier of  3 * 19 k.
 *	48 cycles per bit, 1187.5 bits per second.
 *	With a reduced sample rate of 48K this would mean
 *	48000 / 1187.5 samples per bit, i.e. between 40 and 41
 *	samples per bit.
 *	Notice that mixing to zero IF has been done
 */
	rdsDecoder::rdsDecoder (fmDecoder	*myRadio,
				int32_t		rate,
				SinCos		*mySinCos) {
float	synchronizerSamples;
int16_t	i;
int16_t	length;

	this	-> myDecoder	= myRadio;
	this	-> sampleRate	= rate;
	(void)mySinCos;
	this	-> mySinCos	= new SinCos (rate);
	omegaRDS		= (2 * M_PI * RDS_BITCLK_HZ) / (float)rate;
//
//	for the decoder a la FMStack we need:
	synchronizerSamples	= sampleRate / (float)RDS_BITCLK_HZ;
	symbolCeiling		= ceil (synchronizerSamples);
	symbolFloor		= floor (synchronizerSamples);
	syncBuffer		= new float [symbolCeiling];
	memset (syncBuffer, 0, symbolCeiling * sizeof (float));
	p			= 0;
	bitIntegrator		= 0;
	bitClkPhase		= 0;
	prev_clkState		= 0;
	prevBit			= 0;
	Resync			= true;
//
//	The matched filter is a borrowed from the cuteRDS, who in turn
//	borrowed it from course material 
//      http://courses.engr.illinois.edu/ece463/Projects/RBDS/RBDS_project.doc
//	Note that the formula down has a discontinuity for
//	two values of x, we better make the symbollength odd

	length			= (symbolCeiling & ~01) + 1;
	rdsfilterSize		= 2 * length + 1;
	rdsBuffer		= new float [rdsfilterSize];
	memset (rdsBuffer, 0, rdsfilterSize * sizeof (float));
	ip			= 0;
	rdsKernel		= new float [rdsfilterSize];
	rdsKernel [length] = 0;
	for (i = 1; i <= length; i ++) {
	   float x = ((float)i) / rate * RDS_BITCLK_HZ;
	   rdsKernel [length + i] =  0.75 * cos (4 * M_PI * x) *
					    ((1.0 / (1.0 / x - 64 * x)) -
					    ((1.0 / (9.0 / x - 64 * x))) );
	   rdsKernel [length - i] = - 0.75 * cos (4 * M_PI * x) *
					    ((1.0 / (1.0 / x - 64 * x)) -
					    ((1.0 / (9.0 / x - 64 * x))) );
	}
//
//	The matched filter is followed by a pretty sharp filter
//	to eliminate all remaining "noise".
	sharpFilter		= new BandPassIIR (9, RDS_BITCLK_HZ - 6,
	                                              RDS_BITCLK_HZ + 6,
	                                              rate, S_BUTTERWORTH);
	rdsLastSyncSlope	= 0;
	rdsLastSync		= 0;
	rdsLastData		= 0;
	previousBit		= false;

	my_rdsGroup		= new RDSGroup		();
	my_rdsGroup		->  clear ();
	my_rdsBlockSync		= new rdsBlockSynchronizer (myDecoder);
	my_rdsBlockSync		-> setFecEnabled (true);
	my_rdsGroupDecoder	= new rdsGroupDecoder	(myDecoder);

	connect (this, SIGNAL (setCRCErrors (int)),
		 myDecoder, SLOT (setCRCErrors (int)));
	connect (this, SIGNAL (setSyncErrors (int)),
		 myDecoder, SLOT (setSyncErrors (int)));
}

	rdsDecoder::~rdsDecoder (void) {
	delete[]	syncBuffer;
	delete		my_rdsGroupDecoder;
	delete		my_rdsGroup;
	delete		my_rdsBlockSync;
	delete		rdsKernel;
	delete		rdsBuffer;
	delete		sharpFilter;
}

void	rdsDecoder::reset	(void) {
	my_rdsGroupDecoder	-> reset ();
}

float	rdsDecoder::Match	(float v) {
int16_t		i;
float	tmp = 0;

	rdsBuffer [ip] = v;
	for (i = 0; i < rdsfilterSize; i ++) {
	   int16_t index = (ip - i);
	   if (index < 0)
	      index += rdsfilterSize;
	   tmp += rdsBuffer [index] * rdsKernel [i];
	}

	ip = (ip + 1) % rdsfilterSize;
	return tmp;
}
/*
 *	Signal (i.e. "v") is already downconverted and lowpass filtered
 *	when entering this stage. The return value stored in "*m" is used
 *	to display things to the user
 */
void	rdsDecoder::doDecode (float v, float *m, RdsMode mode) {
	if (mode == NO_RDS)
	   return;		// should not happen

	if (mode == RDS1) 
	   doDecode1 (v, m);
	else
	   doDecode2 (v, m);
}

void	rdsDecoder::doDecode1 (float v, float *m) {
float	rdsMag;
float	rdsSlope	= 0;
bool		bit;

	v	= Match (v);
	rdsMag	= sharpFilter	-> Pass (v * v);
	*m	= (20 * rdsMag + 1.0);
	rdsSlope	= rdsMag - rdsLastSync;
	rdsLastSync	= rdsMag;
	if ((rdsSlope < 0.0) && (rdsLastSyncSlope >= 0.0)) {
//	top of the sine wave: get the data
	   bit = rdsLastData >= 0;
	   processBit (bit ^ previousBit);
	   previousBit = bit;
	}

	rdsLastData = v;
	rdsLastSyncSlope	= rdsSlope;
	my_rdsBlockSync -> resetResyncErrorCounter ();
}

void	rdsDecoder::doDecode2	(float v, float *mag) {
float clkState;

	syncBuffer [p]	= v;
	*mag		= syncBuffer [p] + 1;
	p		= (p + 1) % symbolCeiling;
	v		= syncBuffer [p];	// an old one

	if (Resync ||
	   (my_rdsBlockSync -> getNumSyncErrors () > 3)) {
	   synchronizeOnBitClk (syncBuffer, p);
	   my_rdsBlockSync -> resync ();
	   my_rdsBlockSync -> resetResyncErrorCounter ();
	   Resync = false;
	}

	clkState	= mySinCos -> getSin (bitClkPhase);
	bitIntegrator	+= v * clkState;
//
//	rising edge -> look at integrator
	if (prev_clkState <= 0 && clkState > 0) {
	   bool currentBit = bitIntegrator >= 0;
	   processBit (currentBit ^ previousBit);
	   bitIntegrator = 0;		// we start all over
	   previousBit   = currentBit;
	}

	prev_clkState	= clkState;
	bitClkPhase	= fmod (bitClkPhase + omegaRDS, 2 * M_PI);
}

void	rdsDecoder::processBit (bool bit) {
	switch (my_rdsBlockSync -> pushBit (bit, my_rdsGroup)) {
	   case rdsBlockSynchronizer::RDS_WAITING_FOR_BLOCK_A:
	      break;		// still waiting in block A

	   case rdsBlockSynchronizer::RDS_BUFFERING:
	      break;	// just buffer

	   case rdsBlockSynchronizer::RDS_NO_SYNC:
//	      resync if the last sync failed
	      setSyncErrors (my_rdsBlockSync -> getNumSyncErrors ());
	      my_rdsBlockSync -> resync ();
	      break;

	   case rdsBlockSynchronizer::RDS_NO_CRC:
	      setCRCErrors (my_rdsBlockSync -> getNumCRCErrors ());
	      my_rdsBlockSync -> resync ();
	      break;

	   case rdsBlockSynchronizer::RDS_COMPLETE_GROUP:
	      if (!my_rdsGroupDecoder -> decode (my_rdsGroup)) {
	          ;	// error decoding the rds group
	      }

	      my_rdsGroup -> clear ();
	      break;
	}
}

void	rdsDecoder::synchronizeOnBitClk (float *v, int16_t first) {
bool	isHigh	= false;
int32_t	k = 0;
int32_t	i;
float	phase;
float *correlationVector =
	(float *)alloca (symbolCeiling * sizeof (float));

	memset (correlationVector, 0, symbolCeiling * sizeof (float));

//	synchronizerSamples	= sampleRate / (float)RDS_BITCLK_HZ;
	for (i = 0; i < symbolCeiling; i ++) {
	   phase = fmod (i * (omegaRDS / 2), 2 * M_PI);
//	reset index on phase change
	   if (mySinCos -> getSin (phase) > 0 && !isHigh) {
	      isHigh = true;
	      k = 0;
	   }
	   else
	   if (mySinCos -> getSin (phase) < 0 && isHigh) {
	      isHigh = false;
	      k = 0;
	   }

	   correlationVector [k ++] += v [(first + i) % symbolCeiling];
	}

//	detect rising edge in correlation window
	int32_t iMin	= 0;
	while (iMin < symbolFloor && correlationVector [iMin ++] > 0);
	while (iMin < symbolFloor && correlationVector [iMin ++] < 0);

//	set the phase, previous sample (iMin - 1) is obviously the one
	bitClkPhase = fmod (-omegaRDS * (iMin - 1), 2 * M_PI);
	while (bitClkPhase < 0) bitClkPhase += 2 * M_PI;
	fprintf (stderr, "%f \n", bitClkPhase);
}

