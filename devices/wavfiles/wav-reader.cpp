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
#include	"wav-reader.h"
#include	"utilities.h"

static inline
int64_t		getMyTime	(void) {
struct timeval	tv;

	gettimeofday (&tv, NULL);
	return ((int64_t)tv. tv_sec * 1000000 + (int64_t)tv. tv_usec);
}

	wavReader::wavReader	(QString f,
	                         int32_t rate,	
	                         RingBuffer<std::complex<float> > *b) {
SF_INFO	*sf_info;

	fileName		= f;
	theRate			= rate;
	_I_Buffer		= b;

	sf_info			= (SF_INFO *)alloca (sizeof (SF_INFO));
	sf_info	-> format	= 0;
	filePointer		= sf_open (fileName. toUtf8 (). data (),
	                                   SFM_READ, sf_info);
	if (filePointer == NULL) {
	   fprintf (stderr, "file %s no legitimate sound file\n",
	                    f. toLatin1 (). data ());
	   return;
	}

	samplesinFile	= sf_info	-> frames;
	sampleRate	= sf_info	-> samplerate;
	if (sampleRate != 192000) {
	   fprintf (stderr, "sorry, right now we only handle drm+ files 192000\n");
	   throw (23);
	}
	
	theRate		= rate;
	numofChannels	= sf_info	-> channels;
	resetRequest	= false;
	running. store (false);
}

	wavReader::~wavReader (void) {
	stopReader ();
	sf_close (filePointer);
}

bool	wavReader::restartReader	(void) {
	if (running. load ())
	   return true;
	start ();
	return true;
}

void	wavReader::stopReader	(void) {
	if (!running. load ())
	   return;
	running. store (false);
	while (isRunning ())
           usleep (100);
}

float	temp [10 * 192000];
int32_t	wavReader::readBuffer (std::complex<float> *data, int32_t length) {
int32_t	n, i;
static int cnt	= 0;

	n = sf_readf_float (filePointer, temp, length);
	if (n < length) {
	   sf_seek (filePointer, 0, SEEK_SET);
	   fprintf (stderr, "End of file, restarting\n");
	   return 0;
	}

	for (i = 0; i < n; i ++)
	   data [i] = std::complex<float> (temp [2 * i], temp [2 * i + 1]);

	if (++cnt > 5) {
	   int currPos		= sf_seek (filePointer, 0, SEEK_CUR);
	   emit set_progressBar (currPos * 100 / samplesinFile);
	   cnt = 0;
	}
	return	 n;
}

int32_t	wavReader::setFileat	(int32_t f) {
int32_t	currPos = sf_seek (filePointer, f * samplesinFile / 100, SEEK_SET);
	return currPos * 100 / samplesinFile;
}

void	wavReader::reset	(void) {
	resetRequest	= true;
}

/*
 *	for processing the file-io we start a separate thread
 *	In this thread, we also apply samplerate conversions
 *	- if any
 */

std::complex<float> bi [10 * 192000];
void	wavReader::run (void) {
int32_t	t, i;
int	bufferSize	= 5 * 192000;
int32_t	period;
int64_t	nextStop;

	running. store (true);
	period		= (bufferSize * 1000) / 192;	// full IQś read
	nextStop	= getMyTime ();
	fprintf (stderr, "Period = %ld\n", period);
	nextStop	= getMyTime ();
	while (running. load ()) {
	   nextStop += period;
	   t = readBuffer (bi, bufferSize);
	   for (int k = 0; k < 20; k ++) {
	      while (_I_Buffer -> WriteSpace () < bufferSize / 20) {
	         if (!running. load ())
	            break;
	         usleep (100);
	      }
	      _I_Buffer -> putDataIntoBuffer (& bi [k * bufferSize / 20],
	                                           bufferSize / 20);
	      emit dataAvailable (theRate / 10);
	      usleep (1000000 / 10);
	   }

//	   if (nextStop - getMyTime () > 0)
//	      usleep (nextStop - getMyTime ());
	}
	fprintf (stderr, "taak voor replay eindigt hier\n");
}

