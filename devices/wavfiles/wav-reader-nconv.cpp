#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ receiver
 *
 *    DRM+ receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QFrame>
#include	<samplerate.h>
#include	<sys/time.h>
#include	<time.h>
#include	"radio-constants.h"
#include	"wav-reader-nconv.h"
#include	"utilities.h"

static inline
int64_t		getMyTime	(void) {
struct timeval	tv;

	gettimeofday (&tv, NULL);
	return ((int64_t)tv. tv_sec * 1000000 + (int64_t)tv. tv_usec);
}

	wavReader_nconv::
	            wavReader_nconv	(SF_INFO *sp, SNDFILE *fp,
	                                 RingBuffer<std::complex<float> > *b) {
SF_INFO	*sf_info;
	sf_info			= sp;
	filePointer		= fp;
	_I_Buffer		= b;

	samplesinFile	= sf_info	-> frames;
	sampleRate	= sf_info	-> samplerate;
	resetRequest	= false;
	currPos		= 0;
}

	wavReader_nconv::~wavReader_nconv (void) {
	stopReader ();
}

bool	wavReader_nconv::restartReader	(void) {
	if (running. load ())
	   return true;
	start ();
	return true;
}

void	wavReader_nconv::stopReader	(void) {
	if (!running. load ())
	   return;
	running. store (false);
	while (isRunning ())
           usleep (100);
}
//
//	"length" is the number of IQ samples
int32_t	wavReader_nconv::readBuffer (float *data, int32_t length) {
int32_t	n;
static	int cnt	= 0;

	n = sf_readf_float (filePointer, data, length);

	if (n < length) {
	   sf_seek (filePointer, 0, SEEK_SET);
	   fprintf (stderr, "eof reached, starting again\n");
	}

	if (++cnt > 10) {
	   currPos		= sf_seek (filePointer, 0, SEEK_CUR);
	   float totalTime	= (float)samplesinFile / 192000;
	   float currTime	= (float)currPos / 192000;
	   emit set_progressBar (currPos * 100 / samplesinFile,
	                                           currTime, totalTime);
	   cnt = 0;
	}
	return	 n;
}

int32_t	wavReader_nconv::setFileat	(int32_t f) {
int32_t	currPos = sf_seek (filePointer, f * samplesinFile / 100, SEEK_SET);
	return currPos * 100 / samplesinFile;
}

void	wavReader_nconv::reset	(void) {
	resetRequest	= true;
}

/*
 */
void	wavReader_nconv::run (void) {
int32_t	t;
float	*bi;
int32_t	bufferSize;
int32_t	period;
int64_t	nextStop;

//	We emulate the sending baudrate by building in some
//	delays. The period is 100000 / (baudrate / 10) usecs
	period		= 100000;
	running. store (true);
	bufferSize	= sampleRate / 10;
	bi		= new float [2 * bufferSize];
	nextStop	= getMyTime ();
	while (running. load ()) {
	   nextStop += period;
	   if (resetRequest) {
	      sf_seek (filePointer, 0, SEEK_SET);
	      resetRequest = false;
	   }

	   t = readBuffer (bi, bufferSize);
           while (_I_Buffer -> WriteSpace () < bufferSize) {
	      if (!running. load ())
	         break;
	      usleep (100);
	   }
	   _I_Buffer -> putDataIntoBuffer (bi, bufferSize);
	      emit dataAvailable (bufferSize);
	      usleep (100000);
	}
	delete bi;
	fprintf (stderr, "taak voor replay eindigt hier\n");
}

