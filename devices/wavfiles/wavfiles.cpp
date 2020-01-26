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
#include	"wav-reader.h"
#include	"wavfiles.h"

	wavFiles::wavFiles	(RadioInterface *mr,
	                         int32_t rate,
	                         RingBuffer<std::complex<float>> *b):
	                              deviceHandler (mr) {
	theRate		= rate;
	this	-> myFrame	= new QFrame;
	setupUi		(myFrame);
	myReader	= NULL;
	QString	replayFile
	              = QFileDialog::
	                 getOpenFileName (myFrame,
	                                  tr ("load file .."),
	                                  QDir::homePath (),
	                                  tr ("sound (*.wav)"));
	replayFile	= QDir::toNativeSeparators (replayFile);
	myReader	= new wavReader (replayFile, rate, b);
	connect (myReader, SIGNAL (set_progressBar (int)),
	         this, SLOT (set_progressBar (int)));
	connect (myReader, SIGNAL (dataAvailable (int)),
	         this, SIGNAL (dataAvailable (int)));
	nameofFile	-> setText (replayFile);
	set_progressBar	(0);
	this	-> lastFrequency	= Khz (94000);
	connect (fileProgress, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_progressBar (int)));
}
//
	wavFiles::~wavFiles	(void) {
	if (myReader != nullptr)
	   delete myReader;
	myReader	= NULL;
	delete myFrame;
}

void	wavFiles::handle_progressBar	(int f) {
	myReader	-> setFileat (f);
}

void	wavFiles::set_progressBar	(int f) {
	fileProgress	-> setValue (f);
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

