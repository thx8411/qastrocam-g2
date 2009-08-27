/*
   dcraw.c -- Dave Coffin's raw photo decoder
   Copyright 1997-2003 by Dave Coffin, dcoffin a cybercom o net

   This is a portable ANSI C program to convert raw image files from
   any digital camera into PPM format.  TIFF and CIFF parsing are
   based upon public specifications, but no such documentation is
   available for the raw sensor data, so writing this program has
   been an immense effort.

   This code is freely licensed for all uses, commercial and
   otherwise.  Comments, questions, and encouragement are welcome.

   $Revision: 1.1.1.1 $
   $Date: 2009-08-27 04:07:18 $

   The Canon EOS-1D and some Kodak cameras compress their raw data
   with lossless JPEG.  To read such images, you must also download:

	http://www.cybercom.net/~dcoffin/dcraw/ljpeg_decode.tar.gz
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define strcasecmp stricmp
typedef __int64 INT64;
#else
#define __USE_XOPEN
#include <unistd.h>
#include <netinet/in.h>
typedef long long INT64;
#endif

#ifdef LJPEG_DECODE
#include "jpeg.h"
#include "mcu.h"
#include "proto.h"
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;

/* Global Variables */

FILE *ifp;
short order;
char make[64], model[64], model2[64];
int raw_height=0, raw_width=0;	/* Including black borders */
int raw_depth=0;
int timestamp;
int tiff_data_offset, tiff_data_compression;
int kodak_data_compression;
int nef_curve_offset;
int height=0, width=0, colors, black, rgb_max;
int is_canon, is_cmy, is_foveon, use_coeff, trim, ymag;
unsigned filters=0;
ushort (*image)[4];
void (*load_raw)();
float gamma_val=0.8, bright=1.0, red_scale=1.0, blue_scale=1.0;
int four_color_rgb=0, use_camera_wb=0, document_mode=0, quick_interpolate=0;
float camera_red, camera_blue;
float pre_mul[4], coeff[3][4];
int histogram[0x2000];
void write_ppm(FILE *);
void (*write_fun)(FILE *) = write_ppm;

struct decode {
  struct decode *branch[2];
  int leaf;
} first_decode[32], second_decode[512];

/*
   In order to inline this calculation, I make the risky
   assumption that all filter patterns can be described
   by a repeating pattern of eight rows and two columns

   Return values are either 0/1/2/3 = G/M/C/Y or 0/1/2/3 = R/G1/B/G2
 */
#define FC(row,col) \
	(filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)
/*
   PowerShot 600 uses 0xe1e4e1e4:

	  0 1 2 3 4 5
	0 G M G M G M
	1 C Y C Y C Y
	2 M G M G M G
	3 C Y C Y C Y

   PowerShot A5 uses 0x1e4e1e4e:

	  0 1 2 3 4 5
	0 C Y C Y C Y
	1 G M G M G M
	2 C Y C Y C Y
	3 M G M G M G

   PowerShot A50 uses 0x1b4e4b1e:

	  0 1 2 3 4 5
	0 C Y C Y C Y
	1 M G M G M G
	2 Y C Y C Y C
	3 G M G M G M
	4 C Y C Y C Y
	5 G M G M G M
	6 Y C Y C Y C
	7 M G M G M G

   PowerShot Pro70 uses 0x1e4b4e1b:

	  0 1 2 3 4 5
	0 Y C Y C Y C
	1 M G M G M G
	2 C Y C Y C Y
	3 G M G M G M
	4 Y C Y C Y C
	5 G M G M G M
	6 C Y C Y C Y
	7 M G M G M G

   PowerShots Pro90 and G1 use 0xb4b4b4b4:

	  0 1 2 3 4 5
	0 G M G M G M
	1 Y C Y C Y C

   All RGB cameras use one of these Bayer grids:

	0x16161616:	0x61616161:	0x49494949:	0x94949494:

	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
	0 B G B G B G	0 G R G R G R	0 G B G B G B	0 R G R G R G
	1 G R G R G R	1 B G B G B G	1 R G R G R G	1 G B G B G B
	2 B G B G B G	2 G R G R G R	2 G B G B G B	2 R G R G R G
	3 G R G R G R	3 B G B G B G	3 R G R G R G	3 G B G B G B

 */

