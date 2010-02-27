/*
 *
 *  Copyright (C) 2010 Blaise-Florentin Collin  <thx8411@users.sourceforge.net>
 *
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/

/* private controls for AstroEasyCap */
/* vers. 0.1beta20100228 */

#ifndef _DC60_PRIVATE_IOCTLS_H_
#define _DC60_PRIVATE_IOCTLS_H_

/* True or False : change level on the 27th saa7113 pin */
#define	V4L2_CID_GPSW1		V4L2_CID_PRIVATE_BASE

/* True or False : is the camera integrating ? */
#define V4L2_CID_INTEGRATES	V4L2_CID_PRIVATE_BASE+1

/* True or False : enable or disable pre-amp */
#define V4L2_CID_PREAMP		V4L2_CID_PRIVATE_BASE+2

/* True or False : enable or disable anti-alias (pre-amp must be on for anti-alias on) */
#define V4L2_CID_ANTIALIAS	V4L2_CID_PRIVATE_BASE+3

// to do...

/* True or False : enable or disable chrominance prefilter */
#define V4L2_CID_CPREFILTER	V4L2_CID_PRIVATE_BASE+4

/* True of False : enable or disable chrominance filter */
#define V4L2_CID_CFILTER	V4L2_CID_PRIVATE_BASE+5

/* 0 (bypass) -> 3 (full) : set vertical noise reduction */
#define V4L2_CID_VNOISE         V4L2_CID_PRIVATE_BASE+6

#endif
