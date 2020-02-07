
#ifndef	__POST_PROCESSOR__
#define	__POST_PROCESSOR__

#include	<QObject>
#include	<stdint.h>

class	postProcessor: public QObject {
public:
		postProcessor	();
virtual		~postProcessor	();
virtual	void	process		(uint8_t *, uint8_t *, int);
};

#endif

