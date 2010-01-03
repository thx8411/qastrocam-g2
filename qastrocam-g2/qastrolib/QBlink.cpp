/******************************************************************
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

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


#include "QBlink.moc"

QBlink::QBlink(QWidget* parent) : QLabel(parent) {
   position=0;
   items[0]="-";
   items[1]="\\";
   items[2]="|";
   items[3]="/";
   setText(items[position]);
}

void QBlink::step(){
   position++;
   position%=4;
   setText(items[position]);
}
