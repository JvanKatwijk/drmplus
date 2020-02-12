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
 *
 * 	File reader:
 *	For the 8 bit iq data 
 */
#include        <QLabel>
#include        <QFileDialog>
#include        "radio-constants.h"
#include        "radio.h"
#include	<ctime>
#include	"raw-reader-192-8.h"
#include	"rawfiles-192-8.h"

//
	rawFiles_8::rawFiles_8 (RadioInterface *mr,
	                        int32_t	rate,
	                        RingBuffer<std::complex<float>> *b) :
	                              deviceHandler (mr) {
	theRate		= rate;
	myFrame		= new QFrame;
	setupUi	(myFrame);
	myFrame		-> show ();
	QString	replayFile
	              = QFileDialog::
	                 getOpenFileName (myFrame,
	                                  tr ("load file .."),
	                                  QDir::homePath (),
	                                  tr ("iq (*.*)"));
	replayFile	= QDir::toNativeSeparators (replayFile);
	myReader_8	= new rawReader_8 (replayFile, rate, b);
	connect (myReader_8, SIGNAL (set_progressBar (int, float, float)),
	         this, SLOT (set_progressBar (int, float, float)));
	connect (myReader_8, SIGNAL (dataAvailable (int)),
	         this, SLOT (handleData (int)));
	nameofFile	-> setText (replayFile);
	set_progressBar	(0, 0, 0);
	this	-> lastFrequency	= Khz (94000);
	connect (fileProgress, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_progressBar (int)));
}

	rawFiles_8::~rawFiles_8 () {
	if (myReader_8 != NULL)
	   delete myReader_8;
	myReader_8	= NULL;
	delete	myFrame;
}

void    rawFiles_8::handle_progressBar          (int f) {
        myReader_8        -> setFileat (f);
}

void    rawFiles_8::set_progressBar     (int f, float c, float t) {
	disconnect (fileProgress, SIGNAL (valueChanged (int)),
                    this, SLOT (handle_progressBar (int)));
	fileProgress	-> setValue (f);
	currentTime	-> display  (c);
	totalTime	-> display  (t);
	disconnect (fileProgress, SIGNAL (valueChanged (int)),
	            this, SLOT (handle_progressBar (int)));
}

bool	rawFiles_8::restartReader	() {
	if (myReader_8 != NULL) {
	   bool b = myReader_8 -> restartReader ();
	   return b;
	}
	else
	   return false;
}

void	rawFiles_8::stopReader		(void) {
	if (myReader_8 != NULL)
	   myReader_8	-> stopReader ();
}

void	rawFiles_8::exit		(void) {
	if (myReader_8 != NULL)
	   myReader_8	-> stopReader ();
}

int16_t	rawFiles_8::bitDepth		(void) {
	return 16;
}

void	rawFiles_8::reset		() {
	if (myReader_8 != NULL)
	   myReader_8 -> reset ();
}

int32_t	rawFiles_8::getRate		() {
	return 192000;
}

void	rawFiles_8::handleData		(int a) {
	emit dataAvailable (a);
}

