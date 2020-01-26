#
/*
 *    Copyright (C) 2020
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
 *
 *	This pll was found to give reasonable results.
 *	source DTTSP, all rights acknowledged
 *
 *	This is the version for complex values
 */
#include	"pllC.h"
//
//	rate	is the samplerate
//	lofreq and hifreq the frequencies (in Hz) where the lock is 
//	kept in between,
//	bandwidth the bandwidth of the signal to be received

		pllC::pllC (int32_t	rate,
	                    float	freq,
	                    float	lofreq, float hifreq,
	                    float	bandwidth,
	                    SinCos	*Table) {
float	fac	= 2.0 * M_PI / rate;

	this	-> rate	= rate;
	this	-> cf	= freq;
//	for the control lowpass filter
	Beta		= exp (- 2.0 * M_PI * bandwidth / 2 / rate);
	NcoPhase	= 0;
	phaseError	= 0;
	phaseIncr	= freq * fac ;		// this will change during runs
	NcoLLimit	= lofreq * fac;		// boundary for changes
	NcoHLimit	= hifreq * fac;

	this	-> mySinCos	= Table;

	oldNcoSignal	= 0;
	pll_lock	= false;
}
//
		pllC::~pllC (void) {
}
//
//	It turned out that under Fedora we had from time
//	to time an infinite value for signal. Still have
//	to constrain this value
void		pllC::do_pll (std::complex<float> signal) {
std::complex<float>	NcoSignal;

	NcoSignal = (mySinCos != NULL) ?
	                  mySinCos -> getComplex (NcoPhase) : 
                          std::complex<float> (cos (NcoPhase), sin (NcoPhase));
	    
	pll_Delay	= NcoSignal * signal;
//
//	we use a pretty fast atan here
	phaseError	= - myAtan. atan2 (imag (pll_Delay), real (pll_Delay));
//	... and a pretty simple filter
	phaseIncr	= (1 - Beta) * phaseError + Beta * phaseIncr;
	if (phaseIncr < NcoLLimit || phaseIncr > NcoHLimit)
	   phaseIncr	= cf * 2 * M_PI / rate;

	NcoPhase	+= phaseIncr;
	if (NcoPhase >= 2 * M_PI)
	   NcoPhase = fmod (NcoPhase, 2 * M_PI);
	else
	while (NcoPhase < 0)
	   NcoPhase += 2 * M_PI;
}

std::complex<float>	pllC::getDelay (void) {
	return pll_Delay;
}

float	pllC::getPhaseIncr(void) {
	return phaseIncr;
}

float	pllC::getNco (void) {
	return NcoPhase;
}

float	pllC::getPhaseError (void) {
	return phaseError;
}

bool		pllC::isLocked	(void) {
	return pll_lock;
}

