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

#ifndef	__TIME_CORRELATOR__
#define	__TIME_CORRELATOR__

#include	<stdint.h>
#include	<complex>
#include	"basics.h"

class timeCorrelator {
public:
		timeCorrelator	();
		~timeCorrelator	();
	bool	is_bestIndex	(int symbol);
	void	getCorr		(int, std::complex<float> *);
	int	get_bestIndex	(float *);
private:
	float	corrBank [symbolsperFrame];
};

#endif

