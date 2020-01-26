#
/*
 *    Copyright (C) 2013 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
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
 *    along with DRM+ Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__RAW_READER_8__
#define	__RAW_READER_8__

#include	<QThread>
#include	"radio-constants.h"
#include	"ringbuffer.h"
#include	<atomic>
class	rawFiles_8;

class	rawReader_8:public QThread {
Q_OBJECT
public:
			rawReader_8	(QString,
	                                 int32_t, RingBuffer<std::complex<float>> *); 
			~rawReader_8	();
	bool		restartReader	();
	void		stopReader	();
	void            reset           (void);
        void		setFileat	(int32_t);
private:
virtual void		run();
	int32_t         theRate;
        QString         f;
        RingBuffer<std::complex<float> >        *_I_Buffer;
        QString         fileName;
        FILE		*filePointer;
        bool            readerOK;
        int32_t         sampleRate;
        int16_t         bitsperSample;
        int32_t         samplesinFile;
        int32_t         currPos;
        int16_t         numofChannels;
        std::atomic<bool> running;
        bool            resetRequest;

signals:
	void		dataAvailable	(int);
        void            set_progressBar (int);
};

#endif

