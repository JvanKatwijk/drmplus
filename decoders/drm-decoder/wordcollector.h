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


#ifndef __WORD_COLLECTOR__
#define __WORD_COLLECTOR__

#include        "basics.h"
#include        <stdio.h>
#include        <stdint.h>
#include        <fftw3.h>
#include        <complex>

class   theReader;

class   wordCollector {
public:

        wordCollector		(theReader *);
        ~wordCollector		();
float	getWord			(drmParameters * params,
	                         std::complex<float> *, std::complex<float> *);
int	fine_timeSync (drmParameters *p, std::complex<float> *buffer);

void	reset			();
private:
        theReader       *myReader;
        fftwf_plan      plan;
        std::complex<float> *fftVector;
        std::complex<float> *freqVector;
	int32_t		phasePointer;
	float		theAngle;
	int32_t		actualBase;
};

#endif

