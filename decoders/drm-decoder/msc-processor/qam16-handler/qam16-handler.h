
#ifndef	__QAM16_HANDLER__
#define	__QAM16_HANDLER__

#include	"basics.h"
#include	"msc-handler.h"
#include	"qam16-metrics.h"

class	postProcessor;
class	Mapper;
class	MSC_streamer;
class	prbs;

class qam16_handler: public mscHandler {
public:
		qam16_handler	(drmParameters *, postProcessor *, int, int);
		~qam16_handler	();
	void	process		(theSignal *, bool);
private:
	drmParameters	*params;
	postProcessor	*the_postProcessor;
	prbs		*thePRBS;
	int		muxLength;
	int		streamIndex;
	qam16_metrics   myDecoder;
	int16_t         lengthA;
	int16_t         lengthB;
	uint8_t         *out;
	MSC_streamer    *stream_0;
	MSC_streamer    *stream_1;
	Mapper          *Y13mapper_high;
	Mapper          *Y21mapper_high;
	Mapper          *Y13mapper_low;
	Mapper          *Y21mapper_low;
	int16_t         N1, N2;
	uint8_t		*firstBuffer;
};
#endif

