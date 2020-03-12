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

#include	"drm-decoder.h"
#include	"radio.h"
#include	"frame-handler.h"
#include	"eqdisplay.h"
#include	"iqdisplay.h"

	drmDecoder::drmDecoder	(RadioInterface * parent,
	                         RingBuffer<std::complex<float>> *input,
	                         RingBuffer<std::complex<float>> *audioBuffer) {
	this	-> audioBuffer	= audioBuffer;
	myFrame			= new QFrame (nullptr);
	setupUi (myFrame);
//	techFrame		= new QFrame (nullptr);
	audioFrame		= new QFrame (nullptr);
	audioData. setupUi (audioFrame);

	my_eqDisplay		= new EQDisplay ();
	my_iqDisplay		= new IQDisplay ();
	myFrame                 -> show ();

	timeSyncLabel   -> setStyleSheet ("QLabel {background-color:red}");
	facSyncLabel    -> setStyleSheet ("QLabel {background-color:red}");
	sdcSyncLabel    -> setStyleSheet ("QLabel {background-color:red}");
	faadSyncLabel   -> setStyleSheet ("QLabel {background-color:red}");

	this    -> eqBuffer     = new RingBuffer<std::complex<float>> (32768);
	this    -> iqBuffer     = new RingBuffer<std::complex<float>> (1024);

	my_frameHandler 	= new frameHandler (this,
	                                            input,
	                                            &params,
	                                            eqBuffer, iqBuffer);
	phaseOffset		= 0;
	connect (equalizerButton, SIGNAL (clicked ()),
	         this, SLOT (handle_equalizerButton ()));
	connect (streamData_button, SIGNAL (clicked ()),
	         this, SLOT (handle_streamData ()));
	connect (this, SIGNAL (audioAvailable (int, int)),
	         parent, SLOT (processAudio (int, int)));
	connect (constellationSelector,
	                          SIGNAL (activated (const QString &)),	
	         this, SLOT (handle_constellationSelector (const QString &)));
}

	drmDecoder::~drmDecoder () {
	delete	my_frameHandler;
	delete	myFrame;
	delete	audioFrame;
	my_iqDisplay	-> hide ();
	my_eqDisplay	-> hide ();
//	delete	my_iqDisplay;
//	delete	my_eqDisplay;
}

void	drmDecoder::show_coarseOffset	(float offset) {
	show_int_offset	-> display (offset);
}

void	drmDecoder::show_inputShift	(int shift) {
	inputShiftDisplay -> display (shift);
}

void	drmDecoder::show_angle		(float angle) {
	angleDisplay	-> display (angle);
}

void	drmDecoder::show_timeOffset	(float offset) {
	timeOffsetDisplay	-> display (offset);
}

void	drmDecoder::show_timeDelay	(float del) {
	timeDelayDisplay	-> display (del);
}

void	drmDecoder::show_clockOffset	(float o) {
	clockOffsetDisplay	-> display (o);
}

void	drmDecoder::showMessage (QString m) {
	messageLabel -> setText (m);
}

void	drmDecoder::setTimeSync	(bool f) {
	if (f)
	   timeSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else {
	   timeSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	}
}

void	drmDecoder::setFACSync	(bool f) {
	if (f)
	   facSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else {
	   facSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	}
}

void	drmDecoder::executeSDCSync	(bool f) {
	if (f)
	   sdcSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else {
	   sdcSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	}
}

void	drmDecoder::show_stationLabel (const QString &s) {
	stationLabel -> setText (s);
}

void	drmDecoder::show_audioMode	(QString s) {
	audioModeLabel	-> setText (s);
}

void	drmDecoder::show_mer		(float mer) {
	mer_display	-> display (mer);
}

static std::complex<float> lbuf [4800];
static int fillP        = 0;
void    drmDecoder::sampleOut           (float re, float im) {
std::complex<float> z   = std::complex<float> (re, im);
	lbuf [fillP] = z;
	fillP ++;
	if (fillP >= 4800) {
	   audioBuffer     -> putDataIntoBuffer (lbuf, 4800);
	   audioAvailable (audioBuffer -> GetRingBufferReadAvailable (), 48000);
	   fillP = 0;
	}
}

void	drmDecoder::showSNR		(float snr) {
	snrDisplay	-> display (snr);
}

void	drmDecoder::selectChannel_0	(void) {
int hp_sum	= 0;
	stationLabel	-> setText (params. subChannels [0]. serviceName);
	show_audioData (&params, 0);
	my_frameHandler	-> selectService (0);
}

void	drmDecoder::selectChannel_1	(void) {
	stationLabel	-> setText (params. subChannels [1]. serviceName);
//	we compute the offset of the lower protected part;
	show_audioData (&params, 1);
	my_frameHandler	-> selectService (1);
}

void	drmDecoder::selectChannel_2	(void) {
	stationLabel	-> setText (params. subChannels [2]. serviceName);
	show_audioData (&params, 2);
	my_frameHandler	-> selectService (2);
}

