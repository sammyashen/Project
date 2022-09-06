#ifndef _FILTER_H
#define _FILTER_H

#define MWSPT_NSEC 3

typedef struct
{
	volatile float NotMid[3]; 
}IIRNOTMID;

typedef struct
{
	volatile float w[MWSPT_NSEC][3];
}IIRTEMP;


float IIRFilter(float ADdata,uint8_t chnum);
float IIRNotching(float rawdata,uint8_t chnum);


#endif


