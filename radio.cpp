#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of DRM+-receiver
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
#include	<unistd.h>
#include	<QSettings>
#include	<QDebug>
#include	<QDateTime>
#include	<QObject>
#include	<QDir>
#include	<QColor>
#include	<QMessageBox>
#include	<QFileDialog>
#include	"radio.h"
#include	"fft-scope.h"
#include	"audiosink.h"
#include        "popup-keypad.h"
#include        "program-list.h"
//
//      devices
#include        "device-handler.h"
#include	"wavfiles.h"
#include	"rawfiles-192-8.h"
#include	"rawfiles-192-16.h"
#include	"rawfiles-192-32.h"

#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY_V2
#include	"sdrplay-handler-v2.h"
#endif
#ifdef	HAVE_SDRPLAY_V3
#include	"sdrplay-handler-v3.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef	HAVE_SOAPY
#include	"soapy-handler.h"
#endif
#ifdef	HAVE_LIME
#include	"lime-handler.h"
#endif
//
//	the decoder
#include	"drm-decoder.h"
#include	"fm-decoder.h"

QString	FrequencytoString (int32_t freq) {
	if (freq < 10)
	   return QString ('0' + (uint8_t)(freq % 10));
	return 
	   FrequencytoString (freq / 10). append (QString ('0' + (uint8_t)(freq % 10)));
}
//
//	inputrate = 192K, wide enough to show - if available -
//	the drm+ signal and its surroundings
	RadioInterface::RadioInterface (QSettings	*sI,
	                                QString		stationList,
	                                int		inputRate,
	                                QWidget		*parent):
	                                    QMainWindow (parent) {
	this	-> settings	= sI;
	this	-> stationList	= stationList;
	this	-> inputRate	= inputRate;
	setupUi (this);
	QPalette *p = new QPalette;
	p -> setColor (QPalette::WindowText, Qt::white);
	frequencyDisplay -> setPalette (*p);
//      and some buffers
//	in comes:
        inputData       = new RingBuffer<std::complex<float> > (2048 * 1024);
	bufferData	= new RingBuffer<std::complex<float>> (1024 * 1024);
	audioData	= new RingBuffer<std::complex<float>> (32768);
	deviceSelector	-> addItem ("wav files");
	deviceSelector	-> addItem ("raw files");
	deviceSelector	-> addItem ("raw files 16");
	deviceSelector	-> addItem ("raw files 32");
//
#ifdef	HAVE_SDRPLAY_V2
	deviceSelector	-> addItem ("sdrplay");
#endif
#ifdef	HAVE_SDRPLAY_V3
	deviceSelector	-> addItem ("sdrplay-v3");
#endif
#ifdef	HAVE_RTLSDR
	deviceSelector	-> addItem ("dabstick");
#endif
#ifdef	HAVE_AIRSPY
	deviceSelector	-> addItem ("airspy");
#endif
#ifdef	HAVE_HACKRF
	deviceSelector	-> addItem ("hackrf");
#endif
#ifdef	HAVE_SOAPY
	deviceSelector	-> addItem ("soapy");
#endif
#ifdef	HAVE_LIME
	deviceSelector	-> addItem ("limeSDR");
#endif

	displaySize		= 1024;
	scopeWidth		= inputRate;
	theBand. currentOffset	= 0;
	theBand. lowF		= -48000;
	theBand. highF		= 48000;
//	scope and settings
	hfScopeSlider	-> setValue (50);
        hfScope		= new fftScope (hfSpectrum,
                                        displaySize,
                                        kHz (1),        // scale
                                        inputRate,
                                        hfScopeSlider -> value (),
                                        8);
	hfScope		-> set_bitDepth (12);	// default
	hfScope	-> setScope (94000 * 1000, 0);

        mykeyPad                = new keyPad (this);
	delayCount		= 0;
	secondsTimer. setInterval (1000);
	connect (&secondsTimer, SIGNAL (timeout (void)),
                 this, SLOT (updateTime (void)));
	connect (decoderSelect, SIGNAL (activated (const QString &)),
	         this , SLOT (handle_decoderSelect (const QString &)));
        secondsTimer. start (1000);

	audioHandler	= new audioSink (48000, 16384);
	outTable	= new int16_t
                               [audioHandler -> numberofDevices () + 1];
        for (int i = 0; i < audioHandler -> numberofDevices (); i ++)
           outTable [i] = -1;
        try {
           setupSoundOut (streamOutSelector, audioHandler,
                          48000, outTable);
        } catch (int e) {
           QMessageBox::warning (this, tr ("sdr"),
                                       tr ("Opening audio failed\n"));
           abort ();
        }
        audioHandler -> selectDefaultDevice ();

	connect (deviceSelector, SIGNAL (activated (const QString &)),
                    this,  SLOT (doStart (const QString &)));
	theDevice	= nullptr;
}

