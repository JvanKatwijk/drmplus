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
 *    along with DRM+ Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __WAVFILES__
#define	__WAVFILES__

#include	<QWidget>
#include	<QFrame>
#include	<QString>
#include	"device-handler.h"
#include	"ui_filereader-widget.h"
#include	"ringbuffer.h"

class	QLabel;
class	wavReader;
class	RadioInterface;
/*
 */
class	wavFiles: public deviceHandler,
	                        public Ui_filereaderWidget {
Q_OBJECT
public:
		wavFiles		(RadioInterface *,
	                                 int32_t, 
	                                 RingBuffer<std::complex<float>> *);
		~wavFiles		(void);
	int32_t	getRate			(void);

	bool	restartReader		(void);
	void	stopReader		(void);
	int16_t	bitDepth		(void);
	void	exit			(void);
	bool	isOK			(void);
protected:
	int32_t		setup_Device	(void);
	QFrame		*myFrame;
	wavReader	*myReader;
	int32_t		lastFrequency;
	int32_t		theRate;
private slots:
	void		reset		(void);
	void		handle_progressBar (int);
	void		set_progressBar	(int);
};
#endif