void merror (void *ptr, char *where)
{
  if (ptr) return;
  fprintf (stderr, "Out of memory in %s\n", where);
  exit(1);
}

/*
   load raw file with no hedears. size capteur type is set with parameters
   -W -H -D -F.
*/
void raw_load_raw() {
  ushort  pixels[2000];
  uchar * pixelc=&pixels;
  int col,row;
    fseek (ifp, 0, SEEK_SET);
    for (row=0; row < height; ++row) {
    fread(pixels, raw_depth/8,width, ifp);
    for(col=0;col<width; ++col) {
       image[row*width+col][FC(row,col)] = (raw_depth==8?pixelc[col]:pixels[col]);
    }
  }
}


/*
   Seach from the current directory up to the root looking for
   a ".badpixels" file, and fix those pixels now.
 */
void bad_pixels()
{
  FILE *fp=NULL;
  char *fname, *cp, line[128];
  int len, time, row, col, r, c, rad, tot, n, fixed=0;

  for (len=16 ; ; len *= 2) {
    fname = malloc (len);
    if (!fname) return;
    if (getcwd (fname, len-12)) break;
    free (fname);
    if (errno != ERANGE) return;
  }
  cp = fname + strlen(fname);
  if (cp[-1] == '/') cp--;
  while (*fname == '/') {
    strcpy (cp, "/.badpixels");
    if ((fp = fopen (fname, "r"))) break;
    if (cp == fname) break;
    while (*--cp != '/');
  }
  free (fname);
  if (!fp) return;
  while (1) {
    fgets (line, 128, fp);
    if (feof(fp)) break;
    cp = strchr (line, '#');
    if (cp) *cp = 0;
    if (sscanf (line, "%d %d %d", &col, &row, &time) != 3) continue;
    if ((unsigned) col >= width || (unsigned) row >= height) continue;
    if (time > timestamp) continue;
    for (tot=n=0, rad=1; rad < 3 && n==0; rad++)
      for (r = row-rad; r <= row+rad; r++)
	for (c = col-rad; c <= col+rad; c++)
	  if ((unsigned) r < height && (unsigned) c < width &&
		(r != row || c != col) && FC(r,c) == FC(row,col)) {
	    tot += image[r*width+c][FC(r,c)];
	    n++;
	  }
    image[row*width+col][FC(row,col)] = tot/n;
    if (!fixed++)
      fprintf (stderr, "Fixed bad pixels at:");
    fprintf (stderr, " %d,%d", col, row);
  }
  if (fixed) fputc ('\n', stderr);
  fclose (fp);
}

/*
   Automatic color balance, currently used only in Document Mode.
 */
void auto_scale()
{
  int row, col, c, val;
  int min[4], max[4], count[4];
  double sum[4], maxd=0;

  for (c=0; c < 4; c++) {
    min[c] = INT_MAX;
    sum[c] = max[c] = count[c] = 0;
  }
  for (row=0; row < height; row++)
    for (col=0; col < width; col++)
      for (c=0; c < colors; c++) {
	val = image[row*width+col][c];
	if (!val) continue;
	val -= black;
	if (val < 0) val = 0;
	if (min[c] > val) min[c] = val;
	if (max[c] < val) max[c] = val;
	sum[c] += val;
	count[c]++;
      }
  for (c=0; c < colors; c++) {		/* Smallest pre_mul[] value */
    pre_mul[c] = sum[c]/count[c];	/* should be 1.0 */
    if (maxd < pre_mul[c])
        maxd = pre_mul[c];
  }
  for (c=0; c < colors; c++)
    pre_mul[c] = maxd / pre_mul[c];
}

void scale_colors()
{
  int row, col, c, val;

  rgb_max -= black;
  for (row=0; row < height; row++)
    for (col=0; col < width; col++)
      for (c=0; c < colors; c++) {
	val = image[row*width+col][c];
	if (!val) continue;
	val -= black;
	val *= pre_mul[c];
	if (val < 0) val = 0;
	if (val > rgb_max) val = rgb_max;
	image[row*width+col][c] = val;
      }
}

