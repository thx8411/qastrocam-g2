#include <iostream>
#include "SettingsBackup.hpp"

using namespace std;

int settingsBackup::findKey(const char* key) {
	int i=0;

#ifdef _DEBUG_
        cout << "finding key : '" << key << "' position : ";
#endif
	while((i<NB_RECORDS)&&(datas[0][i]!=key)) i++;
	if(i==NB_RECORDS) i=-1;
#ifdef _DEBUG_
        cout << i << endl;
#endif
	return(i);
}

void settingsBackup::addKey(const char* key, const char* val) {
#ifdef _DEBUG_
        cout << "adding key : '" << key << "' value : '" << val << "' position : " << end_record << endl;
#endif
	if(end_record==NB_RECORDS) cout << "Settings tab full\n";
	else {
		datas[0][end_record]=key;
		datas[1][end_record]=val;
		end_record++;
	}
}

void settingsBackup::serialize() {
	FILE* fd;
	int i;

#ifdef _DEBUG_
	cout << "Writing settings\n";
#endif
	fd=fopen(fileName.c_str(),"w");
	if(fd!=NULL) {
		for(i=0;i<end_record;i++)
			fprintf(fd,"%s\t%s\n",datas[0][i].c_str(),datas[1][i].c_str());
		fclose(fd);
	}
#ifdef _DEBUG_
	else {
		perror(" file error : ");
	}
	cout << "writing " << i << " entries\n";
#endif
}

settingsBackup::settingsBackup() {
	end_record=0;
	fileName=".qastrocam-g2-settings";
	for(int i=0;i<NB_RECORDS;i++) datas[0][i]="";
}

void settingsBackup::setKey(const char* key, const char* val) {
	int i;

	i=findKey(key);
	if(i<0) addKey(key,val);
	else {
		datas[1][i]=val;
#ifdef _DEBUG_
        cout << "key '" << key << " modified, new value : '" << val << "'\n";
#endif
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
#ifdef _DEBUG_
        cout << "reading key " << key << " value : '" << datas[1][i].c_str() << "'\n";
#endif
	return(datas[1][i].c_str());
}

void settingsBackup::deSerialize() {
	FILE* fd;
	int i;
	int j;
	char buffer[256];
	char key[256];
	char value[256];

#ifdef _DEBUG_
	cout << "Loading settings\n";
#endif
	fd=fopen(fileName.c_str(),"r");
	if(fd!=NULL) {
		end_record=0;
		while(!feof(fd)) {
			i=0;
			j=0;
			fgets(buffer,255,fd);
			while(buffer[i]!='\t'&&buffer[i]!='\0') {
				key[j]=buffer[i];
				i++; j++;
			}
			if(buffer[i]=='\0') {
				cout <<  "Invalid record : '" << buffer << "'\n" ;
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
#ifdef _DEBUG_
        			cout << "reading  " << key << " value : '" << value << "'\n";
#endif
				datas[0][end_record]=key;
				datas[1][end_record]=value;
				end_record++;
			}
		}
                end_record--;
		fclose(fd);
	}
#ifdef _DEBUG_
	else {
                perror("file error");
        }
	cout << "reading " << end_record << " entries\n"; 
#endif
}

