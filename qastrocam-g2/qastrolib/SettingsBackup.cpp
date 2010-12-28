/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2010   Blaise-Florentin Collin

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

#include <stdio.h>
#include <iostream>
#include "SettingsBackup.hpp"

using namespace std;

// find the key position in the table
// returns -1 if not found
int settingsBackup::findKey(const char* key) {
   int i=0;
   while((i<NB_RECORDS)&&(datas[0][i]!=key)) i++;
   if(i==NB_RECORDS) i=-1;
   return(i);
}

// add a key and a value in the table
void settingsBackup::addKey(const char* key, const char* val) {
   if(end_record==NB_RECORDS) cout << "Settings tab full" << endl;
   else {
      datas[0][end_record]=key;
      datas[1][end_record]=val;
      end_record++;
   }
}

// write the table to the file
void settingsBackup::serialize() {
   FILE* fd;
   int i;
   fd=fopen(fileName.c_str(),"w");
   if(fd!=NULL) {
      for(i=0;i<end_record;i++)
         fprintf(fd,"%s\t%s\n",datas[0][i].c_str(),datas[1][i].c_str());
      fclose(fd);
   }
}

settingsBackup::settingsBackup() {
   end_record=0;
   for(int i=0;i<NB_RECORDS;i++) datas[0][i]="";
}

void settingsBackup::setName(string name) {
   fileName=name;
}

void settingsBackup::setKey(const char* key, const char* val) {
   int i;
   i=findKey(key);
   if(i<0) addKey(key,val);
   else {
      datas[1][i]=val;
   }
   serialize();
}

bool settingsBackup::haveKey(const char* key) {
   return(getKey(key)!=NULL);
}

const char* settingsBackup::getKey(const char* key) {
   int i;
   i=findKey(key);
   if(i<0) return(NULL);
   return(datas[1][i].c_str());
}

bool settingsBackup::deSerialize() {
   FILE* fd;
   int i;
   int j;
   char* tmp;
   char buffer[256];
   char key[256];
   char value[256];
   fd=fopen(fileName.c_str(),"r");
   if(fd!=NULL) {
      end_record=0;
      while(!feof(fd)) {
         i=0;
         j=0;
         tmp=fgets(buffer,255,fd);
         while(buffer[i]!='\t'&&buffer[i]!='\0') {
            key[j]=buffer[i];
            i++; j++;
         }
         if(buffer[i]=='\0') {
            cout <<  "Invalid record : '" << buffer << "'" << endl;
            break;
         } else {
            i++;
            key[j]='\0';
            j=0;
            while(buffer[i]!='\n') {
               value[j]=buffer[i];
               i++; j++;
            }
            value[j]='\0';
            datas[0][end_record]=key;
            datas[1][end_record]=value;
            end_record++;
         }
      }
      end_record--;
      fclose(fd);
      return(true);
   }
   return(false);
}
