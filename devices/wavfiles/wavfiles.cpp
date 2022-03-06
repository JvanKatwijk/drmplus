#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
#include	<QLabel>
#include	<QFileDialog>
#include	"radio-constants.h"
#include	"radio.h"
#include	"wav-reader-base.h"
#include	"wav-reader-nconv.h"
#include	"wav-reader-conv.h"
#include	"wavfiles.h"

	wavFiles::wavFiles	(RadioInterface *mr,
	                         int32_t rate,
	                         RingBuffer<std::complex<float>> *b):
	                              deviceHandler (mr) {
SF_INFO	*sf_info;

	theRate		= rate;
	this	-> myFrame	= new QFrame;
	setupUi		(myFrame);
	myFrame	-> show ();
	myReader	= nullptr;
	QString	replayFile
	              = QFileDialog::
	                 getOpenFileName (myFrame,
	                                  tr ("load file .."),
	                                  QDir::homePath (),
	                                  tr ("sound (*.wav)"));
	replayFile	= QDir::toNativeSeparators (replayFile);

	sf_info                 = (SF_INFO *)alloca (sizeof (SF_INFO));
        sf_info -> format       = 0;
        filePointer             = sf_open (replayFile. toLatin1 (). data (),
                                           SFM_READ, sf_info);
        if (filePointer == NULL) {
           fprintf (stderr, "file %s no legitimate sound file\n",
                            replayFile. toLatin1 (). data ());
	   throw (21);
        }

	if (sf_info -> channels != 2) {
	   fprintf (stderr, "file %d no legitimate drm+ sdr file\n",
	                     replayFile. toLatin1 (). data ());
	   throw (22);
	}

	fprintf (stderr, "samplerate %d\n", sf_info -> samplerate);
	if (theRate == sf_info -> samplerate)
	   myReader	= new wavReader_nconv (sf_info,
	                                       filePointer, b);
	else
	   myReader	= new wavReader_conv  (sf_info,
	                                       filePointer, b);

	connect (myReader, SIGNAL (set_progressBar (int, float, float)),
	         this, SLOT (set_progressBar (int, float, float)));
	connect (myReader, SIGNAL (dataAvailable (int)),
	         this, SIGNAL (dataAvailable (int)));
	connect (fileProgress, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_progressBar (int)));
	nameofFile	-> setText (replayFile);
	set_progressBar	(0, 0, 0);
	this	-> lastFrequency	= Khz (94000);
}
//
	wavFiles::~wavFiles	(void) {
	if (myReader != nullptr)
	   delete myReader;
	myReader	= NULL;
	sf_close (filePointer);
	delete myFrame;
}

void	wavFiles::handle_progressBar	(int f) {
	myReader	-> setFileat (f);
}

void	wavFiles::set_progressBar	(int f, float c, float t) {
	disconnect (fileProgress, SIGNAL (valueChanged (int)),
                    this, SLOT (handle_progressBar (int)));
	fileProgress	-> setValue (f);
	currentTime	-> display (c);
	totalTime	-> display (t);
	connect (fileProgress, SIGNAL (valueChanged (int)),
                 this, SLOT (handle_progressBar (int)));
}

bool	wavFiles::restartReader		(void) {
	if (myReader != NULL)
	   return myReader -> restartReader ();
	else
	   return false;
}

void	wavFiles::stopReader		(void) {
	if (myReader != NULL)
	   myReader	-> stopReader ();
}

void	wavFiles::exit			(void) {
	if (myReader != NULL)
	   myReader	-> stopReader ();
}

int16_t	wavFiles::bitDepth		(void) {
	return 16;
}

void	wavFiles::reset			(void) {
	if (myReader != NULL)
	   myReader -> reset ();
}

int32_t	wavFiles::getRate		(void) {
	return theRate;
}

