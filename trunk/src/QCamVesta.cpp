/******************************************************************
Qastrocam-g2
Copyright (C) 2013   Blaise-Florentin Collin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License v2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
*******************************************************************/

/******************************************************/
/* Qt moc can't doesn't pre-process #if #else #endif, */
/* so, conditionnal compilation can't be achieved. We */
/* need conditionnal compilation for kernel2/kernel3  */
/* versions of the Philips camera object. So, we let  */
/* moc generates '.moc' files for each version, and   */
/* the proper version is included in this dummy file  */
/* Object code is also included in this dummy file    */
/* Don't compile QCameVestaK2.cpp nor QCamVestaK3.cpp */
/* directly !                                         */
/******************************************************/


#if KERNEL_2
#include "QCamVestaK2.cpp"
#else
#include "QCamVestaK3.cpp"
#endif /* KERNEL_2 */
