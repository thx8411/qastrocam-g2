#ifndef _SETTINGSBACKUP_H_
#define _SETTINGSBACKUP_H_

#include <string>

using namespace std;

#define NB_RECORDS	16

class settingsBackup {
	private :
		int end_record;
		string fileName;
		string datas[2][NB_RECORDS];
		int findKey(const char* key);
		void addKey(const char* key, const char* val);
		void serialize();
	public :
		settingsBackup();
		bool haveKey(const char* key);
		void setKey(const char* key, const char* val);
		const char* getKey(const char* key);
		void deSerialize();
};

#endif
