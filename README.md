
------------------------------------------------------------------
DRMPLUS-0.4 EXPERIMENTAL
------------------------------------------------------------------

DRMPLUS-0.4 is experimental software for the decoding of
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
To be done:
--------------------------------------------------------------------------

 - The handling of non-audio data in a DRM+ stream still has to be addressed;

 - the time and frequency synchronization need more work;

 - alternative approaches to equalization (now I have taken the equalizer from
the swradio drm decoder.

 - ?????
--------------------------------------------------------------------------
Copyright
---------------------------------------------------------------------------
# Copyright

	Copyright (C)  2020
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

	The drmplus software is made available under the GPL-2.0.
	The drmplus software is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the
	implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License
	for more details.

# drmplus
