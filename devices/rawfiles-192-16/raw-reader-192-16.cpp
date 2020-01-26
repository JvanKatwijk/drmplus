#
/*
 *    Copyright (C) 2020
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

#include	<sys/time.h>
#include	"raw-reader-192-16.h"
#include	"rawfiles-192-16.h"


#define	BUFFERSIZE	8192
static inline
int64_t         getMyTime () {
struct timeval  tv;

        gettimeofday (&tv, nullptr);
        return ((int64_t)tv. tv_sec * 1000000 + (int64_t)tv. tv_usec);
}

	rawReader_16::rawReader_16	(QString f,
	                         int32_t rate,
	                         RingBuffer<std::complex<float>> *b) {
	fileName		= f;
	theRate			= rate;
	_I_Buffer		= b;
	filePointer		= fopen (fileName. toLatin1 (). data (), "r+b");
	if (filePointer == NULL) {
	   fprintf (stderr, "file %s no legitimate sound file\n",
                            f. toLatin1 (). data ());
           return;
	}
	fseek (filePointer, 0, SEEK_END);
	samplesinFile		= ftell (filePointer) / 16;
	sampleRate		= 192000;
	readerOK		= true;
	currPos			= 0;
	running. store (false);
}

		rawReader_16::~rawReader_16 () {
	if (!readerOK)
	   return;
	stopReader ();
	fclose (filePointer);
}

bool	rawReader_16::restartReader	() {
	if (!readerOK)
	   return false;
	if (running. load ())
	   return true;

	start ();
	return true;
}

void	rawReader_16::run () {
int64_t	nextStop;
int	i;
int	teller	= 0;
int16_t localBuffer [2 * BUFFERSIZE];
std::complex<float> theBuffer [BUFFERSIZE];
int	period	= (BUFFERSIZE * 1000000) / 192000;  // full IQś read
        fprintf (stderr, "Period = %ld\n", period);

	fprintf (stderr, "writespace %d\n", _I_Buffer -> WriteSpace ());
        fseek (filePointer, 0, SEEK_SET);
	running. store (true);
	nextStop        = getMyTime ();
	try {
	   while (running. load()) {
	      while (_I_Buffer -> WriteSpace () < 2 * BUFFERSIZE + 10) {
	         if (!running. load()) {
	            fprintf (stderr, "space %d\n", _I_Buffer -> WriteSpace ());
	            throw (32);	
	         }
	         usleep (1000);
              }

	      if (++teller >= 20) {
	         int xx = ftell (filePointer);
	         float progress = (float)xx / 4 / samplesinFile;
	         set_progressBar (progress * 100);
                 teller = 0;
	      }

	      nextStop += period;
	      int n = fread (localBuffer,
	                     sizeof (int16_t),
	                     2 * BUFFERSIZE, filePointer);
	      if (n <  BUFFERSIZE) {
	         fprintf (stderr, "eof gehad\n");
	         fseek (filePointer, 0, SEEK_SET);
                 for (i =  n; i < BUFFERSIZE; i ++) {
                    localBuffer [2 * i] = 0;
	            localBuffer [2 * i + 1] = 0;
	         }
              }

	      for (int i = 0; i < BUFFERSIZE; i ++)
	         theBuffer [i] = std::complex<float> (
	                            localBuffer [2 * i] / 32768.0,
	                            localBuffer [2 * i + 1] / 32768.0);
	      _I_Buffer -> putDataIntoBuffer (theBuffer, BUFFERSIZE);
	      if (_I_Buffer -> GetRingBufferReadAvailable () > theRate / 10) {
                 emit dataAvailable (theRate / 10);
	         usleep (100000);
	      }

              if (nextStop - getMyTime() > 0)
                 usleep (nextStop - getMyTime());
           }
	} catch (int e) {}
        fprintf (stderr, "taak voor replay eindigt hier\n");
}

void	rawReader_16::stopReader() {
	if (running) {
	   running = false;
	   while (isRunning())
	      usleep (200);
	}
}

void	rawReader_16::setFileat     (int32_t f) {
	(void)f;
}

void    rawReader_16::reset (void) {
}


