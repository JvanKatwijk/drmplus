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
//
#include	"qam16-handler.h"
#include        "msc-streamer.h"
#include        "mapper.h"
#include        "basics.h"
#include        "prbs.h"

//
//	Implements table 31 (page 111);

static
void	protLevel	(uint8_t prot, int16_t nr,
	                                 int16_t *RX, int16_t *RY) {

	switch (prot){
	   case 0:
	      if (nr == 0) {
	         *RX	= 1; *RY = 6;
	      }
	      else {		// nr better be '1'
	         *RX = 1; *RY = 2;
	      }
	      break;

	   default:		// should not happen
	   case 1:
	      if (nr == 0) {
	         *RX = 1; *RY = 4;
	      }
	      else {	// nr better be '1'
	         *RX = 4; *RY = 7;
	      }
	      break;

	   case 2:
	      if (nr == 0) {
	         *RX = 1; *RY = 3;
	      }
	      else {
	         *RX = 2; *RY = 3;
	      }
	      break;

	   case 3:
	      if (nr == 0) {
	         *RX = 1; *RY = 2;
	      }
	      else {
	         *RX = 3; *RY = 4;
	      }
	      break;
	}
}

static
float	getRp	(uint8_t prot, int16_t nr) {
int16_t	RX, RY;

	protLevel (prot, nr, &RX, &RY);
	return (float (RX))/RY;
}

static
int16_t	getRYlcm_16	(int16_t protLevel) {
	return protLevel == 0 ? 6 :
	       protLevel == 1 ? 28 :
	       protLevel == 2 ? 3 : 4;
}

	qam16_handler::qam16_handler	(drmParameters *params,
	                                 int	muxLength,
	                                 int	lengthA,
	                                 int	lengthB) {
int16_t	RYlcm, i;
float denom;
//
//      apply the formula from page 112 (section 7.2.1.1) to determine
//      the number of QAM cells in the A part, then the number of QAM cells
//      in the lower protected part (the B part) follows

	this    -> params		= params;
	this	-> muxLength		= muxLength;
	this	-> lengthA		= lengthA;
	this	-> lengthB		= lengthB;

	if (lengthA != 0) {     // two real levels
//      apply formula from section 7.2.1. to compute the number
//      of MSC cells for the higher protected part given in bytes
	   RYlcm        = getRYlcm_16 (params -> protLevelA);
	   denom        = 0;
	   for (i = 0; i < 2; i ++)
	      denom += getRp (params -> protLevelA, i);
	   denom        *= 2 * RYlcm;
//	   N1           = int16_t (ceil (8.0 * lengthA / denom) * RYlcm);
	   N1           = int16_t (ceil (lengthA / denom) * RYlcm);
//         fprintf (stderr, "N1 = %d (lengthA = %d)\n", N1, lengthA);
	   N2           	= muxLength - N1;
	   Y13mapper_high       = new Mapper (2 * N1, 13);
	   Y21mapper_high       = new Mapper (2 * N1, 21);
	   Y13mapper_low        = new Mapper (2 * N2, 13);
	   Y21mapper_low        = new Mapper (2 * N2, 21);
	}
	else {
	   N1			= 0;
	   N2			= muxLength;
	   Y13mapper_high	= nullptr;
	   Y21mapper_high	= nullptr;
	   Y13mapper_low        = new Mapper (2 * N2, 13);
	   Y21mapper_low        = new Mapper (2 * N2, 21);
	}

//      we need two streamers:
//      The N1 indicates the number of OFDM cells for the
//      higher protected bits, N2 follows directly

	stream_0        = new MSC_streamer (params, 0, N1,
	                                    Y13mapper_high, Y13mapper_low);
	stream_1        = new MSC_streamer (params, 1, N1,
	                                    Y21mapper_high, Y21mapper_low);
	thePRBS         = new prbs (stream_0 -> highBits () +
	                            stream_1 -> highBits () +
	                            stream_0 -> lowBits  () +
	                            stream_1 -> lowBits  ());
}

	qam16_handler::~qam16_handler	() {
	delete  stream_0;
	delete  stream_1;
	if (Y13mapper_high != nullptr)
	   delete Y13mapper_high;
	if (Y21mapper_high != nullptr)
	   delete Y21mapper_high;
	delete  Y13mapper_low;
	delete  Y21mapper_low;
	delete  thePRBS;
}

void	qam16_handler::process		(theSignal *mux, uint8_t *bitsOut) {
int16_t highProtectedBits       = stream_0 -> highBits () +
	                          stream_1 -> highBits ();
int16_t lowProtectedBits        = stream_0 -> lowBits () +
	                          stream_1 -> lowBits ();
uint8_t *bits_0  = new uint8_t [stream_0 -> highBits () + stream_0 -> lowBits () + 10];
uint8_t *bits_1  = new uint8_t [stream_1 -> highBits () + stream_1 -> lowBits () + 10];

metrics *Y0	= new metrics [2 * muxLength];
metrics *Y1	= new metrics [2 * muxLength];
uint8_t *level_0  = new uint8_t [2 * muxLength + 10];
uint8_t *level_1  = new uint8_t [2 * muxLength + 10];

//	fprintf (stderr, "muxLength = %d\n", muxLength);
	for (int i = 0; i < 4; i ++) {
	   myDecoder. computemetrics (mux, muxLength, 0, Y0,
	                                      i > 0, level_0, level_1);
//	   fprintf (stderr, "Y0 = %d, bits_0 = %d, level_0 = %d\n",
//	                        2 * muxLength, stream_0 -> highBits () +
//	                                       stream_0 -> lowBits (),
//	                                        muxLength);
	   stream_0        -> process      (Y0, bits_0, level_0);
	   myDecoder. computemetrics (mux, muxLength, 1, Y1,
	                                      i > 0, level_0, level_1);
	   stream_1        -> process      (Y1, bits_1, level_1);
	}

	memcpy (&bitsOut [0],
	        &bits_0 [0],
	        stream_0 -> highBits ());
	memcpy (&bitsOut [stream_0 -> highBits ()],
	        &bits_1 [0],
	        stream_1 -> highBits ());
	memcpy (&bitsOut [stream_0 -> highBits () +
	                  stream_1 -> highBits ()],
	        &bits_0 [stream_0 -> highBits ()],
	        stream_0 -> lowBits ());
	memcpy (&bitsOut [stream_0 -> highBits () +
	                  stream_1 -> highBits () +
	                  stream_0 -> lowBits ()],
	        &bits_1 [stream_0 -> highBits ()],
	        stream_1 -> lowBits ());
//	apply PRBS
	thePRBS -> doPRBS (bitsOut);
	delete[] Y0;
	delete[] Y1;
	delete[] level_0;
	delete[] level_1;
}

