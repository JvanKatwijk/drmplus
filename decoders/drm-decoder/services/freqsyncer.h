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

#ifndef	__FREQSYNCER__
#define	__FREQSYNCER__

#include	"basics.h"
#include	<stdio.h>
#include	<stdint.h>
#include        <fftw3.h>
#include	<complex>

class	theReader;

class	freqSyncer {
public:

	freqSyncer	(theReader *);
	~freqSyncer	();
int	sync		(drmParameters *);
private:
	theReader	*myReader;
	std::complex<float> *wordVector;
	fftwf_plan	plan;
	std::complex<float> *freqVector;
	int		offsetIndex;
	float		theAngle;
	int16_t         getWord		(std::complex<float> *,
                                         int32_t,
                                         int32_t,
                                         float);
};

#endif

