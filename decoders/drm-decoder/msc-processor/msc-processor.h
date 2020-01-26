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

#ifndef	__MSC_PROCESSOR__
#define	__MSC_PROCESSOR__

#include	<QObject>
#include	"ringbuffer.h"
#include	"basics.h"
#include	<mutex>
#include	<deinterleaver.h>
#include	"data-processor.h"

class	drmDecoder;
class	mscHandler;

	class	mscProcessor: public QObject {
Q_OBJECT
public:
			mscProcessor	(drmDecoder *,
	                                 drmParameters *,
	                                 RingBuffer<std::complex<float>> *);
			~mscProcessor	();
	void		processFrame	(theSignal ** outbank);
	void		set_Viewer	(bool);
	void		selectService	(int stream);
private:
	drmDecoder		*theParent;
	drmParameters		*params;
	RingBuffer<std::complex<float>> *iqBuffer;
	bool			show_Constellation;
	deInterleaver_long	*my_deInterleaver;
	dataProcessor		my_dataProcessor;
	int			start_frame_2;
	int			start_frame_3;
	int			start_frame_4;

	int			muxLength;
	int			muxCounter;
	theSignal		*muxsampleBuf;
	int			bufferP;
	mscHandler		*my_mscHandler;
	void			process_mux	(theSignal *, bool);
	std::mutex		locker;
signals:
	void			show_const	();
};

#endif
