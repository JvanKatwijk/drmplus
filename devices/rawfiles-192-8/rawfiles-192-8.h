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
#ifndef	__RAW_FILES_8__
#define	__RAW_FILES_8__

#include	<QThread>
#include	<QString>
#include	<QFrame>
#include	<atomic>
#include	"radio-constants.h"
#include	"device-handler.h"
#include	"ringbuffer.h"
#include	"ui_filereader-widget.h"

class	QLabel;
class	QSettings;
class	rawReader_8;
/*
 */
class	rawFiles_8: public deviceHandler,
	                     public Ui_filereaderWidget {
Q_OBJECT
public:

			rawFiles_8	(RadioInterface *,
	                                 int32_t,
	                                 RingBuffer<std::complex<float>> *);
	               ~rawFiles_8	();
        int32_t getRate                 (void);
        bool    restartReader           (void);
        void    stopReader              (void);
        int16_t bitDepth                (void);
        void    exit                    (void);
        bool    isOK                    (void);

protected:
        int32_t         setup_Device    (void);
        QFrame          *myFrame;
        rawReader_8	*myReader_8;
        QLabel          *indicator;
        QLabel          *fileDisplay;
        int32_t         lastFrequency;
        int32_t         theRate;
private slots:
        void            reset           (void);
        void            handle_progressBar (int);
        void            set_progressBar (int, float, float);
public slots:
	void		handleData	(int);
};

#endif

