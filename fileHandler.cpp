#include "fileHandler.h"


//fileHandler::fileHandler(string fileName, size_t currChunkSize = 20MB)
fileHandler::fileHandler(string fileNameGiven, unsigned long long int currChunkSize, unsigned short overlaySize)
{
	fileName = fileNameGiven;  // Set this for the debug messages in future
	chunkSize = currChunkSize; // By having a default value we can change it easier for testing
	overlay = overlaySize;


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
	buffer = (uchar*)malloc(sizeof(uchar)*(chunkSize+overlay));
	if (buffer == NULL)
	{
		cerr << "FileHandler: Memory Allocation Error for chunks of size " << chunkSize << "Bytes";
		exit(2);
	}

	// Good to have handy in a variable
	totalChunkCheck();

	readFirstChunk();

	return;
}

void fileHandler::totalChunkCheck()
{
	totalChunks = (fSize - overlay) / chunkSize;

	// When the remainder is the only chunk we need to stop the subtraction
	remainder = (fSize - overlay) % chunkSize;

	// Remainder is still a chunk. Just the GPU's problem of how to propogate that throughout
	if (remainder)
		totalChunks++;
	
	return;
}

// Checks and Error Handling
unsigned long long int fileHandler::checkFileSize()
{
	ufstream ifile(fileName, std::ios::in | std::ios::binary);

	ifile.seekg(0, ifile.end);//seek to end
	//now get current position as length of file
	fSize = ifile.tellg();

#if DEBUG == true
	cout << "FileHandler: " << fileName << " is of size: " << fSize << " Bytes" << endl << endl;
#endif

	ifile.seekg(0, ifile.beg);

	return fSize;
}

void fileHandler::confirmFileSize()
{
	unsigned long long fileSizeBefore = fSize;

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
	rewind(fileToCarve);
	asyncReadNextChunk(true);
	currChunk = 0;
	return;
}

void fileHandler::readNextChunk()
{ // This method should be called asyncrously
	unique_lock<mutex> localLock(m);
	cout << "\nFileHandler: Next Chunk Read started\n";

	fread(buffer, chunkSize, 1, fileToCarve);

	currChunk++;

	cout << "\nFileHandler: Chunk No " << currChunk + 1 << " Loaded out of " << totalChunks << endl;

	return;
}

void fileHandler::asyncReadNextChunk(bool firstChunk)
{
	// Confirm any previous reads have finished
	if (asyncThread.joinable())
		asyncThread.join();
	
	// Start the next read
	currChunk++;
	//cout << endl << "FileHandler: Chunk No " << currChunk << " Started out of " << totalChunks << endl;
	if (firstChunk)
	{
		asyncThread = thread(fread, buffer, chunkSize + overlay, 1, fileToCarve);
	}
	else
	{
		memcpy(buffer, &buffer[chunkSize], overlay); // Copy end of the buffer to the start of the buffer
		asyncThread = thread(fread, &buffer[overlay], chunkSize, 1, fileToCarve); // Read after copy
	}

	return;
}

// Async Wait
void fileHandler::waitForRead()
{
	if (asyncThread.joinable())
	{
		cout << endl << "FileHandler: Waiting for chunk " << currChunk << " to read in" << endl;
		asyncThread.join();
		cout << "Done!" << endl;
	}
	
	return;
}

// Getters and Setters
unsigned long int fileHandler::getTotalChunks()
{
	return totalChunks;
}

unsigned long int fileHandler::getCurrChunkNo()
{
	return currChunk;
}

unsigned int fileHandler::getOverlay()
{
	return overlay;
}

bool fileHandler::setNewChunkNo(unsigned long int newChunkNo)
{
	if (!(newChunkNo < getTotalChunks()))
	{
		return false;
	}

	// Set the file cursor to the newChunkNo chunks into the file
	if (EXIT_SUCCESS == fseek(fileToCarve, (newChunkNo*chunkSize), SEEK_SET))
	{
		currChunk = newChunkNo;
		asyncReadNextChunk(true);

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
