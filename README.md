
------------------------------------------------------------------
DRMPLUS-0.8
------------------------------------------------------------------

DRMPLUS-0.8 is experimental software for the decoding of
DRM+ signals. The software is "still in development" and certainly not
error free.

![drm-plus](/drmplus-drm.png?raw=true)

Since these signals are meant to be transmitted in the FM band, the
DRMPLUS decoder is therefore equipped with DRM+ and FM decoding

![drm-plus](/drmplus-fm.png?raw=true)

In the development of the software I have used (modified) parts
of the drm decoder of my swradio-8 software.

Since in the region I live there are no DRM + transmissions,
the software was tested with some input files, comments and 
suggestions are welcome.

----------------------------------------------------------------------------
xHE-AAC decoding, services and streams
----------------------------------------------------------------------------

The current (0.8) version of drmplus is - in principle - able to decode 
xHE-AAC encoded audio frames. *However, the ONLY source I currently
have is a recording of a fragment of a transmission from Petersburg,
in which the binding between service and (audio)stream changes
constantly*.
The selection of audio channels seems to work well, selection
of data is being worked on

Any suggestion is more than welcome

-----------------------------------------------------------------------
CONFIGURING
-----------------------------------------------------------------------

In version 0.8 one may configure the "faad" library for decoding AAC encoded
fragments, or "fdk-aac" for decoding AAC or xHE-AAC encoded fragments.

The ".pro" file provides an option to configure the software to use
the "faad" library or the "fdk-aac" library.

The ".pro" file contains in the "unix" section lines

	#CONFIG         += faad
	CONFIG          += fdk-aac

with which one (de)selects the library to be used.
(Note that uncommenting both gives undefined results.
Note further that if "faad" is selected, output of the xHE-AAC processor,
that maps the audio super frames onto audio frames, is not passed on
further.

The same applies to configuring the input device(s),
currently there is support for

 - pcm files, with 2 channels and a sampling rate od 192000,
 - raw files (8 bits IQ), raw files (16 bits IQ) and raw files (32 bits IQ),
all recorded with a sampling rate of 192000,
 - the SDRplay devices that are supported by the 2.13 library
 - RT2932 based "dab" sticks,
 - Hackrf devices
 - limeSDR

-------------------------------------------------------------------------
What is DRM
--------------------------------------------------------------------------

DRM (Digital Radio Mondiale) is a form of digital radio,
it is known for a number of transmitters
in the shortwave bands. Digital radio uses different techniques
than AM and FM, the classic analog radio variants.

One variant of DRM, DRM+, differs from DRM in the bandwidth used.
While DRM (Modes A .. D) mainly use a 10 Khz band (with variations
from 4.5 to app 20 Khz), DRM + uses a 100 Khz bandwidth is is
meant for e.g. the FM Band.

DRM - and DRM+ - can carry up to 4 services within this bandwidth,
both audio - usually encoded as AAC or xHE-AAC - and data - pictures,
slide shows and other data.

In the swradio-8 I already implemented a - to 10 Khz limited - DRM decoder,
and some of that software was "recycled" to help in constructing a DRM+
decoder.

Since the DRM+ signals are in the FM Band, the DRM+ decoder is also
equipped with an FM decoder: the main widget shows a button with which
one selects the deceder.

------------------------------------------------------------------------
Work to be done / being done
------------------------------------------------------------------------

DRM+ is transmitted using OFDM. OFDM is an interesting technique, 
typical problems in decoding occur through 

 - (a) clock errors, that lead to having too many samples (or too few)
for the OFDM symbol at hand, leading to a shift in the "first" sample
of a symbol. With one of the example files, we encountered a shift
of app 1 sample per 2 or 3 frames (a frame consists of 40 symbols). While
an offset of a few samples is not killing, this example showed
that after a few hundred frames, we had skipped 30 to 40 samples
in the input. The software in the 0.6 version now 
reports on the (average) number
of symbols (each symbol 480 samples on an inputrate of 192000)
that pass per sample over- or underflow: the larger the number the 
better.
 - (b) frequency drift. Frequency errors are typically handled
in two steps, the "coarse" error, with a step size of the carrier
distance in the decoded ofdm symbols, and a "fine" error, a resulting error
in the range of this carrier distance.
Drift in the frequency may lead to the "fine" error to grow such a way
that it has effect on the coarse error.

While in 0.6 an attempt is made to handle these error, error recovery
is far from complete as yet.

------------------------------------------------------------------------
Operation of the decoder
--------------------------------------------------------------------------

Operation of the decoder is straight forward, on program start-up 
the main widget appears. One selects an input device and a decoder
and a widget for controlling the decoder appears.

Assuming the DRM+ Decoder is selected:
When a valid DRM signal is detected, the name(s) of the service(s) will
be shown on the buttons in the middle of the widget.
Some data will be shown as well, technical data on the left.

With the buttons on the bottom of the DRM+ Decoder widget one may
make some really technical data visible, and some data on the
selected service.

Data on the selected service will give information on the protection
of the signal (just a technical term, skip if you are not familiar with it),
i.e, the protection of the high and low protected parts as well as the
offsets in the mux where to find the data.
Furthermore, this widget will tell whether the signal was encoded as
a 4QAM or a 16QAM signal (both supported).

The (really) technical widget will show two "scopes". The scope on the
left will show the equalization signal (note that DRM decoding requires
that the incoming signal is to be restored, note that this makes
DRM(+) decoding much more interesting than e.g. DAB decoding),
used to restore the signal. The scope on the right shows the constellation
of the received signal, one may choose here to see the FAC signals,
the SDC signal or the MSC signal.

The picture shows a 16QAM signal, i.e. one sees the 16 dots on the screen.

-----------------------------------------------------------------------
Input devices
-----------------------------------------------------------------------

Supported devices are 

 - the SDRplay,
 - RT2832 based so-called DAB sticks,
 - the HackRF device,
 - the limeSDR

Note that these devices will run with a samplerate of 11 * 192000
and some software filtering is applied - in the decimation process -
to narrow the reception band to 192000 (Note that the SDRplay already
has support for a bandwidth of 200 Khz)

The *dump* button will write out the input data as a PCM (.wav)
file with a samplerate of 192000
Next to these devices, there is support for

 - PCM based files, i.e. ".wav" files;
 - "raw" i.e. "iq" files, files where the raw samples are 8 bit unsigned ints;
 - "raw 16", i.e. files where the raw samples are 16 bit signed int values;
 - "raw 32", i.e. files where the raw samples are 32 bit signed int values.

------------------------------------------------------------------------
Building the software
------------------------------------------------------------------------

The standard way to build an executable is to use a qmake/make
line of work.

The ".pro" file indicates the libraries that are needed

-------------------------------------------------------------------------
Limitations
-------------------------------------------------------------------------

The software is "in development", its development is slightly hindered
by not having live DRM+ transmissions in the neighbourhood. All testing
is - up to now - done with some recordings of the signal.

It is most unlikely that here - in the Netherlands - a live DRM+
transmission can be heard, however, in some other countries, e.g.
Russia. 

Further input is needed to get the job done, any assistance is
really appreciated here

--------------------------------------------------------------------------
Copyright
---------------------------------------------------------------------------

	Copyright (C)  2020
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

	The drmplus software is made available under the GPL-2.0.
	The drmplus software is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the
	implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License
	for more details.

