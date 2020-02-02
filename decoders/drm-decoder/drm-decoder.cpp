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
	techFrame		= new QFrame (nullptr);
	audioFrame		= new QFrame (nullptr);
	techData. setupUi (techFrame);
	audioData. setupUi (audioFrame);

        my_eqDisplay		= new EQDisplay (techData.
	                                              equalizerDisplay);
        my_iqDisplay		= new IQDisplay (techData.
	                                              iq_sampleDisplay, 128);
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
	connect (techData_button, SIGNAL (clicked ()),
	         this, SLOT (handle_techData ()));
	connect (streamData_button, SIGNAL (clicked ()),
	         this, SLOT (handle_streamData ()));
	connect (this, SIGNAL (audioAvailable (int, int)),
	         parent, SLOT (processAudio (int, int)));
	connect (techData. constellationSelector,
	                          SIGNAL (activated (const QString &)),	
	         this, SLOT (handle_constellation (const QString &)));
}

	drmDecoder::~drmDecoder () {
        delete	my_frameHandler;
	delete	myFrame;
	delete	techFrame;
	delete	audioFrame;
	my_iqDisplay	-> hide ();
//	delete	my_iqDisplay;
	delete	my_eqDisplay;
}

void	drmDecoder::show_coarseOffset (float offset) {
	show_int_offset	-> display (offset);
}

void	drmDecoder::show_fineOffset (float offset) {
	show_small_offset -> display (offset);
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
	stationLabel	-> setText (params. theStreams [0]. serviceName);
//
//	we compute the offset of the lower protected part;
	for (int i = 0; i < 4; i ++)
	   if (params. theStreams [i]. inUse)
	      hp_sum += params. theStreams [i]. lengthHigh;
	params. theStreams [0]. offsetHigh = 0;
	params. theStreams [0]. offsetLow  = hp_sum;
	show_audioData (&params, 0);
	my_frameHandler	-> selectService (0);
}

void	drmDecoder::selectChannel_1	(void) {
int hp_sum	= 0;
	stationLabel	-> setText (params. theStreams [1]. serviceName);
//	we compute the offset of the lower protected part;
	for (int i = 0; i < 4; i ++)
	   if (params. theStreams [i]. inUse)
	      hp_sum += params. theStreams [i]. lengthHigh;
	params. theStreams [0]. offsetHigh = 0;
	params. theStreams [0]. offsetLow  = hp_sum;
	params. theStreams [1]. offsetHigh = 
	                     params. theStreams [0]. offsetHigh +
	                     params. theStreams [0]. lengthHigh;
	params. theStreams [1]. offsetLow =
	                     params. theStreams [0]. offsetLow +
	                     params. theStreams [0]. lengthLow;
	show_audioData (&params, 1);
	my_frameHandler	-> selectService (1);
}

void	drmDecoder::selectChannel_2	(void) {
int hp_sum	= 0;
	stationLabel	-> setText (params. theStreams [2]. serviceName);
	for (int i = 0; i < 4; i ++)
	   if (params. theStreams [i]. inUse)
	      hp_sum += params. theStreams [i]. lengthHigh;
	params. theStreams [0]. offsetHigh = 0;
	params. theStreams [0]. offsetHigh  = hp_sum;
	params. theStreams [1]. offsetHigh = 
	                     params. theStreams [0]. offsetHigh +
	                     params. theStreams [0]. lengthHigh;
	params. theStreams [1]. offsetLow =
	                     params. theStreams [0]. offsetLow +
	                     params. theStreams [0]. lengthLow;
	params. theStreams [2]. offsetHigh = 
	                     params. theStreams [1]. offsetHigh +
	                     params. theStreams [1]. lengthHigh;
	params. theStreams [2]. offsetLow =
	                     params. theStreams [1]. offsetLow +
	                     params. theStreams [1]. lengthLow;
	show_audioData (&params, 2);
	my_frameHandler	-> selectService (2);
}

