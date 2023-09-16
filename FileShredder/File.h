#ifndef _File_H
#define _File_H
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <random>
#include "Observer.h"

using namespace std;

class File : public Observable {
private:
	string name;
	string extention;
	string fullName;
	string fullPath;
	size_t length;
	static bool isCanceled;

public:
	File(const string &filePath, Observer& observer);
	virtual ~File() {}
	static void removeFile(const File& file);
	static void secureWipeFile(const File &file, int passes, bool toRemove);
	string getName() { return this->name; }
	string getExtention() { return this->extention; }
	string getFullName() { return this->fullName; }
	string getFullPath() { return this->fullPath; }
	size_t getLength() { return this->length; }
	static bool getIsCanceled() { return isCanceled; }
	static void setIsCanceled(bool state) { isCanceled = state; }
};
#endif