//      The end of all
        RadioInterface::~RadioInterface () {
	secondsTimer. stop ();
        delete  mykeyPad;
        delete  myList;
}
//
//
//	To keep things simple, we start after a device is selected
//	then we run, no re-selection of device is possible.
void	RadioInterface::doStart (const QString &s) {
theDevice	= setDevice (s, inputData);

	if (theDevice == nullptr)
	   return;

	hfScope		-> set_bitDepth (theDevice -> bitDepth ());
	deviceSelector	-> hide ();
        connect (hfScope,
                 SIGNAL (clickedwithLeft (int)),
                 this,
                 SLOT (adjustFrequency_khz (int)));
        connect (hfScopeSlider, SIGNAL (valueChanged (int)),
                 this, SLOT (set_hfscopeLevel (int)));
//	else we start the handle
        connect (freqButton, SIGNAL (clicked (void)),
                 this, SLOT (handle_freqButton (void)));
	connect (freqSave, SIGNAL (clicked (void)),
                 this, SLOT (set_freqSave (void)));
        myList  = new programList (this, stationList);
        myList  -> show ();
	myLine	= NULL;
	connect (theDevice, SIGNAL (dataAvailable (int)),
	         this, SLOT (sampleHandler (int)));
	connect (frequencyBackwards, SIGNAL (clicked ()),
	         this, SLOT (handle_frequencyBackwards ()));
	connect (frequencyForwards, SIGNAL (clicked ()),
	         this, SLOT (handle_frequencyForwards ()));
	theDevice	-> restartReader ();

	if (decoderSelect -> currentText () == "drm+") 
	   theDecoder = new drmDecoder (this, bufferData, audioData);
	else
	   theDecoder = new fmDecoder (this, bufferData, audioData);
}


//	If the user quits before selecting a device ....
void	RadioInterface::handle_quitButton	(void) {
	delete theDecoder;
	if (theDevice != NULL) {
	   theDevice	-> stopReader ();
	   disconnect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	   delete  theDevice;
	}
	sleep (1);
	myList          -> saveTable ();
	myList		-> hide ();
}

//
void    RadioInterface::handle_freqButton (void) {
        if (mykeyPad -> isVisible ())
           mykeyPad -> hidePad ();
        else
           mykeyPad     -> showPad ();
}

//	setFrequency is called from the frequency panel
//	as well as from inside to change VFO and offset
void	RadioInterface::setFrequency (uint32_t frequency) {
uint32_t	VFOFrequency	= frequency;
	theDevice	-> setVFOFrequency (VFOFrequency);
	theBand. currentOffset	= 0;
	hfScope		-> setScope  (VFOFrequency, theBand. currentOffset);
	QString ff	= FrequencytoString (frequency);
	frequencyDisplay	-> display (ff);
}
//
//	adjustFrequency is called whenever clicking the mouse
void	RadioInterface::adjustFrequency_khz (int32_t n) {
	adjustFrequency_hz (1000 * n);
}

void	RadioInterface::adjustFrequency_hz (int32_t n) {
int	lowF	= theBand. lowF;
int	highF	= theBand. highF;
int	currOff	= theBand. currentOffset;

	int32_t newFreq = theDevice -> getVFOFrequency () +
	                                   theBand. currentOffset + n;
	setFrequency (newFreq);
	hfScope -> setScope (theDevice -> getVFOFrequency (), 0);

	QString ff		= FrequencytoString ((int32_t)
	                                    (theDevice -> getVFOFrequency ()));
	frequencyDisplay	-> display (ff);
}
//
void    RadioInterface::set_freqSave    (void) {
        if (myLine == NULL)
           myLine  = new QLineEdit ();
        myLine  -> show ();
        connect (myLine, SIGNAL (returnPressed (void)),
                 this, SLOT (handle_myLine (void)));
}

void    RadioInterface::handle_myLine (void) {
int32_t freq    = theDevice -> getVFOFrequency () + theBand. currentOffset;
QString programName     = myLine -> text ();
        myList  -> addRow (programName, QString::number (freq / Khz (1)));
        disconnect (myLine, SIGNAL (returnPressed (void)),
                    this, SLOT (handle_myLine (void)));
        myLine  -> hide ();
}

//////////////////////////////////////////////////////////////////
//
void	RadioInterface::sampleHandler (int amount) {
std::complex<float>   buffer [512];
int	i, j;

	(void)amount;
	while (inputData -> GetRingBufferReadAvailable () > 512) {
	   inputData	-> getDataFromBuffer (buffer, 512);
	   hfScope	-> addElements (buffer, 512);
	   bufferData -> putDataIntoBuffer (buffer, 512);
	}
}
//
//
void    RadioInterface::set_hfscopeLevel (int level) {
        hfScope -> setLevel (level);
}


