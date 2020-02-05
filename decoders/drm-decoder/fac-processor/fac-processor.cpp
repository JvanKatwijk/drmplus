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

#include	"fac-processor.h"
#include	"drm-decoder.h"
#include	"referenceframe.h"

#define FAC_SAMPLES     244
#define FAC_BITS        (20 + 44 + 44)
#define FAC_CRC         8

//      for the FAC bits we have the following crc polynome
static
const uint16_t crcPolynome [] = {
        0, 0, 0, 1, 1, 1, 0     // MSB .. LSB x⁸ + x⁴ + x³ + x² + 1
};

//
	facProcessor::facProcessor (drmDecoder	*theRadio,
	                            drmParameters	*params,
	                            RingBuffer<std::complex<float>> *iqBuffer):
	                            myMapper    (2 * FAC_SAMPLES, 21),
	                            thePRBS     (FAC_BITS + FAC_CRC),
	                            theCRC      (8, crcPolynome),
	                            deconvolver (FAC_BITS + FAC_CRC) {
	this	-> theRadio	= theRadio;
	this	-> params	= params;
	this	-> iqBuffer	= iqBuffer;
	show_Constellation	= false;
	connect (this, SIGNAL (show_iq ()),
	         theRadio, SLOT (show_iq ()));
	connect (this, SIGNAL (showSNR (float)),
	         theRadio, SLOT (showSNR (float)));
}

	facProcessor::~facProcessor (void) {
}

//
//	The puncture table is the 1/4 table from the standard
//      it is defined as a series of 6 bits, the last two
//      being erased.
//      I.e. 4 bits in and 6 bits out
static
uint8_t punctureTable [] = {
1, 1, 1, 1, 0, 0
};
//
//	On the receiving side we therefore should do the reverse:
//	1. extract the FAC cells into a vector
//	2. 4QAM demodulation
//	3. bit-deinterleaving
//	4. depuncturing and convolutional decoding
//	5. dispersion
//	6. CRC checking, resulting in nice bits
//
int	key_to_number (int serviceKey) {
	switch (serviceKey) {
	   case 13:
	      return 4;
	   case 1:
	      return 1;
	   default:
	      return 1;
	}
}

static	int delayer = 0;
bool	facProcessor:: processFAC (theSignal **outbank,
	                           float meanEnergy, std::complex<float> **H) {
theSignal facVector	[FAC_SAMPLES];
std::complex<float> iqVector [FAC_SAMPLES];
uint8_t	 facBits	[2 * (FAC_BITS + FAC_CRC) + 4];
float	sqrdNoiseSeq	[FAC_SAMPLES];
float	sqrdWeightSeq	[FAC_SAMPLES];
float	sum_WMERFAC     = 0;
float   sum_weight_FAC  = 0;
float   WMERFAC;
int	i;

	for (i = 0; i < FAC_SAMPLES; i ++) {
	   int16_t symbol, carrier, index;
	   getFACcell (i, &symbol, &carrier);
	   index		= carrier - K_min;
	   facVector [i]	= outbank [symbol][index];
	   iqVector  [i]	= facVector [i]. signalValue;
	   sqrdNoiseSeq [i]	= abs (facVector [i]. signalValue) - sqrt (0.5);
           sqrdNoiseSeq [i]	*= sqrdNoiseSeq [i];
           sqrdWeightSeq [i]	= real (H [symbol][index] *
                                            conj (H [symbol][index]));
           sum_WMERFAC          += sqrdNoiseSeq [i] *
                                        (sqrdWeightSeq [i] + 1.0E-10);
           sum_weight_FAC       += sqrdWeightSeq [i];
	}
	WMERFAC	= -10 * log10 (sum_WMERFAC /
	                       (meanEnergy * (sum_weight_FAC + FAC_SAMPLES * 1.0E-10)));


	fromSamplestoBits (facVector, facBits);
//	first: dispersion
	thePRBS. doPRBS (facBits);
//
//	4 bits, position 108 .. 112, were included in the CRC computation
//	but not transmitted
	for (int i = 0; i < 8; i ++)
	   facBits [120 - 1 - i] = facBits [120 - 5 - i];
	for (int i = 108; i < 112; i ++)
	   facBits [i] = 0;
//	next CRC
	if (!theCRC. doCRC (facBits, 120)) {
	   return false;
	}
	
	channelData (facBits);
	serviceData (&facBits [20]);
	serviceData (&facBits [20 + 44]);
	if (++delayer > 4) {
	   if (show_Constellation) {
	      iqBuffer -> putDataIntoBuffer (iqVector, FAC_SAMPLES);
	      emit show_iq ();
	   }
	   delayer = 0;
	   emit showSNR (WMERFAC);
	}
	return true;
}

