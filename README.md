
------------------------------------------------------------------
DRMPLUS-0.4
------------------------------------------------------------------

DRMPLUS-0.5 is experimental software for the decoding of
DRM+ signals.

![drm-plus](/drmplus-drm.png?raw=true)

Since these signals are meant to be transmitted in the FM band, the
DRMPLUS decoder is therefore equipped with DRM+ and FM decoding

![drm-plus](/drmplus-fm.png?raw=true)

In the development of the software I have used (modified) parts
of the drm decoder of my swradio-8 software.

Since in the region I live there are no DRM + transmissions,
the software was tested with some input files, comments and 
suggestions are welcome.

-------------------------------------------------------------------------
Introduction
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
a 4QAM or a 16QAM signal.

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

