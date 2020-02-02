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
	fftVector      = (std::complex<float> *)
                               fftwf_malloc (sizeof (fftwf_complex) * Tu_t);

        plan    	=
	    fftwf_plan_dft_1d (Tu_t,
                               reinterpret_cast <fftwf_complex *>(fftVector),
                               reinterpret_cast <fftwf_complex *>(fftVector),
                               FFTW_FORWARD, FFTW_ESTIMATE);
        freqVector      = new std::complex<float> [SAMPLE_RATE];
        for (int i = 0; i < SAMPLE_RATE; i ++)
           freqVector [i] =
	          std::complex<float> (cos (2 * M_PI * i / SAMPLE_RATE),
                                       sin (2 * M_PI * i / SAMPLE_RATE));
	phasePointer	= 0;
        theAngle        = 0;
	actualBase	= 0;
}

	wordCollector::~wordCollector	() {
        fftwf_destroy_plan      (plan);
        fftwf_free		(fftVector);
        delete freqVector;
}

void	wordCollector::reset	() {
	theAngle	= 0;
	phasePointer	= 0;
	actualBase	= 0;
}

int	wordCollector::fine_timeSync (drmParameters *p,
	                              std::complex<float> *buffer) {
double mse, best_mse;
int	bestIndex	= -1;
double	squares		= 0;
std::complex<float> gamma	= std::complex<float> (0, 0);

	best_mse	= 1E50;
	for (int i = actualBase  - 8; i < actualBase + 8; i ++) {
	   mse	= 0;
	   gamma	= std::complex<float> (0, 0);
	   squares	= 0;
	   for (int symbols = 0; symbols < FINE_TIME_BLOCKS; symbols ++) {
	      for (int j = i; j < i + 48; j ++) {
	         int index = symbols * Ts_t + Ts_t / 2 + j;
	         std::complex<float> first	= buffer [index];
	         std::complex<float> second	= buffer [index + Tu_t];
	         squares	+= abs (first * conj (first)) +
	                           abs (second * conj (second));
	         gamma		+= first * conj (second);
	      }
	   }

	   mse = abs (squares - 2 * abs (gamma));
           if (mse < best_mse) {
              best_mse = mse;
              bestIndex = i;
	   }
	}
//
//	slow down a little on too fast a movement
	if (bestIndex - actualBase > 6)
	   actualBase ++;
	if (actualBase - bestIndex > 6)
	   actualBase --;
	return actualBase;
}

float	wordCollector::getWord (drmParameters *p,
	                        std::complex<float> *buffer,
	                        std::complex<float> *out) {
std::complex<float> temp [Ts_t];
int	i;
std::complex<float> angle	= std::complex<float> (0, 0);
int16_t	d = 0;
float	timeOffsetFractional;

	float timeDelay	= - p -> timeOffset_fractional;
//	correction of the time offset by interpolation
	if (timeDelay < -0.5) {
	   timeDelay ++;
	   d	= -1;
	   timeOffsetFractional = timeDelay;
	}
	else {
	   d	= floor (timeDelay + 0.5);
	   timeOffsetFractional	= timeDelay - d;
	}

//	correction of the time offset by interpolation
        for (i = 0; i < Ts_t; i ++) {
           std::complex<float> one = buffer [Ts_t / 2 + p -> timeOffset_integer + i];
           std::complex<float> two = buffer [Ts_t / 2 + p -> timeOffset_integer + i + 1];
           temp [i] = cmul (one, 1 - timeOffsetFractional) +
                                          cmul (two, timeOffsetFractional);
        }

	for (i = 0; i < Tg_t; i ++)
	   angle += conj (temp [Tu_t + i]) * temp [i];

	if (std::isinf (arg (angle)) || std::isnan (arg (angle)))
	   fprintf (stderr, "XX");
	else
	   theAngle	= 0.9 * theAngle + 0.1 * arg (angle);

	int offset_in_Hz = theAngle / (2 * M_PI) * SAMPLE_RATE / Tu_t;
	for (i = 0; i < Ts_t; i ++) {
	   if (phasePointer < 0)
	      phasePointer += SAMPLE_RATE;
	   if (phasePointer >= SAMPLE_RATE)
	      phasePointer -= SAMPLE_RATE;
	   temp [i] *= freqVector [phasePointer];
	   phasePointer += offset_in_Hz;
	}

	for (i = Tg_t; i < Ts_t; i ++)
	   fftVector [i - Tg_t] = temp [i];

	fftwf_execute (plan);
	memcpy (out, &fftVector [Tu_t + K_min],
	       - K_min * sizeof (std::complex<float>));
	memcpy (&out [nrCarriers - K_max - 1], &fftVector [0],
	             (K_max + 1) * sizeof (std::complex<float>));

	memmove (buffer, &buffer [Ts_t],
	             FINE_TIME_BLOCKS * Ts_t * sizeof (std::complex<float>));
	myReader -> waitfor (Ts_t);
	myReader -> read (&buffer [FINE_TIME_BLOCKS * Ts_t],
	                  Ts_t, p -> freqOffset_integer);
	return theAngle;
}

