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

#ifndef	__EQUALIZER_1__
#define	__EQUALIZER_1__

#include	"equalizer-base.h"
#include	<fftw3.h>
#include	"ringbuffer.h"
#include	<vector>
class		estimatorBase;
class		drmDecoder;

typedef	struct {
	int16_t	symbol;
	int16_t	carrier;
} trainer;

class	equalizer_1: public equalizer_base {
public:
			equalizer_1 	(drmDecoder	*parent,
	                                 RingBuffer<std::complex<float>> *);
			~equalizer_1	();
	bool		equalize	(std::complex<float> *,
	                                 int16_t,
	                                 theSignal **,
	                                 float *,
	                                 float *,
	                                 float *);
	bool		equalize	(std::complex<float> *,
	                                 int16_t,
	                                 theSignal **);
//	void		init		(std::complex<float> *, int16_t);
private:
	drmDecoder	*parent;
	RingBuffer<std::complex<float>>	*eqBuffer;
	void		getRelAddress	(int16_t, int16_t *, int16_t *);
	int16_t		buildTrainers	(int16_t);
	int16_t		rndcnt;
	estimatorBase	**Estimators;
	int16_t		windowsinFrame;
	int16_t		periodforPilots;
	int16_t		periodforSymbols;
	double		**W_symbol_blk [10];
	float		f_cut_t;
	float		f_cut_k;
	std::vector<std::vector<trainer>> theTrainers;
	std::vector<std::vector<std::complex<float>>> pilotEstimates;
	int16_t		trainers_per_window [10];
	int16_t		symbols_to_delay;
	int16_t		symbols_per_window;
	void		processSymbol	(int16_t, theSignal *);
};

#endif