void	RadioInterface::updateTime		(void) {
QDateTime currentTime = QDateTime::currentDateTime ();

	timeDisplay     -> setText (currentTime.
                                    toString (QString ("dd.MM.yy:hh:mm:ss")));
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

        QMessageBox::StandardButton resultButton =
                        QMessageBox::question (this, "dabRadio",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
        if (resultButton != QMessageBox::Yes) {
           event -> ignore();
        } else {
           handle_quitButton ();
           event -> accept ();
        }
}

//	do not forget that ocnt starts with 1, due
//	to Qt list conventions
void	RadioInterface::setupSoundOut (QComboBox	*streamOutSelector,
	                               audioSink	*our_audioSink,
	                               int32_t		cardRate,
	                               int16_t		*table) {
uint16_t	ocnt	= 1;
uint16_t	i;

	for (i = 0; i < our_audioSink -> numberofDevices (); i ++) {
	   const char *so =
	             our_audioSink -> outputChannelwithRate (i, cardRate);

	   if (so != NULL) {
	      streamOutSelector -> insertItem (ocnt, so, QVariant (i));
	      table [ocnt] = i;
	      ocnt ++;
	   }
	}

	qDebug () << "added items to combobox";
	if (ocnt == 1)
	   throw (22);
}

void    RadioInterface::processAudio  (int amount, int rate) {
std::complex<float> buffer [rate / 10];

        (void)amount;
//	usleep (1000);
        while (audioData -> GetRingBufferReadAvailable () >
                                                 (uint32_t)rate / 10) {
           audioData	-> getDataFromBuffer (buffer, rate / 10);
	   audioHandler      -> putSamples (buffer, rate / 10);
        }
}

deviceHandler	*RadioInterface::setDevice (const QString &s,
	                                   RingBuffer<std::complex<float>> *b) {
QString	file;
deviceHandler	*inputDevice	= nullptr;
///	OK, everything quiet, now let us see what to do
#ifdef	HAVE_AIRSPY
	if (s == "airspy") {
	   try {
	      inputDevice	= new airspyHandler (drmSettings, b);
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                           tr ("Airspy or Airspy mini not found\n"));
	      return nullptr;
	   }
	}
	else
#endif
#ifdef	HAVE_HACKRF
	if (s == "hackrf") {
	   try {
	      inputDevice	= new hackrfHandler (this, inputRate, b);
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                           tr ("hackrf not found\n"));
	      return nullptr;
	   }
	}
	else
#endif
#ifdef	HAVE_LIME
	if (s == "limeSDR") {
	   try {
	      inputDevice = new limeHandler (this, inputRate, b);
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                                  tr ("no lime device found\n"));
	      return nullptr;
	   }
	}
	else
#endif
#ifdef	HAVE_SDRPLAY_V2
	if (s == "sdrplay") {
	   try {
	      inputDevice	= new sdrplayHandler_v2 (settings,
	                                                 this, inputRate, b);
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("SDRplay: no library or device\n"));
	      return nullptr;
	   }
	}
	else
#endif
#ifdef	HAVE_SDRPLAY_V3
	if (s == "sdrplay-v3") {
	   try {
	      inputDevice	= new sdrplayHandler_v3 (drmSettings, b);
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("SDRplay: no library or device\n"));
	      return nullptr;
	   }
	}
	else
#endif
#ifdef	HAVE_RTLSDR
	if (s == "dabstick") {
	   try {
	      inputDevice	= new rtlsdrHandler (this, inputRate, b);
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                           tr ("DAB stick not found! Please use one with RTL2832U or similar chipset!\n"));
	      fprintf (stderr, "error = %d\n", e);
	      return nullptr;
	   }
	}
	else
#endif
	if (s == "wav files") {
	   try {
	      inputDevice	= new wavFiles (this, inputRate,  b);
	      freqButton	-> hide ();
	      mykeyPad		-> hide ();
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("file not found"));
	      return nullptr;
	   }
	}
	else
	if (s == "raw files 16")  {
	   try {
	      inputDevice	= new rawFiles_16 (this, inputRate, b);
	      freqButton	-> hide ();
	      mykeyPad		-> hide ();
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("file not found"));
	      return nullptr;
	   }
	}
	else
	if (s == "raw files") {
	   try {
	      inputDevice	= new rawFiles_8 (this, inputRate, b);
	      freqButton	-> hide ();
	      mykeyPad		-> hide ();
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("file not found"));
	      return nullptr;
	   }
	}
	else
	if (s == "raw files 32") {
	   try {
	      inputDevice	= new rawFiles_32 (this, inputRate, b);
	      freqButton	-> hide ();
	      mykeyPad		-> hide ();
	   }
	   catch (int e) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("file not found"));
	      return nullptr;
	   }
	}
	else {
	   fprintf (stderr, "unknown device, failing\n");
	   return nullptr;
	}
	return inputDevice;
}

void	RadioInterface::handle_decoderSelect (const QString &s) {
	if (theDevice == nullptr) 
	   return;		// nothing started yet
	theDevice	-> stopReader ();

	delete theDecoder;
	if (s == "drm+")
	   theDecoder = new drmDecoder (this, bufferData, audioData);
	else
	   theDecoder = new fmDecoder (this, bufferData, audioData);

	theDevice	-> restartReader ();
}

void	RadioInterface::handle_frequencyBackwards () {
	adjustFrequency_khz (-50);
}

void	RadioInterface::handle_frequencyForwards () {
	adjustFrequency_khz (50);
}

