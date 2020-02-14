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
#include	"wav-reader-conv.h"
#include	"utilities.h"

static inline
int64_t		getMyTime	(void) {
struct timeval	tv;

	gettimeofday (&tv, NULL);
	return ((int64_t)tv. tv_sec * 1000000 + (int64_t)tv. tv_usec);
}

	wavReader_conv::
	        wavReader_conv	(SF_INFO *sp,
	                         SNDFILE *fp,
	                         RingBuffer<std::complex<float> > *b) {
SF_INFO *sf_info;
	sf_info			= sp;
	filePointer		= fp;
	_I_Buffer		= b;

	samplesinFile		= sf_info	-> frames;
	sampleRate		= sf_info	-> samplerate;
	numofChannels		= sf_info	-> channels;
	resetRequest		= false;
	currPos			= 0;
}

	wavReader_conv::
	       ~wavReader_conv (void) {
	stopReader ();
}

bool	wavReader_conv::restartReader	(void) {
	if (running. load ())
	   return true;
	start ();
	return true;
}

void	wavReader_conv::stopReader	(void) {
	if (!running. load ())
	   return;
	running. store (false);
	while (isRunning ())
           usleep (100);
}

/*	length is number of samples we read
 *	In case of mono, we add zeros to the right channel
 *	and therefore only read length / 2 floats
 */
int32_t	wavReader_conv::readBuffer (float *data, int32_t length) {
int32_t	n, i;
float	*tempBuffer;
static	int cnt	= 0;
static	int offset	= 0;

	n = sf_readf_float (filePointer, data, length);
	if (n < length) {
	   sf_seek (filePointer, 0, SEEK_SET);
	   fprintf (stderr, "eof reached, starting again\n");
	   offset = 0;
	}

	if (++cnt > 10) {
	   currPos		= sf_seek (filePointer, 0, SEEK_CUR);
	   float totalTime	= (float)samplesinFile / sampleRate;
	   float currTime	= (float)currPos / sampleRate;
	   float ccc		= (float)currPos / samplesinFile;
	   emit set_progressBar ((int)(ccc * 100), 
	                                       currTime, totalTime);
	   cnt = 0;
	}
	offset += n;
	return	n;
}

int32_t	wavReader_conv::setFileat	(int32_t f) {
int32_t	currPos = sf_seek (filePointer, f * samplesinFile / 100, SEEK_SET);
	return currPos * 100 / samplesinFile;
}

void	wavReader_conv::reset	(void) {
	resetRequest	= true;
}

void	wavReader_conv::run (void) {
int32_t	t;
float	*bi;
int32_t	period;
int64_t	nextStop;

//	We emulate the sending baudrate by building in some
//	delays. The period is 100000 / (baudrate / 10) usecs
	period		= 100000;
	running. store (true);
	int32_t	outputLimit	= sampleRate / 10;
	double	ratio	= (double)192000 / sampleRate;

//	inputbuffer size:
	int inputSize	= ceil (outputLimit / ratio);
	bi			= new float [2 * inputSize];
	int		error;
//	outputbuffer size
	float	bo [2 * outputLimit];
	SRC_STATE	*converter	= src_new (SRC_LINEAR, 2, &error);
	SRC_DATA	*src_data	= new SRC_DATA;
	if (converter == 0) {
	   fprintf (stderr, "error creating a converter %d\n", error);
	   return;
	}

	fprintf (stderr,
	            "Starting converter with ratio %f (in %d, out %d)\n",
	                                       ratio, sampleRate, 192000);
	src_data -> data_in		= bi;
	src_data -> data_out		= bo;
	src_data -> src_ratio		= ratio;
	src_data -> end_of_input	= 0;
	nextStop			= getMyTime ();
	while (running. load ()) {
	   int res;
	   if (resetRequest) {
	      sf_seek (filePointer, 0, SEEK_SET);
	      resetRequest = false;
	   }

	   while (_I_Buffer -> WriteSpace () < 2 * outputLimit + 10) {
	      if (!running. load ())
	         break;
	      usleep (100);
	   }

	   nextStop	+= period;
	   t = readBuffer (bi, inputSize);
	   src_data -> input_frames	= t;
	   src_data -> output_frames	= outputLimit;

//	we are reading continously, after eof we just start all over again,
//	therefore end_of_input flag is not altered
	   res	= src_process	(converter, src_data);
	   if (res != 0) {
	      fprintf (stderr, "error %s\n", src_strerror (t));
	   }
	   else {
	      int numofOutputs = src_data -> output_frames_gen;
	      _I_Buffer -> putDataIntoBuffer ((std::complex<float>*)src_data -> data_out,
                                               src_data -> output_frames_gen);
	   }

	   if (_I_Buffer -> GetRingBufferReadAvailable () > 192000 / 10) {
	      emit dataAvailable (192000 / 10);
	      usleep (1000000 / 10);
	   }

//	   if (nextStop - getMyTime () > 0)
//	      usleep (nextStop - getMyTime ());
	}

	delete	bi;
	src_delete (converter);
	delete src_data;
	fprintf (stderr, "taak voor replay eindigt hier\n");
}

