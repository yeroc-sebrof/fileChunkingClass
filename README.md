# fileChunkingClass
A class I've made to point to a file and seek chunks from it individually

Usage:
```
#include 'fileHandler.h'

...

long chunkSizeIfTheDefaultIsntSufficent;

fileHandler example("FileName.Here", chunkSizeIfTheDefaultIsntSufficent);
// At this point your first chunk should be grabbed asynronously

example.waitForRead(); // To wait for file chunk thread

example.buffer; // direct access to the file chunk
                //This is stored as char[]

example.asyncReadNextChunk(); // This can be continuanly called until the file has ran out
// If a task is underway this should still be able to be called again without error even though there is no good reason to do so

void readFirstChunk(); // Will take you back to the first chunk

bool setNextChunkNo(long);	// Will take you to any chunk within the range
```

There is a full test case that should be more specific about how this class should be used but I hope the comments and method names make it clear enough