/*
   This algorithm is officially called:

   "Interpolation using a Threshold-based variable number of gradients"

   described in http://www-ise.stanford.edu/~tingchen/algodep/vargra.html

   I've extended the basic idea to work with non-Bayer filter arrays.
   Gradients are numbered clockwise from NW=0 to W=7.
 */
void vng_interpolate()
{
  static const signed char *cp, terms[] = {
    -2,-2,+0,-1,0,0x01, -2,-2,+0,+0,1,0x01, -2,-1,-1,+0,0,0x01,
    -2,-1,+0,-1,0,0x02, -2,-1,+0,+0,0,0x03, -2,-1,+0,+1,0,0x01,
    -2,+0,+0,-1,0,0x06, -2,+0,+0,+0,1,0x02, -2,+0,+0,+1,0,0x03,
    -2,+1,-1,+0,0,0x04, -2,+1,+0,-1,0,0x04, -2,+1,+0,+0,0,0x06,
    -2,+1,+0,+1,0,0x02, -2,+2,+0,+0,1,0x04, -2,+2,+0,+1,0,0x04,
    -1,-2,-1,+0,0,0x80, -1,-2,+0,-1,0,0x01, -1,-2,+1,-1,0,0x01,
    -1,-2,+1,+0,0,0x01, -1,-1,-1,+1,0,0x88, -1,-1,+1,-2,0,0x40,
    -1,-1,+1,-1,0,0x22, -1,-1,+1,+0,0,0x33, -1,-1,+1,+1,1,0x11,
    -1,+0,-1,+2,0,0x08, -1,+0,+0,-1,0,0x44, -1,+0,+0,+1,0,0x11,
    -1,+0,+1,-2,0,0x40, -1,+0,+1,-1,0,0x66, -1,+0,+1,+0,1,0x22,
    -1,+0,+1,+1,0,0x33, -1,+0,+1,+2,0,0x10, -1,+1,+1,-1,1,0x44,
    -1,+1,+1,+0,0,0x66, -1,+1,+1,+1,0,0x22, -1,+1,+1,+2,0,0x10,
    -1,+2,+0,+1,0,0x04, -1,+2,+1,+0,0,0x04, -1,+2,+1,+1,0,0x04,
    +0,-2,+0,+0,1,0x80, +0,-1,+0,+1,1,0x88, +0,-1,+1,-2,0,0x40,
    +0,-1,+1,+0,0,0x11, +0,-1,+2,-2,0,0x40, +0,-1,+2,-1,0,0x20,
    +0,-1,+2,+0,0,0x30, +0,-1,+2,+1,0,0x10, +0,+0,+0,+2,1,0x08,
    +0,+0,+2,-2,1,0x40, +0,+0,+2,-1,0,0x60, +0,+0,+2,+0,1,0x20,
    +0,+0,+2,+1,0,0x30, +0,+0,+2,+2,1,0x10, +0,+1,+1,+0,0,0x44,
    +0,+1,+1,+2,0,0x10, +0,+1,+2,-1,0,0x40, +0,+1,+2,+0,0,0x60,
    +0,+1,+2,+1,0,0x20, +0,+1,+2,+2,0,0x10, +1,-2,+1,+0,0,0x80,
    +1,-1,+1,+1,0,0x88, +1,+0,+1,+2,0,0x08, +1,+0,+2,-1,0,0x40,
    +1,+0,+2,+1,0,0x10
  }, chood[] = { -1,-1, -1,0, -1,+1, 0,+1, +1,+1, +1,0, +1,-1, 0,-1 };
  ushort (*brow[5])[4], *pix;
  int code[8][640], *ip, gval[8], gmin, gmax, sum[4];
  int row, col, shift, x, y, x1, x2, y1, y2, t, weight, grads, color, diag;
  int g, diff, thold, num, c;

  for (row=0; row < 8; row++) {		/* Precalculate for bilinear */
    ip = code[row];
    for (col=1; col < 3; col++) {
      memset (sum, 0, sizeof sum);
      for (y=-1; y <= 1; y++)
	for (x=-1; x <= 1; x++) {
	  shift = (y==0) + (x==0);
	  if (shift == 2) continue;
	  color = FC(row+y,col+x);
	  *ip++ = (width*y + x)*4 + color;
	  *ip++ = shift;
	  *ip++ = color;
	  sum[color] += 1 << shift;
	}
      for (c=0; c < colors; c++)
	if (c != FC(row,col)) {
	  *ip++ = c;
	  *ip++ = sum[c];
	}
    }
  }
  for (row=1; row < height-1; row++) {	/* Do bilinear interpolation */
    pix = image[row*width+1];
    for (col=1; col < width-1; col++) {
      if (col & 1)
	ip = code[row & 7];
      memset (sum, 0, sizeof sum);
      for (g=8; g--; ) {
	diff = pix[*ip++];
	diff <<= *ip++;
	sum[*ip++] += diff;
      }
      for (g=colors; --g; ) {
	c = *ip++;
	pix[c] = sum[c] / *ip++;
      }
      pix += 4;
    }
  }
  if (quick_interpolate)
    return;
  for (row=0; row < 8; row++) {		/* Precalculate for VNG */
    ip = code[row];
    for (col=0; col < 2; col++) {
      for (cp=terms, t=0; t < 64; t++) {
	y1 = *cp++;  x1 = *cp++;
	y2 = *cp++;  x2 = *cp++;
	weight = *cp++;
	grads = *cp++;
	color = FC(row+y1,col+x1);
	if (FC(row+y2,col+x2) != color) continue;
	diag = (FC(row,col+1) == color && FC(row+1,col) == color) ? 2:1;
	if (abs(y1-y2) == diag && abs(x1-x2) == diag) continue;
	*ip++ = (y1*width + x1)*4 + color;
	*ip++ = (y2*width + x2)*4 + color;
	*ip++ = weight;
	for (g=0; g < 8; g++)
	  if (grads & 1<<g) *ip++ = g;
	*ip++ = -1;
      }
      *ip++ = INT_MAX;
      for (cp=chood, g=0; g < 8; g++) {
	y = *cp++;  x = *cp++;
	*ip++ = (y*width + x) * 4;
	color = FC(row,col);
	if ((g & 1) == 0 &&
	    FC(row+y,col+x) != color && FC(row+y*2,col+x*2) == color)
	  *ip++ = (y*width + x) * 8 + color;
	else
	  *ip++ = 0;
      }
    }
  }
  brow[4] = calloc (width*3, sizeof **brow);
  merror (brow[4], "vng_interpolate()");
  for (row=0; row < 3; row++)
    brow[row] = brow[4] + row*width;
  for (row=2; row < height-2; row++) {		/* Do VNG interpolation */
    pix = image[row*width+2];
    for (col=2; col < width-2; col++) {
      if ((col & 1) == 0)
	ip = code[row & 7];
      memset (gval, 0, sizeof gval);
      while ((g = *ip++) != INT_MAX) {		/* Calculate gradients */
	diff = abs(pix[g] - pix[*ip++]);
	diff <<= *ip++;
	while ((g = *ip++) != -1)
	  gval[g] += diff;
      }
      gmin = INT_MAX;				/* Choose a threshold */
      gmax = 0;
      for (g=0; g < 8; g++) {
	if (gmin > gval[g]) gmin = gval[g];
	if (gmax < gval[g]) gmax = gval[g];
      }
      thold = gmin + (gmax >> 1);
      memset (sum, 0, sizeof sum);
      color = FC(row,col);
      for (num=g=0; g < 8; g++,ip+=2) {		/* Average the neighbors */
	if (gval[g] <= thold) {
	  for (c=0; c < colors; c++)
	    if (c == color && ip[1])
	      sum[c] += (pix[c] + pix[ip[1]]) >> 1;
	    else
	      sum[c] += pix[ip[0] + c];
	  num++;
	}
      }
      for (c=0; c < colors; c++) {		/* Save to buffer */
	t = pix[color] + (sum[c] - sum[color])/num;
	brow[2][col][c] = t > 0 ? (t < 0xffff ? t : 0xffff) : 0;
      }
      pix += 4;
    }
    if (row > 3)				/* Write buffer to image */
      memcpy (image[(row-2)*width+2], brow[0]+2, (width-4)*sizeof *image);
    for (g=0; g < 4; g++)
      brow[(g-1) & 3] = brow[g];
  }
  memcpy (image[(row-2)*width+2], brow[0]+2, (width-4)*sizeof *image);
  memcpy (image[(row-1)*width+2], brow[1]+2, (width-4)*sizeof *image);
  free(brow[4]);
}