//	de-interleaving is done inline. The function's main concern is
//	preparation for the deconvolution
void	facProcessor::fromSamplestoBits (theSignal *v, uint8_t *outBuffer) {
metrics rawBits 	[2 * FAC_SAMPLES];
metrics demappedBits 	[2 * FAC_SAMPLES];
int16_t	bufferSize	= 6 * (FAC_BITS + FAC_CRC + 6);
metrics	deconvolveBuffer [bufferSize];
int16_t	deconvolveCnt   = 0;
int16_t	inputCnt	= 0;
int16_t	i;
//
	myMetrics. computemetrics (v, FAC_SAMPLES, rawBits);

//	The puncturing table is given, it is a 6 entry table
//	so coded in-line, together with de-interleaving
//
	for (i = 0; i < 2 * FAC_SAMPLES; i ++)
	   demappedBits [myMapper. mapIn (i)] = rawBits [i];

	while (deconvolveCnt < bufferSize) {
	   if (punctureTable [deconvolveCnt % 6] == 1) {
	      deconvolveBuffer [deconvolveCnt] = demappedBits [inputCnt ++];
	   }
	   else {	// add a zero costs for both
	      deconvolveBuffer [deconvolveCnt]. rTow0 = 0;
	      deconvolveBuffer [deconvolveCnt]. rTow1 = 0;
	   }

	   deconvolveCnt ++;
	}
//
//	This "viterbi" understands the defined metrics
	deconvolver. deconvolve (deconvolveBuffer,
	                             FAC_BITS + FAC_CRC, outBuffer);
}

void	facProcessor::channelData (uint8_t *facBits) {
	params	-> theChannel. Identity	= facBits [1] * 2 + facBits [2];
	params	-> theChannel. RMFlag	= facBits [3];
	params	-> theChannel. InterleaverDepthFlag = facBits [7];
	params	-> theChannel. MSC_Mode	= facBits [8] * 2 + facBits [9];

	params	-> theChannel. sdcMode	= facBits [10];
	params	-> theChannel. serviceEncoding =
	                          facBits [11] * 8 + facBits [12] * 4 +
	                          facBits [13] * 2 + facBits [14];
	params	-> theChannel. nrAudioServices	=
	                          (facBits [11] << 1) | (facBits [12]);
	params	-> theChannel. nrDataServices	=
	                          (facBits [13] << 1) | (facBits [14]);
	params	-> theChannel. toggleFlag = facBits [18];
}

void	facProcessor::serviceData (uint8_t *serviceBits) {
int shortId	= serviceBits [24] * 2 + serviceBits [25];
uint32_t serviceId	= 0;
	for (int i = 0; i < 24; i ++) {
	   serviceId <<= 1;
	   serviceId |= serviceBits [i];
	}
	params	-> subChannels [shortId]. serviceId = serviceId;
	params	-> subChannels [shortId]. shortId  = shortId;

	uint8_t language = 0;
	for (int i = 0; i < 4; i ++) {
	   language <<= 1;
	   language |= serviceBits [27 + i];
	}
	params	-> subChannels [shortId]. language = language;
	params	-> subChannels [shortId]. is_audioService =
	                                    serviceBits [31] == 0;

	uint8_t serviceDescriptor = 0;
	for (int i = 0; i < 5; i ++) {
	   serviceDescriptor <<= 1;
	   serviceDescriptor |= serviceBits [32 + i];
	}
	params -> subChannels [shortId]. serviceDescriptor = serviceDescriptor;
}

void	facProcessor::set_Viewer	(bool b) {
	show_Constellation	= b;
}

