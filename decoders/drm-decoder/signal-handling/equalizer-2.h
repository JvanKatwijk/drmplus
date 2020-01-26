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

#ifndef	__EQUALIZER_2__
#define	__EQUALIZER_2__

#include	<QObject>
#include	"ringbuffer.h"
#include	<complex>
#include	"basics.h"
class		drmDecoder;

class	equalizer_2: public QObject {
Q_OBJECT
public:
			equalizer_2 	(drmDecoder	*parent,
	                                 RingBuffer<std::complex<float>> *);
			~equalizer_2 	(void);
	bool		equalize	(std::complex<float> *,
	                                 int16_t,
	                                 theSignal **);
private:
	drmDecoder	*parent;
	RingBuffer<std::complex<float>>	*eqBuffer;
void	process (int startCarrier, int endCarrier,
                 int symbolNumber, std::complex<float> *inVec,
                              theSignal ** outVec);

signals:
	void	show_eqsymbol		(int);

};

#endif