/*
   Convert the entire image to RGB colorspace and build a histogram.
 */
void convert_to_rgb() {
  int row, col, r, g, c=0;
  ushort *img;
  float rgb[4];

  if (document_mode)
    colors = 1;
  memset (histogram, 0, sizeof histogram);
  for (row = trim; row < height-trim; row++)
    for (col = trim; col < width-trim; col++) {
      img = image[row*width+col];
      if (document_mode)
	c = FC(row,col);
      if (colors == 4 && !use_coeff)	/* Recombine the greens */
	img[1] = (img[1] + img[3]) >> 1;
      if (colors == 1)			/* RGB from grayscale */
	for (r=0; r < 3; r++)
	  rgb[r] = img[c];
      else if (use_coeff) {		/* RGB from GMCY or Foveon */
	for (r=0; r < 3; r++)
	  for (rgb[r]=g=0; g < colors; g++)
	    rgb[r] += img[g] * coeff[r][g];
      } else if (is_cmy) {		/* RGB from CMY */
	rgb[0] = img[0] + img[1] - img[2];
	rgb[1] = img[1] + img[2] - img[0];
	rgb[2] = img[2] + img[0] - img[1];
      } else				/* RGB from RGB (easy) */
	for (r=0; r < 3; r++)
	  rgb[r] = img[r];
      for (rgb[3]=r=0; r < 3; r++) {	/* Compute the magnitude */
	if (rgb[r] < 0) rgb[r] = 0;
	if (rgb[r] > rgb_max) rgb[r] = rgb_max;
	rgb[3] += rgb[r]*rgb[r];
      }
      rgb[3] = sqrt(rgb[3])/2;
      if (rgb[3] > 0xffff) rgb[3] = 0xffff;
      for (r=0; r < 4; r++)
	img[r] = rgb[r];
      histogram[img[3] >> 3]++;		/* bin width is 8 */
    }
}

