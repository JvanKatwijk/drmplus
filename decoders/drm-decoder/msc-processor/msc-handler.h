
#ifndef	__MSC_HANDLER__
#define	__MSC_HANDLER__

#include	"basics.h"
	
class mscHandler {
public:
		mscHandler	(drmParameters *, int muxLength, int stream);
virtual		~mscHandler	();
virtual	void	process		(theSignal *b, bool);
};

#endif

