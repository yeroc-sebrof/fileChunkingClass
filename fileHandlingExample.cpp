#include "fileHandler.h"

#define HASHING true

#if HASHING // Using https://github.com/vog/sha1
#include "..\sha1\sha1.hpp"
#endif

#include <future>
#include <vector>

using std::vector;
using std::string;
using std::cout;
using std::cerr;

#define KB (unsigned long int)1024
#define MB (unsigned long int)1024*1024

#define chunksize (20 * MB)

int test1()
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
	fileHandler test("TestFile.test", chunksize);
		
	cout << "Fetch for chunk 0 has started on init" << endl << endl;

#if HASHING // Init the vars
	SHA1 verifyContents;
	vector<string> keepingHashes;
#endif // HASHING


	for (unsigned long int i = 1; i < test.getTotalChunks(); i++)
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
	test.readFirstChunk();
	
	unsigned long int nextChunk = 2;

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

	nextChunk += 5;

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

	cout << (keepingHashes.back() == keepingHashes[nextChunk] ? "true\n" : "false\n");
#elif
	cout << test.buffer;
#endif

	return 0;
}

int test2()
{
	fileHandler a("TestFile.test", 210 * MB);
	fileHandler b("TestFile.test", 21 * MB);

	SHA1 checkera;
	
	string holding;

	char* c = new char[210 * MB];

	for (int i = 0; i < b.getTotalChunks() - 1; i++)
	{
		b.waitForRead();
		std::memcpy(&c[i*(21 * MB)], b.buffer, 21 * MB);
		b.asyncReadNextChunk();
	}

	std::future<string> test = std::async(std::launch::async, [c] { SHA1 checker; checker.update(c); return checker.final(); });

	checkera.update(a.buffer);
	
	cout << test.get() << endl;
	cout << checkera.final() << endl;

	return 0;
}

int test3()
{
	int size = 7;
	int overlaySize = 2;

	int patternMax = 6;
	int patternMin = 4;

	fileHandler abc("abcz.txt", size, overlaySize);
	string qwe;


	for (int i = 0; i < abc.getTotalChunks() - 1; i++)
	{
		abc.waitForRead();
		qwe = abc.buffer;
		cout << qwe.substr(0, size + overlaySize) << endl;
		abc.asyncReadNextChunk();
	}

	qwe = abc.buffer;

	abc.waitForRead();
	if (abc.remainder)
	{
		if (abc.remainder + overlaySize >= patternMin)
		{
			cout << qwe.substr(0, abc.remainder + overlaySize) << endl;
		}
		else
		{
			cout << "Chunk remainder is too small" << endl;
		}
	}
	else
	{
		cout << qwe.substr(0, size + overlaySize) << endl;
	}
	

	return 0;
}

int main(int argc, char** argv)
{
	int test;

	//test = test1();
	
	//if (!test)
	//	return test;

	//test = test2();

	test = test3();

	return test;
}