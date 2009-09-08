#ifndef _SETTINGSBACKUP_H_
#define _SETTINGSBACKUP_H_

#include <string>

using namespace std;

#define NB_RECORDS	16

class settingsBackup {
	private :
		string fileName;
		string datas[NB_RECORDS][2];
		int findKey(const char* key);
		void addKey(const char* key, const char* val);
		void serialize();
	public :
		settingsBackup();
		void setKey(const char* key, const char* val);
		const char* getKey(const char* key);
		void deSerialize();
};

#endif
