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

#include	"time-correlator.h"
#include	"referenceframe.h"

	timeCorrelator::timeCorrelator () {
}

	timeCorrelator::~timeCorrelator () {}

bool	timeCorrelator::is_bestIndex (int lc) {
float	tester	= corrBank [lc];
float	sum = 0;
	for (int symbol = 0; symbol < symbolsperFrame; symbol ++)
	   sum += corrBank [symbol];
	for (int symbol = 0; symbol < symbolsperFrame; symbol ++) {
	   if (tester < corrBank [symbol])
	      return false;
	}
	return true;
}

void	timeCorrelator::getCorr (int symbol,
	                         std::complex<float> *vec) {
std::complex<float>	sum	= std::complex<float> (0, 0);
int i;
	for (int carrier = K_min; carrier <= K_max; carrier ++) {
	   if (isTimeCell (carrier)) {
	      std::complex<float> temp =
	            vec [carrier - K_min] *
	                conj (getTimeRef (carrier));
	      sum += temp;
	   }
	}
	corrBank [symbol] = real (sum * conj (sum));
}

int	timeCorrelator::get_bestIndex (float *v) {
float max	= 0;
int	best	= 0;
float	avg	= 0;

	for (int i = 0; i < symbolsperFrame; i ++) {
	   avg += corrBank [i];
	   if (corrBank [i] > max) {
	      max = corrBank [i];
	      best = i;
	   }
	}
	*v = corrBank [best] / (avg / symbolsperFrame);
	return best;
}

