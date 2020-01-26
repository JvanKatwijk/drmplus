#
/*
 *    Copyright (C) 2014 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *	with contributions from Fabio Capozzi
 *
 *    This file is part of swradio-8
 *
 *    swradio-8 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    swradio-8 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with swradio-8 if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QHBoxLayout>
#include	<QLabel>
#include	<QDebug>
#include	"hackrf-handler.h"

#define	DEFAULT_GAIN	30
static float convTable [256];

	hackrfHandler::hackrfHandler	(RadioInterface *mr,
	                                 int32_t        outputRate,
	                                 RingBuffer<std::complex<float>> *r):
	                                              deviceHandler (mr) {
int	err;
int	res;
int	i;

	this    -> outputRate		= outputRate;
	this	-> myFrame		= new QFrame (NULL);
	setupUi (this -> myFrame);
	this	-> myFrame	-> show ();
	this	-> inputRate		= Khz (12 * outputRate);
	this	-> decimationFactor	= inputRate / outputRate;
	_I_Buffer			= r;

	for (i = 0; i < 128; i ++)
	   convTable [i] = (float)(i) / 128.0;
	for (i = 0; i < 128; i ++)
	   convTable [128 + i] =  -(float)(128 - i) / 128.0;

#ifdef  __MINGW32__
	const char *libraryString = "libhackrf.dll";
	Handle          = LoadLibrary ((wchar_t *)L"libhackrf.dll");
#else
	const char *libraryString = "libhackrf.so";
	Handle          = dlopen (libraryString, RTLD_NOW);
#endif

	if (Handle == NULL) {
	   fprintf (stderr, "failed to open %s\n", libraryString);
	   delete myFrame;
	   throw (20);
	}

	libraryLoaded   = true;
	if (!load_hackrfFunctions ()) {
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   delete myFrame;
	   throw (21);
	}
//
	lastFrequency	= Khz (220000);
//
	res	= this -> hackrf_init ();
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_init:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (21);
	}

	res	= this	-> hackrf_open (&theDevice);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_open:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (22);
	}

	res	= this -> hackrf_set_sample_rate (theDevice, inputRate);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_samplerate:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (23);
	}

	res	= this -> hackrf_set_baseband_filter_bandwidth (theDevice,
	                                                        1536000);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_bw:");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (24);
	}

	res	= this -> hackrf_set_freq (theDevice, 220000000);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_freq: ");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (25);
	}

	res = this -> hackrf_set_antenna_enable (theDevice, 1);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_antenna_enable: ");
	   fprintf (stderr, "%s \n",
	                this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (26);
	}

	res = this -> hackrf_set_amp_enable (theDevice, 1);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_antenna_enable: ");
	   fprintf (stderr, "%s \n",
	                this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (27);
	}

	uint16_t regValue;
	res = this -> hackrf_si5351c_read (theDevice, 162, &regValue);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_si5351c_read: ");
	   fprintf (stderr, "%s \n",
	             this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (28);
	}

	res = this -> hackrf_si5351c_write (theDevice, 162, regValue);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_si5351c_write: ");
	   fprintf (stderr, "%s \n",
	              this -> hackrf_error_name (hackrf_error (res)));
	   delete myFrame;
	   throw (29);
	}


	oscillatorTable = new std::complex<float> [inputRate];
	for (int32_t i = 0; i < inputRate; i ++)
	   oscillatorTable [i] = std::complex<float> (
	                           cos ((float) i * 2 * M_PI / inputRate),
	                           sin ((float) i * 2 * M_PI / inputRate));
	localShift      = 0;
	oscillatorPhase = 0;
	filter          = new decimatingFIR (inputRate / outputRate * 5 - 1,
	                                     + outputRate / 2,
	                                     inputRate,
	                                     inputRate / outputRate);

	setLNAGain	(lnagainSlider	-> value ());
	setVGAGain	(vgagainSlider	-> value ());
	EnableAntenna   (1);            // value is a dummy really
	EnableAmpli     (1);            // value is a dummy, really
	set_ppmCorrection (ppm_correction       -> value ());

//	and be prepared for future changes in the settings
	connect (lnagainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setLNAGain (int)));
	connect (vgagainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setVGAGain (int)));

	connect (AntEnableButton, SIGNAL (stateChanged (int)),
	         this, SLOT (EnableAntenna (int)));
	connect (AmpEnableButton, SIGNAL (stateChanged (int)),
	         this, SLOT (EnableAmpli (int)));
	connect (ppm_correction, SIGNAL (valueChanged (int)),
	         this, SLOT (set_ppmCorrection  (int)));

	hackrf_device_list_t *deviceList = this -> hackrf_device_list ();
	if (deviceList != NULL) {
	   char *serial = deviceList -> serial_numbers [0];
	   serial_number_display -> setText (serial);
	   enum hackrf_usb_board_id board_id =
	                 deviceList -> usb_board_ids [0];
	   usb_board_id_display ->
	                setText (this -> hackrf_usb_board_id_name (board_id));
	}

	running. store (false);
}

	hackrfHandler::~hackrfHandler	(void) {
	stopReader ();
	delete myFrame;
	this	-> hackrf_close (theDevice);
	this	-> hackrf_exit ();
}
//

void	hackrfHandler::setVFOFrequency	(int32_t newFrequency) {
int	res;
	if (newFrequency < Mhz (1))
	   return;
	res	= this -> hackrf_set_freq (theDevice, newFrequency);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_freq: \n");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
//
//      It seems that after changing the frequency, the preamp is switched off
	if (AmpEnableButton -> checkState () == Qt::Checked)
	   EnableAmpli (1);

	vfoFrequency = newFrequency;
}

int32_t	hackrfHandler::getVFOFrequency	(void) {
	return vfoFrequency;
}

void	hackrfHandler::setLNAGain	(int newGain) {
int	res;
	if ((newGain <= 40) && (newGain >= 0)) {
	   res	= this -> hackrf_set_lna_gain (theDevice, newGain);
	   if (res != HACKRF_SUCCESS) {
	      fprintf (stderr, "Problem with hackrf_lna_gain :\n");
	      fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	      return;
	   }
	   lnagainDisplay	-> display (newGain);
	}
}

void	hackrfHandler::setVGAGain	(int newGain) {
int	res;
	if ((newGain <= 62) && (newGain >= 0)) {
	   res	= this -> hackrf_set_vga_gain (theDevice, newGain);
	   if (res != HACKRF_SUCCESS) {
	      fprintf (stderr, "Problem with hackrf_vga_gain :\n");
	      fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	      return;
	   }
	   vgagainDisplay	-> display (newGain);
	}
}

void    hackrfHandler::EnableAntenna (int d) {
int res;
bool    b;

	(void)d;
	b = AntEnableButton     -> checkState () == Qt::Checked;
	res = this -> hackrf_set_antenna_enable (theDevice, b);
//      fprintf(stderr,"Passed %d\n",(int)b);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_antenna_enable :\n");
	   fprintf (stderr, "%s \n",
	                    this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
//      AntEnableButton -> setChecked (b);
}

void    hackrfHandler::EnableAmpli (int a) {
int res;
bool    b;

	(void)a;
	b = AmpEnableButton     -> checkState () == Qt::Checked;
	res = this -> hackrf_set_amp_enable (theDevice, b);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_set_amp_enable :\n");
	   fprintf (stderr, "%s \n",
	                   this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
//      AmpEnableButton->setChecked (b);
}

//      correction is in Hz
// This function has to be modified to implement ppm correction
// writing in the si5351 register does not seem to work yet
// To be completed

void    hackrfHandler::set_ppmCorrection        (int32_t ppm) {
int res;
uint16_t value;

	res = this -> hackrf_si5351c_write (theDevice,
	                                    162,
	                                    static_cast<uint16_t>(ppm));
	res = this -> hackrf_si5351c_read (theDevice,
	                                   162, &value);
	(void) res;
	qDebug() << "Read si5351c register 162 : " << value <<"\n";
}

//
static
int	callback (hackrf_transfer *transfer) {
hackrfHandler *ctx = static_cast <hackrfHandler *>(transfer -> rx_ctx);
uint8_t *p	= transfer -> buffer;
int	i;
RingBuffer<std::complex<float> > * q = ctx -> _I_Buffer;
std::complex<float> localBuf [transfer -> valid_length / (2 * ctx -> decimationFactor) + 10];
int	cnt	= 0;

	for (i = 0; i < transfer -> valid_length / 2; i ++) {
	   int8_t re = (int8_t)(p [2 * i]);
	   int8_t im = (int8_t)(p [2 * i + 1]);
	   std::complex<float> temp  =
	              std::complex<float> (((float)re) / 128.0,
	                                   ((float)im) / 128.0);

	   if (ctx -> filter -> Pass (temp, &(localBuf [cnt])))
	      if (localBuf [cnt] == localBuf [cnt])
	         cnt ++;
	}

	ctx -> _I_Buffer -> putDataIntoBuffer (localBuf, cnt);
	ctx -> sampleCnt += cnt;

	if (ctx -> sampleCnt > ctx -> outputRate / 8) {
	   ctx -> report_dataAvailable ();
	   ctx -> sampleCnt = 0;
	}
	return 0;
}

bool	hackrfHandler::restartReader	(void) {
int	res;

	if (running. load ())
	   return true;

	res	= this -> hackrf_start_rx (theDevice, callback, this);	
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_start_rx :\n");
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return false;
	}
	running. store (this -> hackrf_is_streaming (theDevice));
	sampleCnt	= 0;
	fprintf (stderr, "hackrf is %s running\n",
	                        running. load () ? " " : "not");
	return running. load ();
}

void	hackrfHandler::stopReader	(void) {
int	res;

	if (!running. load ())
	   return;

	res	= this -> hackrf_stop_rx (theDevice);
	if (res != HACKRF_SUCCESS) {
	   fprintf (stderr, "Problem with hackrf_stop_rx :\n", res);
	   fprintf (stderr, "%s \n",
	                 this -> hackrf_error_name (hackrf_error (res)));
	   return;
	}
	running. store (false);
}

int16_t	hackrfHandler::bitDepth	(void) {
	return 8;
}

int32_t hackrfHandler::getRate  (void) {
	return Khz (2112);
}

bool	hackrfHandler::load_hackrfFunctions (void) {
//
//	link the required procedures
	this -> hackrf_init	= (pfn_hackrf_init)
	                       GETPROCADDRESS (Handle, "hackrf_init");
	if (this -> hackrf_init == NULL) {
	   fprintf (stderr, "Could not find hackrf_init\n");
	   return false;
	}

	this -> hackrf_open	= (pfn_hackrf_open)
	                       GETPROCADDRESS (Handle, "hackrf_open");
	if (this -> hackrf_open == NULL) {
	   fprintf (stderr, "Could not find hackrf_open\n");
	   return false;
	}

	this -> hackrf_close	= (pfn_hackrf_close)
	                       GETPROCADDRESS (Handle, "hackrf_close");
	if (this -> hackrf_close == NULL) {
	   fprintf (stderr, "Could not find hackrf_close\n");
	   return false;
	}

	this -> hackrf_exit	= (pfn_hackrf_exit)
	                       GETPROCADDRESS (Handle, "hackrf_exit");
	if (this -> hackrf_exit == NULL) {
	   fprintf (stderr, "Could not find hackrf_exit\n");
	   return false;
	}

	this -> hackrf_start_rx	= (pfn_hackrf_start_rx)
	                       GETPROCADDRESS (Handle, "hackrf_start_rx");
	if (this -> hackrf_start_rx == NULL) {
	   fprintf (stderr, "Could not find hackrf_start_rx\n");
	   return false;
	}

	this -> hackrf_stop_rx	= (pfn_hackrf_stop_rx)
	                       GETPROCADDRESS (Handle, "hackrf_stop_rx");
	if (this -> hackrf_stop_rx == NULL) {
	   fprintf (stderr, "Could not find hackrf_stop_rx\n");
	   return false;
	}

	this -> hackrf_device_list	= (pfn_hackrf_device_list)
	                       GETPROCADDRESS (Handle, "hackrf_device_list");
	if (this -> hackrf_device_list == NULL) {
	   fprintf (stderr, "Could not find hackrf_device_list\n");
	   return false;
	}

	this -> hackrf_set_baseband_filter_bandwidth	=
	                      (pfn_hackrf_set_baseband_filter_bandwidth)
	                      GETPROCADDRESS (Handle,
	                         "hackrf_set_baseband_filter_bandwidth");
	if (this -> hackrf_set_baseband_filter_bandwidth == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_baseband_filter_bandwidth\n");
	   return false;
	}

	this -> hackrf_set_lna_gain	= (pfn_hackrf_set_lna_gain)
	                       GETPROCADDRESS (Handle, "hackrf_set_lna_gain");
	if (this -> hackrf_set_lna_gain == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_lna_gain\n");
	   return false;
	}

	this -> hackrf_set_vga_gain	= (pfn_hackrf_set_vga_gain)
	                       GETPROCADDRESS (Handle, "hackrf_set_vga_gain");
	if (this -> hackrf_set_vga_gain == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_vga_gain\n");
	   return false;
	}

	this -> hackrf_set_freq	= (pfn_hackrf_set_freq)
	                       GETPROCADDRESS (Handle, "hackrf_set_freq");
	if (this -> hackrf_set_freq == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_freq\n");
	   return false;
	}

	this -> hackrf_set_sample_rate	= (pfn_hackrf_set_sample_rate)
	                       GETPROCADDRESS (Handle, "hackrf_set_sample_rate");
	if (this -> hackrf_set_sample_rate == NULL) {
	   fprintf (stderr, "Could not find hackrf_set_sample_rate\n");
	   return false;
	}

	this -> hackrf_is_streaming	= (pfn_hackrf_is_streaming)
	                       GETPROCADDRESS (Handle, "hackrf_is_streaming");
	if (this -> hackrf_is_streaming == NULL) {
	   fprintf (stderr, "Could not find hackrf_is_streaming\n");
	   return false;
	}

	this -> hackrf_error_name	= (pfn_hackrf_error_name)
	                       GETPROCADDRESS (Handle, "hackrf_error_name");
	if (this -> hackrf_error_name == NULL) {
	   fprintf (stderr, "Could not find hackrf_error_name\n");
	   return false;
	}

	this -> hackrf_usb_board_id_name = (pfn_hackrf_usb_board_id_name)
	                       GETPROCADDRESS (Handle, "hackrf_usb_board_id_name");
	if (this -> hackrf_usb_board_id_name == NULL) {
	   fprintf (stderr, "Could not find hackrf_usb_board_id_name\n");
	   return false;
	}

// Aggiunta Fabio
	this -> hackrf_set_antenna_enable = (pfn_hackrf_set_antenna_enable)
	                  GETPROCADDRESS (Handle, "hackrf_set_antenna_enable");
	if (this -> hackrf_set_antenna_enable == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_antenna_enable\n");
	   return false;
	}

	this -> hackrf_set_amp_enable = (pfn_hackrf_set_amp_enable)
	                  GETPROCADDRESS (Handle, "hackrf_set_amp_enable");
	if (this -> hackrf_set_amp_enable == nullptr) {
	   fprintf (stderr, "Could not find hackrf_set_amp_enable\n");
	   return false;
	}

	this -> hackrf_si5351c_read = (pfn_hackrf_si5351c_read)
	                 GETPROCADDRESS (Handle, "hackrf_si5351c_read");
	if (this -> hackrf_si5351c_read == nullptr) {
	   fprintf (stderr, "Could not find hackrf_si5351c_read\n");
	   return false;
	}

	this -> hackrf_si5351c_write = (pfn_hackrf_si5351c_write)
	                 GETPROCADDRESS (Handle, "hackrf_si5351c_write");
	if (this -> hackrf_si5351c_write == nullptr) {
	   fprintf (stderr, "Could not find hackrf_si5351c_write\n");
	   return false;
	}

	fprintf (stderr, "OK, functions seem to be loaded\n");
	return true;
}

void	hackrfHandler::report_dataAvailable (void) {
	emit dataAvailable (10);
}