void	drmDecoder::selectChannel_3	(void) {
	stationLabel	-> setText (params. subChannels [3]. serviceName);
	show_audioData (&params, 3);
	my_frameHandler	-> selectService (3);
}

static	int	faadCounter	= 0;
static	int	goodfaad	= 0;
void	drmDecoder::faadSuccess		(bool b) {
	faadCounter ++;
	if (b)
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:green}");
	else
	   faadSyncLabel -> setStyleSheet ("QLabel {background-color:red}");
	if (b)
	   goodfaad ++;
	if (faadCounter > 500) {
	   fprintf (stderr, "faad ratio is %f\n", (float)goodfaad / faadCounter);
	   goodfaad	= 0;
	   faadCounter	= 0;
	}
}

void	drmDecoder::aacData (QString text) {
	aacDataLabel -> setText (text);
}

//	showMOT is triggered by the MOT handler,
//	the GUI may decide to ignore the data sent
//	since data is only sent whenever a data channel is selected
void	drmDecoder::showMOT		(QByteArray data, int subtype) {
	if (!running)
	   return;

	QPixmap p;
	p. loadFromData (data, subtype == 0 ? "GIF" :
	                       subtype == 1 ? "JPEG" :
	                       subtype == 2 ? "BMP" : "PNG");
//	pictureLabel ->  setPixmap (p);
//	pictureLabel ->  show ();
}

void    drmDecoder::set_phaseOffset (int f) {
	phaseOffset += f;
//	phaseOffsetDisplay	-> display (phaseOffset);
}

void	drmDecoder::show_country (QString s) {
	countryLabel	-> setText (s);
}

void	drmDecoder::show_time	(QString s) {
	timeLabel		-> setText (s);
}

void	drmDecoder::show_eqsymbol	(int amount) {
int xx	= eqBuffer -> GetRingBufferReadAvailable ();
std::complex<float> line [xx];

	eqBuffer	-> getDataFromBuffer (line, xx);
	my_eqDisplay	-> show (line, xx);
}

void	drmDecoder::show_iq		() {
std::complex<float> Values [100];
std::complex<float> pixels [5 * 96];
int16_t i;
int16_t t;
double  avg     = 0;
	while (iqBuffer -> GetRingBufferReadAvailable () >= 100) {
	   t = iqBuffer -> getDataFromBuffer (Values, 100);
	   for (i = 0; i < t; i ++) {
	      float x = abs (Values [i]);
	      if (!std::isnan (x) && !std::isinf (x))
	         avg += abs (Values [i]);
	   }

	   for (i = 0; i < t; i ++) {
	      pixels [i] = cdiv (Values [i], avg / t);
	   }
	   avg     /= t;
	   my_iqDisplay -> DisplayIQ (pixels, avg);
	}
}

void	drmDecoder::cleanup_db		() {
	params. theChannel. nrAudioServices = 0;
	params. theChannel. nrDataServices = 0;
	params. hours	= -1;
	for (int i = 0; i < 4; i ++) {
	   params. subChannels [i]. inUse	= false;
	   params. subChannels [i]. serviceName = "";
	}
	
	disconnect (channel_0, SIGNAL (clicked (void)),
	            this, SLOT (selectChannel_0 (void)));
	channel_0	-> setText ("not available");
	disconnect (channel_1, SIGNAL (clicked (void)),
	            this, SLOT (selectChannel_1 (void)));
	channel_1	-> setText ("not available");
	disconnect (channel_2, SIGNAL (clicked (void)),
	            this, SLOT (selectChannel_2 (void)));
	channel_2	-> setText ("not available");
	disconnect (channel_3, SIGNAL (clicked (void)),
	            this, SLOT (selectChannel_3 (void)));
	channel_3	-> setText ("not available");
}

QString monthTable [] = {"jan", "feb", "mar", "apr", "may", "jun",
	                 "jul", "aug", "sep", "oct", "nov", "dec"};

bool	channels [] = {false, false, false, false};

void	drmDecoder::update_GUI		() {
	if (params. subChannels [0]. inUse) {
	   channel_0 -> setText (params. subChannels [0]. serviceName);
	   if (!channels [0]) { 
	      connect (channel_0, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_0 ()));
	      channels [0] = true;
	   }
	}

	if (params. subChannels [1]. inUse) {
	   channel_1 -> setText (params. subChannels [1]. serviceName);
	   if (!channels [1]) {
	      connect (channel_1, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_1 ()));
	      channels [1] = true;
	   }
	}

	if (params. subChannels [2]. inUse) {
	   channel_2 -> setText (params. subChannels [2]. serviceName);
	   if (!channels [2]) {
	      connect (channel_2, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_2 ()));
	      channels [2] = true;
	   }
	}

	if (params. subChannels [3]. inUse) {
	   channel_3 -> setText (params. subChannels [3]. serviceName);
	   if (!channels [3]) {
	      connect (channel_3, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_3 ()));
	      channels [3] = true;
	   }
	}

	if (params. hours >= 0) {
	   QString dd = QString::number (params.Year) + ":" +
	                monthTable [params. Month - 1] + ":" +
	                QString::number (params. Day) + " " +
	                QString::number (params. hours) + ":" +
	                QString::number (params. minutes);
	   timeLabel -> setText (dd);
	}
	else
	   timeLabel	-> setText (" ");
}

