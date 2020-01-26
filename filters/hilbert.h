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
 *
 *	This filter is taken from
 *	https://www.dsprelated.com/showarticle/874.php
 *	Copyright Richard Lyons
 *
 *	seems to work great, thanks
 */
#ifndef	__HILBERT__
#define	__HILBERT__
#include	<stdlib.h>
#include	<stdint.h>
#include	<math.h>
#include	<vector>
#include	<complex>

class	Hilbert {
public:
			Hilbert		(int);
			~Hilbert	(void);
std::complex<float>	Pass		(float v);
std::complex<float>	Pass		(std::complex<float>);
private:

#define	BUFSIZE	15
	int	dir;
	float   ibuffer [BUFSIZE];
	float   qbuffer [BUFSIZE];
	int	p;

};

#endif


