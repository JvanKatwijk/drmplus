#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ decoder
 *
 *    DRM+ decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"fm-decoder.h"
#include	"fm-processor.h"
#include	"radio.h"

	fmDecoder::fmDecoder (RadioInterface *mr,
	                      RingBuffer<std::complex<float>> * inputData,
	                      RingBuffer<std::complex<float>> * audioData) {
        myFrame                 = new QFrame (nullptr);
        setupUi (myFrame);
	myFrame		-> show ();
	connect (demodulatorSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (handle_demodulatorSelector (const QString &)));
	theProcessor	= new fmProcessor (this, inputData, audioData);
	connect (theProcessor, SIGNAL (audioAvailable (int, int)),
	         mr, SLOT (processAudio (int, int)));
	connect (fmDeemphasisSelector, SIGNAL(activated(const QString &)),
	         this, SLOT (handle_fmDeemphasisSelector (const QString &)));
	connect (fmMode, SIGNAL (activated (const QString &)),
	         this, SLOT (handle_fmMode (const QString &)));
	connect (squelchButton, SIGNAL (clicked ()),
	         this, SLOT (handle_squelchButton ()));
	connect (squelchSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_squelchSlider (int)));
}

	fmDecoder::~fmDecoder	() {
	delete theProcessor;
	delete myFrame;
}

void	fmDecoder::handle_demodulatorSelector (const QString &s) {
	if (theProcessor == nullptr)
	   return;

	fprintf (stderr, "going to select %s\n", s. toLatin1 (). data ());
	if (s == "fm1decoder")
	   theProcessor	-> setFMdecoder (1);
	else
	if (s == "fm2decoder")
	   theProcessor -> setFMdecoder (2);
	else
	if (s == "fm3decoder")
	   theProcessor -> setFMdecoder (3);
	else
	if (s == "fm4decoder")
	   theProcessor -> setFMdecoder (4);
	else
	if (s == "fm5decoder")
	   theProcessor -> setFMdecoder (5);
}

void	fmDecoder::audioAvailable	(int a, int b) {
	processAudio (a, b);
}

void	fmDecoder::setRDSisSynchronized (bool syn) {
	if (!syn)
           rdsSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
        else
           rdsSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
}

void	fmDecoder::setbitErrorRate (double d) {
	bitErrorRate	-> display (d);
}

void	fmDecoder::setGroup (int i) {}
void	fmDecoder::setPTYCode (int i) {}
void	fmDecoder::setPiCode (int n) {
	rdsPiDisplay	-> display (n);
}

void	fmDecoder::setStationLabel (const QString &s) {
	stationLabelTextBox	-> setText (s);
}

void	fmDecoder::clearStationLabel (void) {
	stationLabelTextBox	-> setText ("");
}

void	fmDecoder::setRadioText (const QString &s) {
	radioTextBox	-> setText (s);
}
void	fmDecoder::clearRadioText (void) {
	radioTextBox	-> setText ("");
}

void	fmDecoder::setAFDisplay (int n) {
	rdsAFDisplay	-> display (n);
}

void	fmDecoder::setMusicSpeechFlag (int n) {
	if (n != 0)
           speechLabel -> setText (QString ("music"));
        else
           speechLabel -> setText (QString ("speech"));
}

void	fmDecoder::clearMusicSpeechFlag (void) {
	speechLabel	-> setText ("");
}

void	fmDecoder::setCRCErrors (int n) {
	crcErrors	-> display (n);
}

void	fmDecoder::setSyncErrors (int n) {
	syncErrors	-> display (n);
}

void	fmDecoder::showLocked	(bool pilot, float dcComp) {
	if (pilot)
	   pilotIndicator ->
	           setStyleSheet ("QLabel {background-color:green}");
	else
	   pilotIndicator ->
	           setStyleSheet ("QLabel {background-color:red}");
	dc_component    -> display (dcComp);
}

void	fmDecoder::handle_fmDeemphasisSelector	(const QString &s) {
	if (theProcessor != nullptr)
	   theProcessor -> setDeemphasis (s == "50" ? 50 : 75);
}

void	fmDecoder::handle_fmMode		(const QString &s) {
	if (theProcessor != nullptr)
	   theProcessor -> setfmMode (s == "STEREO");
}

void	fmDecoder::handle_audioChannelSelect	(const QString &s) {}

static	bool squelchMode	= false;
void	fmDecoder::handle_squelchButton		() {
	squelchMode	= !squelchMode;
	if (theProcessor != nullptr)
	   theProcessor -> set_squelchMode (squelchMode);
	squelchButton -> setText (squelchMode ? "squelch on" : "squelch off");
}

void	fmDecoder::handle_squelchSlider		(int v) {
	if (theProcessor != nullptr)
	   theProcessor -> set_squelchValue (v);
}