/*
   Write the image to a 24-bit PPM file.
 */
void write_ppm(FILE *ofp)
{
  int row, col, i, c, val, total;
  float max, mul, scale;
  ushort *rgb;
  uchar (*ppm)[3];

/*
   Set the white point to the 99th percentile
 */
  for (val=0x2000, total=0; --val; )
    if ((total+=histogram[val]) > (int)(width*height*0.01)) break;
  max = val << 4;

  fprintf (ofp, "P6\n%d %d\n255\n",
	width-trim*2, ymag*(height-trim*2));

  ppm = calloc (width-trim*2, 3);
  merror (ppm, "write_ppm()");
  mul = bright * 442 / max;

  for (row=trim; row < height-trim; row++) {
    for (col=trim; col < width-trim; col++) {
      rgb = image[row*width+col];
/* In some math libraries, pow(0,expo) doesn't return zero */
      scale = rgb[3] ? mul * pow (rgb[3]*2/max, gamma_val-1) : 0;
      for (c=0; c < 3; c++) {
	val = rgb[c] * scale;
	if (val > 255) val=255;
	ppm[col-trim][c] = val;
      }
    }
    for (i=0; i < ymag; i++)
      fwrite (ppm, width-trim*2, 3, ofp);
  }
  free(ppm);
}

int main(int argc, char **argv)
{
  int arg, id, identify_only=0, write_to_stdout=0;
  char opt, *ofname, *cp;
  const char *write_ext = ".ppm";
  FILE *ofp;

  image = calloc (height * width, sizeof *image);
  //(*load_raw)();
  //bad_pixels();
  scale_colors();
  vng_interpolate();
  convert_to_rgb();
  return 0;
}
