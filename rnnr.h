#ifndef _rnnr_h
#define _rnnr_h

#include "rnnoise.h"

#define FRAME_SIZE

typedef struct _rnnr
{
	int run;
    	int position;
        int frame_size;
        DenoiseState *st;
        double *in;
        double *out;
//        short temp[FRAME_SIZE];
}rnnr, *RNNR;

extern RNNR create_rnnr (int run, int position, double *in, double *out);
extern void setBuffers_rnnr (RNNR a, double* in, double* out);
extern void destroy_rnnr (RNNR a);
extern void xrnnr (RNNR a, int pos);

#endif //_rnnr_h
