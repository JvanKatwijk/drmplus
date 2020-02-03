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

#ifndef __WAVREADER_NCONV__
#define	__WAVREADER_NCONV__

#include	<QThread>
#include	<QString>
#include	<sndfile.h>
#include	<atomic>
#include	"radio-constants.h"
#include	"ringbuffer.h"
#include	"wav-reader-base.h"
//
//
class	wavReader_nconv: public wavreaderBase {
Q_OBJECT
public:
	wavReader_nconv	(SF_INFO *,
	                 SNDFILE *, 
	                 RingBuffer<std::complex<float> > *);
	~wavReader_nconv	(void);
//	
	bool		restartReader	(void);
	void		stopReader	(void);
	void		reset		(void);
	int32_t		setFileat	(int32_t);
protected:
virtual void		run		(void);
	int		readBuffer	(float *, int);
	RingBuffer<std::complex<float> >	*_I_Buffer;
	QString		fileName;
	int32_t		sampleRate;
	SNDFILE		*filePointer;
	int32_t		samplesinFile;
	int		numofChannels;
	int32_t		currPos;
	std::atomic<bool> running;
	bool		resetRequest;
};
#endif

