#ifndef CHART_H_
#define CHART_H_

#include "observer.h"
#include "stdint.h"

typedef struct{
	uint8_t kk;
}chart, *chart_t;

struct Chart
{
	observer parant;
	notify_fun_t update;
};

void Chart_Init(struct Chart* chart);

#endif /* CHART_H_ */

