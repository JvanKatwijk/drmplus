#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the swradio
 *
 *    swradio is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    swradio is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with swradio; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"hilbert.h"

		Hilbert::Hilbert	(int q) {
	(void)q;
	p	= 0;
}

		Hilbert::~Hilbert	(void) {
}

std::complex<float> Hilbert::Pass	(float v) {
        ibuffer [p]	= v;
        qbuffer [p]	= v;
        int     cp      = p + BUFSIZE;
        float sum_1 = 1.0 / 7 * (ibuffer [p] - ibuffer [(cp - 14) % BUFSIZE]);
        float sum_2 = 1.0 / 6 * (ibuffer [(cp - 1) % BUFSIZE] -
                                              ibuffer [(cp - 13) % BUFSIZE]);
        float sum_3 = 1.0 / 5 * (ibuffer [(cp - 2) % BUFSIZE] -
                                              ibuffer [(cp - 12) % BUFSIZE]);
        float sum_4 = 1.0 / 4 * (ibuffer [(cp - 3) % BUFSIZE] -
                                              ibuffer [(cp - 11) % BUFSIZE]);
        float sum_5 = 1.0 / 3 * (ibuffer [(cp - 4) % BUFSIZE] -
                                              ibuffer [(cp - 10) % BUFSIZE]);
        float sum_6 = 1.0 / 2 * (ibuffer [(cp - 5) % BUFSIZE] -
                                              ibuffer [(cp -  9) % BUFSIZE]);
        float sum_7 =           (-ibuffer [(cp - 6) % BUFSIZE] +
                                              ibuffer [(cp - 8) % BUFSIZE]);

        float new_i = - sum_1 + sum_2 +  - sum_3 + sum_4 + - sum_5
                             + sum_6 + sum_7;
        return std::complex <float> (new_i, qbuffer [(cp - 15) % BUFSIZE]);
}

std::complex<float> Hilbert::Pass	(std::complex<float> z) {
        ibuffer [p]	= real (z);
        qbuffer [p]	= imag (z);
        int     cp      = p + BUFSIZE;
        float sum_1 = 1.0 / 7 * (qbuffer [p] - qbuffer [(cp - 14) % BUFSIZE]);
        float sum_2 = 1.0 / 6 * (qbuffer [(cp - 1) % BUFSIZE] -
                                              qbuffer [(cp - 13) % BUFSIZE]);
        float sum_3 = 1.0 / 5 * (qbuffer [(cp - 2) % BUFSIZE] -
                                              qbuffer [(cp - 12) % BUFSIZE]);
        float sum_4 = 1.0 / 4 * (qbuffer [(cp - 3) % BUFSIZE] -
                                              qbuffer [(cp - 11) % BUFSIZE]);
        float sum_5 = 1.0 / 3 * (qbuffer [(cp - 4) % BUFSIZE] -
                                              qbuffer [(cp - 10) % BUFSIZE]);
        float sum_6 = 1.0 / 2 * (qbuffer [(cp - 5) % BUFSIZE] -
                                              qbuffer [(cp -  9) % BUFSIZE]);
        float sum_7 =           (-qbuffer [(cp - 6) % BUFSIZE] +
                                              qbuffer [(cp - 8) % BUFSIZE]);

        float new_i = - sum_1 + sum_2 +  - sum_3 + sum_4 + - sum_5
                             + sum_6 + sum_7;
        return std::complex <float> (new_i, qbuffer [(cp - 15) % BUFSIZE]);
}


