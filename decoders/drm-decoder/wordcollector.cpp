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

#include	"wordcollector.h"
#include	"reader.h"
#include	"radio-constants.h"

	wordCollector::wordCollector	(theReader *myReader) {
	this	-> myReader	= myReader;
	wordVector      = (std::complex<float> *)
                               fftwf_malloc (sizeof (fftwf_complex) * Tu_t);

        plan    	=
	    fftwf_plan_dft_1d (Tu_t,
                               reinterpret_cast <fftwf_complex *>(wordVector),
                               reinterpret_cast <fftwf_complex *>(wordVector),
                               FFTW_FORWARD, FFTW_ESTIMATE);
        freqVector      = new std::complex<float> [SAMPLE_RATE];
        for (int i = 0; i < SAMPLE_RATE; i ++)
           freqVector [i] =
	          std::complex<float> (cos (2 * M_PI * i / SAMPLE_RATE),
                                       sin (2 * M_PI * i / SAMPLE_RATE));
	offsetIndex	= 0;
        theAngle        = 0;
	sampleOffset	= 0;
	avgRateOffset	= 0;
}

	wordCollector::~wordCollector	() {
        fftwf_destroy_plan      (plan);
        fftwf_free              (wordVector);
        delete freqVector;
}

void	wordCollector::reset	() {
	theAngle = 0;
}

float	wordCollector::getWord (std::complex<float> *out,
	                        drmParameters *p, int offset) {
std::complex<float> temp [Ts_t];
int	i;
std::complex<float> angle	= std::complex<float> (0, 0);
int     base			= myReader -> currentIndex;
int	bufMask			= myReader -> bufMask;
int16_t	d = 0;
float	timeOffsetFractional;

	myReader	-> waitfor (Ts_t + 10);

//	correction of the time offset by interpolation
	float timeDelay	= p -> timeOffset_fractional;
	if (timeDelay < 0)
	   timeDelay = 0;
	d	= floor (timeDelay + 0.5);
	timeOffsetFractional	= timeDelay - d;

//      keep it simple, just linear interpolation
        int f = (myReader -> currentIndex + d) & bufMask;

        if (sampleOffset < 0) {
           sampleOffset = 1 + sampleOffset;
           f -= 1;
        }

//	correction of the time offset by interpolation
        for (i = 0; i < Ts_t; i ++) {
           std::complex<float> one = myReader -> data [(f + i) & bufMask];
           std::complex<float> two = myReader -> data [(f + i + 1) & bufMask];
           temp [i] = cmul (one, 1 - timeOffsetFractional) +
                                    cmul (two, timeOffsetFractional);
        }

	myReader -> currentIndex = (f + Ts_t) & bufMask;

	for (i = 0; i < Tg_t; i ++)
	   angle += conj (temp [Tu_t + i]) * temp [i];

	if (std::isinf (arg (angle)) || std::isnan (arg (angle)))
	   fprintf (stderr, "XX");
	else
	   theAngle	= 0.9 * theAngle + 0.1 * arg (angle);

	int offset_in_Hz = theAngle / (2 * M_PI) * SAMPLE_RATE / Tu_t;
	offset_in_Hz -= p -> freqOffset_integer * SAMPLE_RATE / Tu_t;
	for (i = 0; i < Ts_t; i ++) {
	   if (offsetIndex < 0)
	      offsetIndex += SAMPLE_RATE;
	   if (offsetIndex >= SAMPLE_RATE)
	      offsetIndex -= SAMPLE_RATE;
	   temp [i] *= freqVector [offsetIndex];
	   offsetIndex += offset_in_Hz;
	}

	for (i = Tg_t; i < Ts_t; i ++)
	   wordVector [i - Tg_t] = temp [i];

	fftwf_execute (plan);
	memcpy (out, &wordVector [Tu_t + K_min],
	       - K_min * sizeof (std::complex<float>));
	memcpy (&out [nrCarriers - K_max - 1], &wordVector [0],
	             (K_max + 1) * sizeof (std::complex<float>));
	return theAngle;
}

