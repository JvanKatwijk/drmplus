
#include	"equalizer-2.h"
#include	"drm-decoder.h"
#include	"referenceframe.h"

	equalizer_2::equalizer_2 (drmDecoder *drm,
	                          RingBuffer<std::complex<float>> *eq) {
	this	-> parent	= drm;
	this	-> eqBuffer	= eq;
	connect (this, SIGNAL (show_eqsymbol (int)),
                 parent, SLOT (show_eqsymbol (int)));
}

	equalizer_2::~equalizer_2 () {}

bool	equalizer_2::equalize	(std::complex<float> *inVec,
	                         int16_t	symbolNumber,
	                         theSignal **	outVec) {
	for (int gainCell = K_min; gainCell < K_max; gainCell ++) {
	   int nextGainCell;
	   if (!isGainCell (symbolNumber, gainCell))
	      continue;
	   if (gainCell == K_max - 3) {
	      break;
	   }
	   nextGainCell = -1;
	   for (int i = 1; i <= 16; i ++) {
	      if (gainCell + i > K_max)
	         break;
	      if (isGainCell (symbolNumber, gainCell + i)) {
	         nextGainCell = gainCell + i;
	         break;
	      }
	   }
	   if (nextGainCell != -1) {
	      process (gainCell, nextGainCell, symbolNumber, inVec, outVec);
	   }
	}
	if (symbolNumber == 39) {
	   show_eqsymbol (K_max - K_min + 1);
	}
	return symbolNumber == 39;
}

void	equalizer_2::process (int startCarrier, int endCarrier, 
	                      int symbolNumber,
	                      std::complex<float> *inVec,
	                      theSignal ** outVec) {
std::complex<float> first  = inVec [startCarrier - K_min] /
	                           getGainValue (symbolNumber, startCarrier);
std::complex<float> second = inVec [endCarrier - K_min] /
	                           getGainValue (symbolNumber, endCarrier);

int	segmentLength = endCarrier - startCarrier + 1;

	for (int i = 0; i < segmentLength; i ++) {
	   std::complex<float> een  = cmul (first, ((float)(segmentLength) - i) / segmentLength);
	   std::complex<float> twee = cmul (second, (float) i/segmentLength);
	   std::complex<float> corrector = een + twee;
	   outVec [symbolNumber][startCarrier + i - K_min]. signalValue = 
	         inVec [startCarrier + i - K_min] / (een + twee);
	   outVec [symbolNumber] [startCarrier + i - K_min]. rTrans =
	                                   abs (een + twee);
//	   if (symbolNumber == 5)
//	   if (isGainCell (symbolNumber, startCarrier + i))
//	      fprintf (stderr, "%f ", abs (outVec [symbolNumber][startCarrier + i - K_min]. signalValue));
	   if (symbolNumber == 39) {
	      eqBuffer -> putDataIntoBuffer (&corrector, 1);
	   }
	}
}


