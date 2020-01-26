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

#ifndef	__TIME_SYNCER__
#define	__TIME_SYNCER__

#include	"reader.h"
#include	<complex>
#include	"basics.h"
#include	<stdint.h>
#include	"referenceframe.h"

class timeSyncer {
public:
		timeSyncer	(theReader *, int);
		~timeSyncer	();
	bool	dosync		(drmParameters *);
private:
	float	EPSILON;
	std::complex<float> *summedCorrelations;
	float	*summedSquares;
	int	nSymbols;
	int	nSamples;
	int	mask;
	theReader *myReader;
	void	computeGamma	(drmParameters *, float *, float *, int16_t *);
	void	compute_b	(drmParameters *, int16_t *, float);
};

#endif