void	drmDecoder::show_audioData (drmParameters *drm, int shortId) {
streamParameters *theStream;
int	streamId	= 0;

	if (!drm -> subChannels [shortId]. is_audioService) {
	   fprintf (stderr, "Sorry, not an audio channel\n");
	   return;
	}

	for (int i = 0; i < 4; i ++) 
	   if (drm -> theStreams [i]. inUse &&
	       drm -> theStreams [i]. audioStream &&
	       (drm -> theStreams [i]. shortId == shortId)) {
	      streamId = i;
	      break;
	}
	theStream	= &(drm	-> theStreams [streamId]);
	audioData. streamId	-> display (streamId);
	audioData. serviceName	->
	           setText (drm -> subChannels [shortId].serviceName);
	audioData. QamLabel	-> setText (drm -> theChannel. MSC_Mode == 0 ?
	                                    "16 QAM" : "4 QAM");
	audioData. protection_High -> display (drm -> protLevelA);
	audioData. protection_Low  -> display (drm -> protLevelB);
	audioData. prot_A_Length -> display (theStream -> lengthHigh);
	audioData. prot_B_Length -> display (theStream -> lengthLow);
	audioData. start_High	-> display  (theStream -> offsetHigh);
	audioData. start_Low	-> display  (theStream -> offsetLow);
	audioData. samplingrateDisplay	-> display (
	              theStream -> audioCoding == 0 ?
	                  theStream -> audioSamplingRate == 1 ? 12000:
	                  theStream -> audioSamplingRate == 3 ? 24000: 48000 :
	                  theStream -> audioSamplingRate == 0 ? 9600 :
	                  theStream -> audioSamplingRate == 1 ? 12000  :
	                  theStream -> audioSamplingRate == 2 ? 16000 :
	                  theStream -> audioSamplingRate == 3 ? 19200 :
	                  theStream -> audioSamplingRate == 4 ? 24000 :
	                  theStream -> audioSamplingRate == 5 ? 32000 :
	                  theStream -> audioSamplingRate == 6 ? 38400 :
	                  theStream -> audioSamplingRate == 7 ? 48000 : -1);
	if (drm -> subChannels [shortId]. is_audioService) {
	   int programType = drm -> subChannels [shortId]. serviceDescriptor;
	   programTypeLabel	-> setText (getProgramType (programType));
	}
	fprintf (stderr, "stream contains %smessage\n",
	              theStream -> textFlag ? "a " : "no ");
}

void	drmDecoder::handle_constellationSelector (const QString &s) {
	if (s == "no tech") {
	   my_iqDisplay	-> hide ();
	   return;
	}
	my_iqDisplay	-> show ();
	my_frameHandler	-> set_constellationView (s);
}

void	drmDecoder::handle_equalizerButton () {
	if (my_eqDisplay -> isHidden ())
	   my_eqDisplay -> show ();
	else
	   my_eqDisplay -> hide ();
}

void	drmDecoder::handle_streamData		() {
	if (audioFrame -> isHidden ())
	   audioFrame -> show ();
	else
	   audioFrame -> hide ();
}


QString	drmDecoder::getProgramType	(int programType) {
	switch (programType) {
	   case 0:     return "No programme type";
	   case 1:     return "News";
	   case 2:     return "Current Affairs";
	   case 3:     return "Information";
	   case 4:     return "Sport";
	   case 5:     return "Education";
	   case 6:     return "Drama";
	   case 7:     return "Culture";
	   case 8:     return "Science";
	   case 9:     return "Varied";    //Talk
	   case 10:    return "Pop Music";
	   case 11:    return "Rock Music";
	   case 12:    return "Easy Listening Music";
	   case 13:    return "Light Classical";
	   case 14:    return "Serious Classical";
	   case 15:    return "Other Music";
	   case 16:    return "Weather/meteorology";
	   case 17:    return "Finance/Business";
	   case 18:    return "Children's programmes";
	   case 19:    return "Social Affairs";    //Factual
	   case 20:    return "Religion";
	   case 21:    return "Phone In";
	   case 22:    return "Travel";
	   case 23:    return "Leisure";
	   case 24:    return "Jazz Music";
	   case 25:    return "Country Music";
	   case 26:    return "National Music";
	   case 27:    return "Oldies Music";
	   case 28:    return "Folk Music";
	   case 29:    return "Documentary";
	   case 30:    return "unknown programme type 30";
	   case 31:    return "unknown programme type 31";
	   default:    return "unknown programme type";
	}
}
