#ifndef SV_CORRELATION_H
#define SV_CORRELATION_H

#include <vector>
#include <list>
using namespace std;



class SVCorrelation
{
public:
	SVCorrelation(int timestamp);
	~SVCorrelation();
public:

private:
	int m_axis;
	int m_timestamp;
};

#endif //SV_CORRELATION