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

#include	"timesyncer.h"

		timeSyncer::timeSyncer (theReader *myReader,
	                                           int nSymbols) {
	this	-> myReader	= myReader;
	this	-> nSymbols	= nSymbols;
	this	-> nSamples	= nSymbols * Ts_t;
	this	-> mask		= myReader -> bufSize - 1;
	summedCorrelations 	= new std::complex<float>[nSamples - Tu_t];
	summedSquares 		= new float [nSamples - Tu_t];
	EPSILON			= 1.0E-10;
}

		timeSyncer::~timeSyncer () {
	delete summedCorrelations;
	delete summedSquares;
}

void	timeSyncer::computeGamma (drmParameters *params,
	                          float		*gammaRelative,
	                          float		*Epsilon,
	                          int16_t	*Offsets) {
std::complex<float> gamma [Ts_t];   // large enough
float   squareTerm	[Ts_t];
int32_t i, k, theOffset;

	for (i = 0; i < Ts_t; i ++) {
	   gamma [i]		= std::complex<float> (0, 0);
	   squareTerm [i]	= 0;
	   int32_t base		= myReader -> currentIndex + i;
	   for (int j = 0; j < nSymbols - 1; j ++) {
	      for (k = 0; k < Tg_t; k ++) {
                 std::complex<float> f1 =
                       myReader -> data [(base + j * Ts_t + k     ) & mask];
                 std::complex<float> f2 =
                       myReader -> data [(base + j * Ts_t + Tu_t + k) & mask];
                 gamma [i]      += f1 * conj (f2);
                 squareTerm [i] += real (f1 * conj (f1)) +
	                                    real (f2 * conj (f2));
              }
           }
	}

        theOffset               = 0;
        float minValue          = 1000000.0;
        for (i = 0; i < Ts_t; i ++) {
	   float mmse = abs (squareTerm [i] - 2 * abs (gamma [i]));
           if (mmse < minValue) {
              minValue = mmse;
              theOffset = i;
           }
        }

        *gammaRelative  =
	             2 * abs (gamma [theOffset]) / squareTerm [theOffset];
        *Epsilon	= (float) arg (gamma [theOffset]);
	*Offsets	= theOffset;
}

void	timeSyncer::compute_b	(drmParameters	*params,
	                         int16_t	*b,
	                         float		averageOffset) {
std::complex<float> gamma [Ts_t];
float	   squareTerm [Ts_t];
int32_t i, j;

	int32_t	base	= myReader	-> currentIndex;
	for (i = 0; i < nSamples - Tu_t; i ++) {
	   summedCorrelations [i] = std::complex<float> (0, 0);
	   summedSquares [i]	  = 0;
	   for (j = 0; j < Tg_t; j ++) {
	      std::complex<float> f1        =
	                      myReader -> data [(base + i + j) & mask];
              std::complex<float> f2        =
	                      myReader -> data [(base + i + Tu_t + j) & mask];
	      summedCorrelations [i] += f1 * conj (f2);
	      summedSquares	 [i] += real (f1 * conj (f1)) +
	                                     real (f2 * conj (f2));
	   }
	}

//	OK, the terms are computed, now find the minimum
	int16_t index	= Tg_t + averageOffset + Ts_t / 2;

	fprintf (stderr, "index %d\n", index);
	for (j = 0; j < nSymbols - 2; j++) {
	   float minValue	= 1000000.0;
	   for (i = 0; i < Ts_t; i++) {
	      gamma [i]		= summedCorrelations [(index + i)];
              squareTerm [i]	= summedSquares [index + i];
	      float mmse = squareTerm [i] - 2 * abs (gamma [i]);
	      if (mmse < minValue) {
	         minValue = mmse;
	         b [j] = i;
	      }
	   }
	   index += Ts_t;
	}
}
//
//	Formula 5.16 from Tsai reads
//      Phi (m) = sum (r=0, R-1, abs (z[m - r] ^ 2)) +
//                sum (r=0, R-1, abs (z[m - r - L]) ^ 2) -
//             2 * abs (sum (r=0, R-1, z [m - r] * conj (z [m - r - L])));
//      The start of the symbol is there where
//      Phi (m) is minimal

//	since we "find" a starting point, regardless what datastream
//	we handle, we need some threshold
bool	timeSyncer::dosync (drmParameters *params) {
int16_t	b [nSymbols];
int	nSamples	= nSymbols * Ts_t;
float	gammaRelative	= 0;
float	epsilon		= 0;
int16_t	offsets		= 0;

	myReader	-> waitfor (nSamples + 100);
	computeGamma (params, &gammaRelative, &epsilon, &offsets);
	if (gammaRelative < 0.85)
	   return false;

	compute_b (params, b, offsets);

//	Now least squares to 0...symbols_to_check and b [0] .. */
	float	sumx	= 0.0;
	float	sumy	= 0.0;
	float	sumxx	= 0.0;
	float	sumxy	= 0.0;

	for (int i = 0; i < nSymbols - 2; i++) {
	   sumx	+= (float) i;
	   sumy += (float) b [i];
	   sumxx += (float) i * (float) i;
	   sumxy += (float) i * (float) b [i];
	}

	float	slope, boffs;
	slope = (float) (((nSymbols - 2) * sumxy - sumx * sumy) /
	                 ((nSymbols - 2) * sumxx - sumx * sumx));
	boffs = (float) ((sumy * sumxx - sumx * sumxy) /
	                 ((nSymbols - 2) * sumxx - sumx * sumx));

//	OK, all set, we are done

	params -> timeOffset	= fmodf (
	                          (Tg_t + Ts_t /  2 +
	                                offsets + boffs - 1),
	                                            (float)Ts_t);
	fprintf (stderr, "nu: %f\n", params -> timeOffset);
	params -> timeOffset = offsets;
	params -> sampleRate_offset = slope / (float) Ts_t;
	params -> freqOffset_fract = epsilon;
	return true;
}

