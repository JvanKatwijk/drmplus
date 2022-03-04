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

#include	"qam4-handler.h"
#include	"puncture-tables.h"
#include	"viterbi-drm.h"
#include	"prbs.h"

	qam4_handler::qam4_handler	(drmParameters *params,
	                                 int	muxLength,
	                                 int	lengthA,
	                                 int	lengthB):
	                                    myDecoder (),
	                                    myTables () {
	this	-> params	= params;
	this	-> muxLength	= muxLength;
	this	-> lengthA	= lengthA;
	this	-> lengthB	= lengthB;

//	for now
	map_protLevel (params -> protLevelA, &Rx_high, &Ry_high);
	map_protLevel (params -> protLevelB, &Rx_low, &Ry_low);
	double	Rp0	= (double)Rx_high / Ry_high;
	double	Rp1	= (double)Rx_low / Ry_low;
	float RYlcm	= getRYlcm_4 (params -> protLevelA);
	float denom	= Rp0;
	denom		*= 2 * RYlcm;

#if 1
	fprintf (stderr, "lengthA = %d, lengthB = %d, denom = %f, RYlcm = %f\n",
	                     lengthA, lengthB, denom, RYlcm);
#endif
	N1		= int16_t (ceil (lengthA / denom)) * RYlcm;
	N2		= muxLength - N1;
#if 1
	fprintf (stderr, "protLevels %d, %d\n",
	                 params -> protLevelA, params -> protLevelB);
	fprintf (stderr, "high protection %d %d, low %d %d\n",
	                  Rx_high, Ry_high, Rx_low, Ry_low);
	fprintf (stderr, "Cells A %d, cellsB %d\n",
	                                N1, N2);
#endif
	puncture_high	= myTables. getPunctureTable (Rx_low,  Ry_low);
	puncture_low	= myTables. getPunctureTable (Rx_low,  Ry_low);
//
//      The residu tables relate to the Lower protected part
        residuTable	= myTables. getResiduTable   (Rx_low, Ry_low, N2);
	nrBits_high	= Rx_high * (2 * N1) / Ry_high;
	nrBits_low	= Rx_low  * (2 * N2 - 12) / Ry_low;

	fprintf (stderr, "nrBits_high %d, low %d\n",
	                         nrBits_high, nrBits_low);
	Y21Mapper_high		= new Mapper (2 * N1, 21);
	Y21Mapper_low		= new Mapper (2 * N2, 21);
	deconvolver		= new  viterbi_drm (lengthA + lengthB);
	thePRBS			= new prbs	   (lengthA + lengthB);
}

	qam4_handler::~qam4_handler	() {
	delete Y21Mapper_high;
	delete Y21Mapper_low;
	delete deconvolver;
	delete thePRBS;
}
//
void	qam4_handler::process		(theSignal *mux, uint8_t *out) {
int deconvolveLength	= 6 * (lengthA + lengthB + 6);
metrics	deconvolveBuffer [deconvolveLength];
metrics	softBits	[2 * muxLength];
metrics	hulpVec		[2 * muxLength];
int	Teller	= 0;

	myDecoder. computemetrics (mux, muxLength, softBits);
	for (int i = 0; i < 2 * N1; i ++)
	   hulpVec [Y21Mapper_high -> mapIn (i)] = softBits [i];
	for (int i = 0; i < 2 * N2; i ++)
	   hulpVec [2 * N1 + Y21Mapper_low -> mapIn (i)] =
	                              softBits [2 * N1 + i];

	int deconvolveIndex = 0;
	for (int i = 0; i < 6 * lengthA; i ++) {
	   if (puncture_high [i % (6 * Rx_high)] == 1) 
	      deconvolveBuffer [deconvolveIndex] = hulpVec [Teller ++];
	   else {
	      deconvolveBuffer [deconvolveIndex]. rTow0 = 0;
	      deconvolveBuffer [deconvolveIndex]. rTow1 = 0;
	   }
	   deconvolveIndex ++;
	}

	for (int i = 0; i < 6 * lengthB; i ++) {
	   if (puncture_low [i % (6 * Rx_low)] == 1) 
	      deconvolveBuffer [deconvolveIndex] = hulpVec [Teller ++];
	   else {
	      deconvolveBuffer [deconvolveIndex]. rTow0 = 0;
	      deconvolveBuffer [deconvolveIndex]. rTow1 = 0;
	   }
	   deconvolveIndex ++;
	}

	for (int i = 0; i < 36; i ++) {
	   if (residuTable [i] == 1) 
	      deconvolveBuffer [deconvolveIndex] = hulpVec [Teller ++];
	   else {
	      deconvolveBuffer [deconvolveIndex]. rTow0 = 0;
	      deconvolveBuffer [deconvolveIndex]. rTow1 = 0;
	   }
	   deconvolveIndex ++;
	}

	deconvolver	-> deconvolve (deconvolveBuffer,
	                              lengthA + lengthB, out);
	thePRBS		-> doPRBS (out);
}

void	qam4_handler::map_protLevel (int protLevel, int* Rx, int* Ry) {
	switch (protLevel) {
	   default:
	   case 0:
	      *Rx = 1; *Ry = 4;
	      break;
	   case 1:
	      *Rx = 1; *Ry = 3;
	      break;
	   case 2:
	      *Rx = 2; *Ry = 5;
	      break;
	   case 3:
	      *Rx = 1; *Ry = 2;
	}
}

int16_t qam4_handler::getRYlcm_4     (int16_t protLevel) {
int Rx, Ry;
	map_protLevel (protLevel, &Rx, &Ry);
	if ((Rx == 1) && (Ry == 4))
	   return 4;
	if ((Rx == 1) && (Ry == 3))
	   return 3;
	if ((Rx == 2) && (Ry == 5))
	   return 5;
	if ((Rx == 1) && (Ry == 2))
	   return 2;
	return 4;
}


