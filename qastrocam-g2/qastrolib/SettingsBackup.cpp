#include <iostream>
#include "SettingsBackup.hpp"

using namespace std;

int settingsBackup::findKey(const char* key) {
	int i=0;

	while((i<NB_RECORDS)&&(datas[0][i]!=key)) i++;
	if(i==NB_RECORDS) i=-1;
	return(i);
}

void settingsBackup::addKey(const char* key, const char* val) {
	int i;

	while((i<NB_RECORDS)&&(datas[0][i]!="")) i++;
	if(i==NB_RECORDS) cout << "Settings tab full\n";
	else {
		datas[0][i]=key;
		datas[1][i]=val;
	}
}

void settingsBackup::serialize() {
	FILE* fd;
	int i=0;

	//cout << "Writing settings\n";
	fd=fopen(fileName.c_str(),"w");
	if(fd!=NULL) {
		while(datas[0][i]!="") {
			fprintf(fd,"%s %s",datas[0][i].c_str(),datas[1][i].c_str());
			i++;
		}
		fclose(fd);
	}
}

settingsBackup::settingsBackup() {
	fileName=".qastrocam-g2-settings";
}

void settingsBackup::setKey(const char* key, const char* val) {
	int i;

	i=findKey(key);
	if(i<0) addKey(key,val);
	else
		datas[1][i]=val;
	serialize();
}

const char* settingsBackup::getKey(const char* key) {
	int i;
	i=findKey(key);
	if(i<0) return(NULL);
	return(datas[1][i].c_str());
}

void settingsBackup::deSerialize() {
	FILE* fd;
	int i=0;
	char key[256];
	char value[256];

	//cout << "Loading settings\n";
	fd=fopen(fileName.c_str(),"r");
	if(fd!=NULL) {
		while(feof(fd)) {
			fscanf(fd,"%s",key);
			fscanf(fd,"%s",value);
			datas[0][i]=key;
			datas[1][i]=value;
			i++;
		}
		fclose(fd);
	}
}

