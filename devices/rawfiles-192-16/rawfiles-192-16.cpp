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
 *	For the 16 bit raw data 
 */
#include        <QLabel>
#include        <QFileDialog>
#include        "radio-constants.h"
#include        "radio.h"
#include	<ctime>
#include	"raw-reader-192-16.h"
#include	"rawfiles-192-16.h"

//
	rawFiles_16::rawFiles_16 (RadioInterface *mr,
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
	myReader_16	= new rawReader_16 (replayFile, rate, b);
	connect (myReader_16, SIGNAL (set_progressBar (int)),
	         this, SLOT (set_progressBar (int)));
	connect (myReader_16, SIGNAL (dataAvailable (int)),
	         this, SLOT (handleData (int)));
	nameofFile	-> setText (replayFile);
	set_progressBar	(0);
	this	-> lastFrequency	= Khz (94000);
	connect (fileProgress, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_progressBar (int)));
}

	rawFiles_16::~rawFiles_16 () {
	if (myReader_16 != NULL)
	   delete myReader_16;
	myReader_16	= NULL;
	delete	myFrame;
}

void    rawFiles_16::handle_progressBar          (int f) {
        myReader_16        -> setFileat (f);
}

void    rawFiles_16::set_progressBar     (int f) {
	fileProgress     -> setValue (f);
}

bool	rawFiles_16::restartReader	() {
	if (myReader_16 != NULL) {
	   bool b = myReader_16 -> restartReader ();
	   return b;
	}
	else
	   return false;
}

void	rawFiles_16::stopReader		(void) {
	if (myReader_16 != NULL)
	   myReader_16	-> stopReader ();
}

void	rawFiles_16::exit		(void) {
	if (myReader_16 != NULL)
	   myReader_16	-> stopReader ();
}

int16_t	rawFiles_16::bitDepth		(void) {
	return 16;
}

void	rawFiles_16::reset		() {
	if (myReader_16 != NULL)
	   myReader_16 -> reset ();
}

int32_t	rawFiles_16::getRate		() {
	return 192000;
}

void	rawFiles_16::handleData		(int a) {
	emit dataAvailable (a);
}

