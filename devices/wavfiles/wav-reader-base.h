#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
 *    along with DRM+Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __WAVREADER_BASE__
#define	__WAVREADER_BASE__

#include	<QThread>
#include	<QString>
#include	<sndfile.h>
#include	<atomic>
#include	"radio-constants.h"
#include	"ringbuffer.h"
//
//
class	wavreaderBase: public QThread {
Q_OBJECT
public:
	wavreaderBase	();
virtual	~wavreaderBase	(void);
//	
virtual	bool		restartReader	(void);
virtual	void		stopReader	(void);
virtual	void		reset		(void);
virtual	int32_t		setFileat	(int32_t);
protected:
virtual void		run		(void);
signals:
	void		dataAvailable	(int);
	void		set_progressBar	(int);
};
#endif

