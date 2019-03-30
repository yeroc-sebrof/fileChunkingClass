#include "fileHandler.h"

#define HASHING true

#if HASHING // Using https://github.com/vog/sha1
#include "..\sha1\sha1.hpp"
#endif

#include <vector>

using std::vector;
using std::string;
using std::cout;
using std::cerr;

#define KB (int)1024
#define MB (int)1024*1024

#define chunksize (2 * KB)

int main(int argc, char** argv)
{

	cout << "We are currently in path" << endl;

	#ifdef _WIN32
		system("cd");
	#endif //_WIN32

	#ifdef __unix__
		system("pwd");
	#endif //__unix__

	cout << endl << endl;

	// Init the file handler class
	fileHandler test("TestFile.test");
		
	cout << "Fetch for chunk 0 has started on init" << endl << endl;

#if HASHING // Init the vars
	SHA1 verifyContents;
	vector<string> keepingHashes;
#endif // HASHING


	for (double i = 1; i < test.getTotalChunks(); i++)
	{
		test.waitForRead();

#if HASHING // Don't need to see both every time
		cout << "Hashing current chunk" << endl;
		verifyContents.update(test.buffer);
		keepingHashes.push_back(verifyContents.final());
#elif
		cout << test.buffer;
#endif

		// Lets check the next one
		test.asyncReadNextChunk();
		cout << "We're on chunk " << i << endl;
	}

	// reseting the pointer to the start of the file
	test.resetPointer();
	
	long nextChunk = 2;

	cout << "Setting next chunk to " << nextChunk << endl;

	// setting the chunk to one in
	if (!test.setNextChunkNo(nextChunk))
	{
		cerr << "That didn't work as planned";
		return 1;
	}

	test.asyncReadNextChunk();

	test.waitForRead();

#if HASHING // Don't need to see both every time
	cout << "Hashing Repeated Chunk No: " << nextChunk << endl;
	verifyContents.update(test.buffer);
	keepingHashes.push_back(verifyContents.final());

	cout << (keepingHashes.back() == keepingHashes[nextChunk] ? "true" : "false");
#elif
	cout << test.buffer;
#endif

	nextChunk += 20;

	cout << endl << "Setting next chunk to " << nextChunk << endl;

	// setting the chunk to one in
	if (!test.setNextChunkNo(nextChunk))
	{
		cerr << "That didn't work as planned";
		return 1;
	}

	test.asyncReadNextChunk();

	test.waitForRead();

#if HASHING // Don't need to see both every time
	cout << endl << "Hashing Repeated Chunk No: " << nextChunk << endl;
	verifyContents.update(test.buffer);
	keepingHashes.push_back(verifyContents.final());

	cout << (keepingHashes.back() == keepingHashes[nextChunk] ? "true" : "false");
#elif
	cout << test.buffer;
#endif

	return 0;
}
