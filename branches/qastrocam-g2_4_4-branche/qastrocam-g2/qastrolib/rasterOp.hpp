#ifndef _rasterOp_hpp_
#define _rasterOp_hpp_

#define SIGN(x)       (((x) < 0) ? -1 : 1)
#define ABS(x)        (((x) < 0) ? (-1 * (x)) : (x))
#define MIN(x,y)      (((x) < (y)) ? (x) : (y))

void rasteropVipLow(unsigned int  *data,
                    int    pixw,
                    int    pixh,
                    int    depth,
                    int    wpl,
                    int    x,
                    int    w,
                    int    shift);
void rasteropHipLow(unsigned int  *data,
                    int    pixh,
                    int    depth,
                    int    wpl,
                    int    y,
                    int    h,
                    int    shift);
void
shiftDataHorizontalLow(unsigned int  *datad,
                       int    wpld,
		       unsigned int  *datas,
		       int    wpls,
		       int    shift);
#endif