void	drmDecoder::selectChannel_3	(void) {
int hp_sum	= 0;
	stationLabel	-> setText (params. theStreams [1]. serviceName);
	for (int i = 0; i < 4; i ++)
	   if (params. theStreams [i]. inUse)
	      hp_sum += params. theStreams [i]. lengthHigh;
	params. theStreams [0]. offsetHigh = 0;
	params. theStreams [0]. offsetHigh  = hp_sum;
	params. theStreams [1]. offsetHigh = 
	                     params. theStreams [0]. offsetHigh +
	                     params. theStreams [0]. lengthHigh;
	params. theStreams [1]. offsetLow =
	                     params. theStreams [0]. offsetLow +
	                     params. theStreams [0]. lengthLow;
	params. theStreams [2]. offsetHigh = 
	                     params. theStreams [1]. offsetHigh +
	                     params. theStreams [1]. lengthHigh;
	params. theStreams [2]. offsetLow =
	                     params. theStreams [1]. offsetLow +
	                     params. theStreams [1]. lengthLow;
	params. theStreams [3]. offsetHigh = 
	                     params. theStreams [2]. offsetHigh +
	                     params. theStreams [2]. lengthHigh;
	params. theStreams [3]. offsetLow =
	                     params. theStreams [2]. offsetLow +
	                     params. theStreams [2]. lengthLow;
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

void	drmDecoder::show_programType (QString s) {
	programTypeLabel	-> setText (s);
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
std::complex<float> Values [128];
std::complex<float> pixels [5 * 128];
int16_t i;
int16_t t;
double  avg     = 0;
int     scopeWidth      = techData. scopeSlider -> value();
float	pixS	= 1.0 / 128;
	while (iqBuffer -> GetRingBufferReadAvailable () >= 128) {
           t = iqBuffer -> getDataFromBuffer (Values, 128);
           for (i = 0; i < t; i ++) {
              float x = abs (Values [i]);
              if (!std::isnan (x) && !std::isinf (x))
              avg += abs (Values [i]);
           }

	   for (i = 0; i < t; i ++) {
	      pixels [5 * i] = std::complex<float> (real (Values [i]) - pixS,
	                                            imag (Values [i]));
	      pixels [5 * i + 1] = std::complex<float> (real (Values [i]) - pixS,
	                                                imag (Values [i]) - pixS);
	      pixels [5 * i + 2] = std::complex<float> (real (Values [i]),
	                                                imag (Values [i]));
	      pixels [5 * i + 3] = std::complex<float> (real (Values [i]) + pixS,
	                                                imag (Values [i]));
	      pixels [5 * i + 4] = std::complex<float> (real (Values [i]) + pixS,
	                                                imag (Values [i]) + pixS);
	   }
           avg     /= t;
           my_iqDisplay -> DisplayIQ (pixels, scopeWidth / avg);
	}
}

void	drmDecoder::cleanup_db		() {
	params. theChannel. nrServices = 0;
	params. hours	= -1;
	for (int i = 0; i < 4; i ++) {
	   params. theStreams [i]. inUse	= false;
	   params. theStreams [i]. serviceName = "";
	   params. theStreams [i]. programType = "";
	   params. theStreams [i]. languagetxt = "";
	   params. theStreams [i]. country	= "";
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
	if (params. theStreams [0]. inUse) {
	   channel_0 -> setText (params. theStreams [0]. serviceName);
	   if (!channels [0]) { 
	      connect (channel_0, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_0 ()));
	      channels [0] = true;
	   }
	}

	if (params. theStreams [1]. inUse) {
	   channel_1 -> setText (params. theStreams [1]. serviceName);
	   if (!channels [1]) {
	      connect (channel_1, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_1 ()));
	      channels [1] = true;
	   }
	}

	if (params. theStreams [2]. inUse) {
	   channel_2 -> setText (params. theStreams [2]. serviceName);
	   if (!channels [2]) {
	      connect (channel_2, SIGNAL (clicked ()),
	               this, SLOT (selectChannel_2 ()));
	      channels [2] = true;
	   }
	}

	if (params. theStreams [3]. inUse) {
	   channel_3 -> setText (params. theStreams [3]. serviceName);
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

void	drmDecoder::show_audioData (drmParameters *drm, int subchId) {
streamParameters *sc 	= &(drm -> theStreams [subchId]);
	if (!sc -> is_audio) {
	   fprintf (stderr, "Sorry, not an audio channel\n");
	   return;
	}

	audioData. streamId	-> display (sc -> streamId);
	audioData. serviceName	-> setText (sc -> serviceName);
	audioData. QamLabel	-> setText (drm -> theChannel. MSC_Mode == 0 ?
	                                    "16 QAM" : "4 QAM");
	audioData. protection_High -> display (drm -> protLevelA);
	audioData. protection_Low  -> display (drm -> protLevelB);
	audioData. prot_A_Length -> display (sc -> lengthHigh);
	audioData. prot_B_Length -> display (sc -> lengthLow);
	audioData. start_High	-> display (sc -> offsetHigh);
	audioData. start_Low	-> display (sc -> offsetLow);
	audioData. samplingrateDisplay	-> display (
	              sc -> audioCoding == 0 ?
	                  sc -> audioSamplingRate == 1 ? 12000:
	                  sc -> audioSamplingRate == 3 ? 24000: 48000 :
	                  sc -> audioSamplingRate == 0 ? 9600 :
	                  sc -> audioSamplingRate == 1 ? 12000  :
	                  sc -> audioSamplingRate == 2 ? 16000 :
	                  sc -> audioSamplingRate == 3 ? 19200 :
	                  sc -> audioSamplingRate == 4 ? 24000 :
	                  sc -> audioSamplingRate == 5 ? 32000 :
	                  sc -> audioSamplingRate == 6 ? 38400 :
	                  sc -> audioSamplingRate == 7 ? 48000 : -1);
	fprintf (stderr, "audioCoding %s\n",
	              sc -> audioCoding == 0 ? "aac" :
	              sc -> audioCoding == 3 ? "xHE-AAC" : "????");
	fprintf (stderr, "SBR %s used\n",
	              sc -> sbrFlag ? "": "not");
	fprintf (stderr, "audioMode %s\n",
	              sc -> audioMode == 0 ? "mono" :
	              sc -> audioMode == 1 ? "parametric stereo" :
	              sc -> audioMode == 2 ? "stereo" : "???");
	fprintf (stderr, "audioSamplingRate %s\n",
	              sc -> audioCoding == 0 ?
	                  sc -> audioSamplingRate == 1 ? "12 KHz":
	                  sc -> audioSamplingRate == 3 ? "24 KHz": "48 KHz" :
	                  sc -> audioSamplingRate == 0 ? "9,6 KHz" :
	                  sc -> audioSamplingRate == 1 ? "12 KHz"  :
	                  sc -> audioSamplingRate == 2 ? "16 KHz" :
	                  sc -> audioSamplingRate == 3 ? "19,2 KHz" :
	                  sc -> audioSamplingRate == 4 ? "24 KHz" :
	                  sc -> audioSamplingRate == 5 ? "32 KHz" :
	                  sc -> audioSamplingRate == 6 ? "38,4 KHz" :
	                  sc -> audioSamplingRate == 7 ? "48 KHz" : "???");
	fprintf (stderr, "stream contains %smessage\n",
	              sc -> textFlag ? "a " : "no ");
	if (!sc -> languagetxt. isEmpty ())
	   fprintf (stderr, "Language %s\n", sc -> languagetxt. toLatin1 (). data ());
	if (!sc -> country. isEmpty ())
	   fprintf (stderr, "Country %s\n", sc -> country. toLatin1 (). data ());
	if (!sc -> programType. isEmpty ())
	   fprintf (stderr, "Program type %s\n", 
	                            sc -> programType. toLatin1 (). data ());
}

void	drmDecoder::handle_techData	() {
	if (techFrame -> isHidden ())
	   techFrame -> show ();
	else
	   techFrame -> hide ();
}

void	drmDecoder::handle_streamData		() {
	if (audioFrame -> isHidden ())
	   audioFrame -> show ();
	else
	   audioFrame -> hide ();
}

void	drmDecoder::handle_constellation	(const QString &s) {
	my_frameHandler	-> set_constellationView (s);
}

