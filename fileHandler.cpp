#include "fileHandler.h"


//fileHandler::fileHandler(string fileName, size_t currChunkSize = 20MB)
fileHandler::fileHandler(string fileNameGiven, size_t currChunkSize)
{
	fileName = fileNameGiven;  // Set this for the debug messages in future
	chunkSize = currChunkSize; // By having a default value we can change it easier for testing

	// Open the file requested and since this is a C method it doesn't like strings just char arrays
	#pragma warning(suppress : 4996)
	fileToCarve = fopen(fileName.c_str(), "rb");

	// Checking the file opened worked before setting buffers in the event an error occurs
	if (fileToCarve == NULL) // If file didn't open
	{
		cerr << "FileHandler: Error with: " << fileName << endl
			 << "Does this file exist here?" << endl;

	#if defined(_WIN32)
		system("cd");
	#elif defined(__unix__)
		system("pwd");
	#endif // _WIN32
		exit(1);
	}

	// This may need done again later so is kept to a function
	checkFileSize();

	// Setup buffers - Due to the use of C methods and the size of the file chunks
	// sticking to the methods demonstrated in the examples seemed the best approach
	// compared to trying to load the files into CPP char arrays.
	buffer = (char*)malloc(sizeof(char)*chunkSize);
	if (buffer == NULL)
	{
		cerr << "FileHandler: Memory Allocation Error for chunks of size " << chunkSize << "Bytes";
		exit(2);
	}

	// Good to have handy in a variable
	totalChunks = ceil((double)fSize/chunkSize);
	// Remainder is still a chunk. Just the GPU's problem of how to propogate that throughout

	readFirstChunk();

	return;
}

// Checks and Error Handling
long fileHandler::checkFileSize()
{
	rewind(fileToCarve);
	fseek(fileToCarve, 0, SEEK_END);
	fSize = ftell(fileToCarve);

	#if DEBUG == true
	cout << "FileHandler: " <<  fileName << " is of size: " << fSize << " Bytes" << endl << endl;
	#endif 

	rewind(fileToCarve);
	currChunk = 0;
	return fSize;
}

void fileHandler::confirmFileSize()
{
	long fileSizeBefore = fSize;

	if (fileSizeBefore != checkFileSize())
	{
		cerr << "FileHandler: The file has been altered in some way. This is not the file from before; exiting";
		free(buffer); // To be safe
		exit(3);
	}
}

void fileHandler::resetPointer()
{
	rewind(fileToCarve);
	currChunk = 0;
	fread(buffer, chunkSize, 1, fileToCarve);
	return;
}

// Chunk Handling
void fileHandler::readFirstChunk()
{
	asyncReadNextChunk();
	currChunk = 0;
	return;
}

void fileHandler::readNextChunk()
{ // This method should be called asyncrously
	unique_lock<mutex> localLock(m);
	cout << "\nFileHandler: Next Chunk Read started\n";

	fread(buffer, chunkSize, 1, fileToCarve);

	currChunk++;
	fetched = true;
	cout << "\nFileHandler: Chunk No " << currChunk << " Loaded into RAM\n";
	return;
}

void fileHandler::asyncReadNextChunk()
{
	// Confirm any previous reads have finished
	if (asyncThread.joinable())
		asyncThread.join();
	
	// Flag on main that we do not have the new buffer
	fetched = false;
	
	// Start the next read
	asyncThread = thread(fread, buffer, chunkSize, 1, fileToCarve);
	return;
}

// Async Wait -- This shouldn't be needed unless threads can be retasked without being joined
void fileHandler::waitForRead()
{
	if (!fetched) // We must wait for fetch to complete before we can continue
	{
		cout << "\nFileHandler: Waiting for read to complete\n";
		if (asyncThread.joinable())
			asyncThread.join();

		else // This shouldn't be met but if it is then we should be safe anyway
			fetched = true;
	}
	
	return;
}

// Getters and Setters
long fileHandler::getTotalChunks()
{
	return totalChunks;
}

long fileHandler::getCurrChunkNo()
{
	return currChunk;
}


bool fileHandler::setNextChunkNo(long newChunkNo)
{
	if (!(newChunkNo < getTotalChunks()))
	{
		return false;
	}

	// Set the file cursor to the newChunkNo chunks into the file
	if (EXIT_SUCCESS == fseek(fileToCarve, newChunkNo*chunkSize, SEEK_SET))
	{
		currChunk = newChunkNo;
	} else { // If the file was not repointed correctly
		
		cerr << "FileHandler: There was an issue repointing to position " << newChunkNo <<
			" the pointer was reset to the start as a result and execution has continued" << endl;
		resetPointer();
		return false;
	}

	return true;
}


fileHandler::~fileHandler()
{
	free(buffer);
	
	if (asyncThread.joinable())
		asyncThread.join();

	return;
}
