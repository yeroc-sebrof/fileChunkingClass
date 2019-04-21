#pragma once
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>

#define endl (string)"\n"

#define KB (unsigned long int)1024
#define MB (unsigned long int)1024*1024

#define DEBUG true
#define CLIARGS false

#define fSizeType unsigned long long int

// This was made clearer with https://stackoverflow.com/questions/43095864/using-cout-with-basic-stringunsigned-char
//						  and https://stackoverflow.com/questions/20294746/strings-of-unsigned-chars
#define ustring std::basic_string<unsigned char>
#define uchar unsigned char

using std::string;
using std::cout;
using std::cerr;

using std::vector;

using std::thread;
using std::mutex;
using std::unique_lock;

#define uifstream std::basic_ifstream<unsigned char>

class fileHandler
{
	uifstream ifile;

	//FILE * fileToCarve; // File pointer
	fSizeType chunkSize;
	string fileName;

	unsigned long int currChunk=0;
	unsigned long int totalChunks;
	fSizeType fSize; // File size variable

	unsigned short overlay;

	// For async tasking
	thread asyncThread;
	//bool fetched = true;

	// Methods that don't need to be seen
	void resetPointer();	// return the file pointer to zero
	void readNextChunk();		// reads currChunkNo into buffer and increments the counter
	fSizeType checkFileSize();	// rerun the file size check
	void totalChunkCheck();
public:
	mutex m; // This mutex should ensure safety when handling buffer
	uchar* buffer; // The current block of data read from the file
	unsigned long int remainder;

	fileHandler(string filename, fSizeType=2*MB, unsigned short=0);
	virtual ~fileHandler();

	void confirmFileSize(); // Will check if the file size is equal to the file size given at the start.
	// This should only change in the event the file has been changed as we use it

	void readFirstChunk();		// Reads first chunk Asyncronously and sets the currentChunkNo
	void asyncReadNextChunk(bool=false);	// starts the read of currChunkNo into buffer and increments the counter

	void waitForRead();

	unsigned long int getTotalChunks();	// returns the file size / chunk size rounded up
	unsigned long int getCurrChunkNo();	// returns the int value of ChunkNo
	unsigned int getOverlay();

	bool setNewChunkNo(unsigned long int);	// sets the next chunk to be read (Chunks start at 0)
};

