#include "include.h"

//50Hz陷波、IIRnotching、带宽10Hz、采样率250Hz、Apass=1
const float NUM_NOT50[3] = {
     0.9396002918425,	 -0.580704916198,	0.9396002918425
};
const float DEN_NOT50[3] = {
     1,	 -0.580704916198,	 0.879200583685
};

//1~50Hz带通、IIR切比雪夫1型、2阶、采样率250Hz、Apass=10
const int NL[MWSPT_NSEC] = { 1,3,1 };
const float NUM[MWSPT_NSEC][3] = {
  {
     0.1908327908583,                 0,                 0 
  },
  {
                   1,                 0,                -1 
  },
  {
                   1,                 0,                 0 
  }
};
const int DL[MWSPT_NSEC] = { 1,3,1 };
const float DEN[MWSPT_NSEC][3] = {
  {
                   1,                 0,                 0 
  },
  {
                   1,   -1.589049453443,   0.6183344182834 
  },
  {
                   1,                 0,                 0 
  }
};


IIRNOTMID IIRNMID[8];
IIRTEMP IIRWDATA[8];

float IIRFilter(float ADdata,uint8_t chnum)
{
	uint8_t k;
	float ADtemp=ADdata;
	float DenMulW1,DenMulW2;
	float NumMulW0,NumMulW1,NumMulW2;
	
	for(k=0;k<MWSPT_NSEC;k++)
	{ 
		if(DL[k]==3)
		{  
			if(DEN[k][1]==1.0)
				DenMulW1 = IIRWDATA[chnum].w[k][1];
			else
				DenMulW1 = DEN[k][1]*IIRWDATA[chnum].w[k][1];
				
			if(DEN[k][2]==1.0)
				DenMulW2 = IIRWDATA[chnum].w[k][2];
			else
				DenMulW2 = DEN[k][2]*IIRWDATA[chnum].w[k][2];

			IIRWDATA[chnum].w[k][0]=ADtemp-DenMulW1-DenMulW2;
		}
		else
		{ 
			IIRWDATA[chnum].w[k][0]=ADtemp;
		}

		if(NL[k]==3)
		{
			if(NUM[k][0]==1.0)
				NumMulW0 = IIRWDATA[chnum].w[k][0];
			else
				NumMulW0 = NUM[k][0]*IIRWDATA[chnum].w[k][0];

			if(NUM[k][1]==1.0)
				NumMulW1 = IIRWDATA[chnum].w[k][1];
			else
				NumMulW1 = NUM[k][1]*IIRWDATA[chnum].w[k][1];

			if(NUM[k][2]==1.0)
				NumMulW2 = IIRWDATA[chnum].w[k][2];
			else
				NumMulW2 = NUM[k][2]*IIRWDATA[chnum].w[k][2];

			ADtemp=NumMulW0+ NumMulW1+NumMulW2;

		}
		else
		{
			if(NUM[k][0]==1.0)
				ADtemp=IIRWDATA[chnum].w[k][0];
			else 
				ADtemp=NUM[k][0]*IIRWDATA[chnum].w[k][0];
		}

		IIRWDATA[chnum].w[k][2]=IIRWDATA[chnum].w[k][1];
		IIRWDATA[chnum].w[k][1]=IIRWDATA[chnum].w[k][0];
	}
	 
	return  ADtemp;
 }	

 float IIRNotching(float rawdata,uint8_t chnum)
 {
 	float ADdatabuf=rawdata;

	IIRNMID[chnum].NotMid[0]=ADdatabuf-DEN_NOT50[1]*IIRNMID[chnum].NotMid[1]-DEN_NOT50[2]*IIRNMID[chnum].NotMid[2];
 	ADdatabuf=NUM_NOT50[0]*IIRNMID[chnum].NotMid[0]+ NUM_NOT50[1]*IIRNMID[chnum].NotMid[1]+NUM_NOT50[2]*IIRNMID[chnum].NotMid[2];

 	IIRNMID[chnum].NotMid[2]=IIRNMID[chnum].NotMid[1];
	IIRNMID[chnum].NotMid[1]=IIRNMID[chnum].NotMid[0];

	return  ADdatabuf;
 }


