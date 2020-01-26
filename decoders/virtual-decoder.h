
#ifndef	__VIRTUAL_DECODER__
#define	__VIRTUAL_DECODER__

#include	<QObject>

class	virtualDecoder: public QObject {
Q_OBJECT
public:
			virtualDecoder	();
virtual			~virtualDecoder	();

signals:
	void	audioAvailable	(int, int);
};

#endif
