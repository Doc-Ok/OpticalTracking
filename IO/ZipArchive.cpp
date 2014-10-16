/***********************************************************************
ZipArchive - Class to represent ZIP archive files, with functionality to
traverse contained directory hierarchies and extract files using a File
interface.
Copyright (c) 2011-2014 Oliver Kreylos

This file is part of the I/O Support Library (IO).

The I/O Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The I/O Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the I/O Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <IO/ZipArchive.h>

#include <string.h>
#include <zlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <Misc/SizedTypes.h>
#include <Misc/ThrowStdErr.h>
#include <IO/StandardFile.h>
#include <IO/FixedMemoryFile.h>

namespace IO {

namespace {

/**************
Helper classes:
**************/

/********************************************************
Class to read ZIP archive entries in a streaming fashion:
********************************************************/

class ZipArchiveStreamingFile:public File
	{
	/* Embedded classes: */
	public:
	typedef SeekableFile::Offset Offset; // Type for file positions
	
	/* Elements: */
	private:
	SeekableFilePtr archive; // Reference to the ZIP archive containing the file
	Offset nextReadPos; // Position of next data block to read from archive
	size_t compressedSize; // Amount of data remaining to be read from archive
	size_t compressedBufferSize; // Size of allocated buffer for compressed data read from the archive
	Bytef* compressedBuffer; // Buffer for compressed data read from the archive
	z_stream* stream; // Zlib decompression object; only allocated if file is compressed
	bool eof; // Flag set to true when all uncompressed data has been read
	
	/* Protected methods from File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	ZipArchiveStreamingFile(SeekableFilePtr sArchive,unsigned int sCompressionMethod,Offset sNextReadPos,size_t sCompressedSize);
	virtual ~ZipArchiveStreamingFile(void);
	};

/****************************************
Methods of class ZipArchiveStreamingFile:
****************************************/

size_t ZipArchiveStreamingFile::readData(File::Byte* buffer,size_t bufferSize)
	{
	if(eof)
		return 0;
	
	if(stream!=0)
		{
		/* Decompress data into the provided buffer: */
		stream->next_out=buffer;
		stream->avail_out=bufferSize;
		
		/* Repeat until some output is produced: */
		do
			{
			/* Check if the decompressor needs more input: */
			if(stream->avail_in==0)
				{
				/* Read the next chunk of compressed data from the archive: */
				size_t compressedReadSize=compressedBufferSize;
				if(compressedReadSize>compressedSize)
					compressedReadSize=compressedSize;
				archive->setReadPosAbs(nextReadPos);
				compressedReadSize=archive->readUpTo(compressedBuffer,compressedReadSize);
				nextReadPos+=compressedReadSize;
				compressedSize-=compressedReadSize;
				
				/* Pass the compressed data to the decompressor: */
				stream->next_in=compressedBuffer;
				stream->avail_in=compressedReadSize;
				}
			
			/* Decompress: */
			int result=inflate(stream,Z_NO_FLUSH);
			if(result==Z_STREAM_END)
				{
				/* Clean out the decompressor and set the eof flag: */
				if(inflateEnd(stream)!=Z_OK)
					Misc::throwStdErr("IO::ZipArchiveStreamingFile: Data corruption detected after decompressing");
				eof=true;
				break;
				}
			else if(result!=Z_OK)
				Misc::throwStdErr("IO::ZipArchiveStreamingFile: Internal zlib error while decompressing");
			}
		while(stream->avail_out==bufferSize);
		
		return bufferSize-stream->avail_out;
		}
	else
		{
		/* Read uncompressed data from the archive directly into the provided buffer: */
		size_t readSize=bufferSize;
		if(readSize>compressedSize)
			readSize=compressedSize;
		archive->setReadPosAbs(nextReadPos);
		readSize=archive->readUpTo(buffer,readSize);
		nextReadPos+=readSize;
		compressedSize-=readSize;
		eof=compressedSize==0;
		
		return readSize;
		}
	}

void ZipArchiveStreamingFile::writeData(const File::Byte* buffer,size_t bufferSize)
	{
	/* Writing is not supported; ignore silently */
	}

ZipArchiveStreamingFile::ZipArchiveStreamingFile(SeekableFilePtr sArchive,unsigned int sCompressionMethod,SeekableFile::Offset sNextReadPos,size_t sCompressedSize)
	:File(ReadOnly),
	 archive(sArchive),
	 nextReadPos(sNextReadPos),compressedSize(sCompressedSize),
	 compressedBufferSize(8192),compressedBuffer(sCompressionMethod!=0?new Bytef[compressedBufferSize]:0),
	 stream(0),eof(false)
	{
	if(sCompressionMethod!=0)
		{
		/* Read the first chunk of compressed data from the archive: */
		size_t compressedReadSize=compressedBufferSize;
		if(compressedReadSize>compressedSize)
			compressedReadSize=compressedSize;
		archive->setReadPosAbs(nextReadPos);
		compressedReadSize=archive->readUpTo(compressedBuffer,compressedReadSize);
		nextReadPos+=compressedReadSize;
		compressedSize-=compressedReadSize;
		
		/* Create and initialize the zlib decompression object: */
		stream=new z_stream;
		memset(stream,0,sizeof(z_stream));
		stream->next_in=compressedBuffer;
		stream->avail_in=compressedReadSize;
		stream->zalloc=0;
		stream->zfree=0;
		stream->opaque=0;
		if(inflateInit2(stream,-MAX_WBITS)!=Z_OK)
			{
			delete[] compressedBuffer;
			delete stream;
			Misc::throwStdErr("IO::ZipArchiveStreamingFile: Internal zlib error while initializing decompression");
			}
		}
	}

ZipArchiveStreamingFile::~ZipArchiveStreamingFile(void)
	{
	delete[] compressedBuffer;
	delete stream;
	}

}

/**************************************************************************************
Class to represent directories inside a ZIP archive using an IO::Directory abstraction:
**************************************************************************************/

class ZipArchiveDirectory:public Directory
	{
	/* Embedded classes: */
	private:
	typedef ZipArchive::Offset Offset;
	
	/* Elements: */
	ZipArchivePtr archive; // The ZIP archive from which this directory was extracted
	ZipArchive::Directory* directory; // Pointer to this directory's node in the ZIP archive's directory tree
	std::vector<ZipArchive::Directory::Entry>::iterator currentEntry; // Iterator to the currently enumerated directory entry
	
	/* Constructors and destructors: */
	public:
	ZipArchiveDirectory(ZipArchivePtr sArchive,ZipArchive::Directory* sDirectory)
		:archive(sArchive),
		 directory(sDirectory),
		 currentEntry(directory->entries.end())
		{
		}
	
	/* Methods from Directory: */
	virtual std::string getName(void) const;
	virtual std::string getPath(void) const;
	virtual std::string getPath(const char* relativePath) const;
	virtual bool hasParent(void) const;
	virtual DirectoryPtr getParent(void) const;
	virtual void rewind(void);
	virtual bool readNextEntry(void);
	virtual const char* getEntryName(void) const;
	virtual Misc::PathType getEntryType(void) const;
	virtual Misc::PathType getPathType(const char* relativePath) const;
	virtual FilePtr openFile(const char* fileName,File::AccessMode accessMode =File::ReadOnly) const;
	virtual DirectoryPtr openDirectory(const char* directoryName) const;
	};

/************************************
Methods of class ZipArchiveDirectory:
************************************/

std::string ZipArchiveDirectory::getName(void) const
	{
	/* Check for the special case of the root directory: */
	if(directory->parent==0)
		return "/";
	else
		{
		/* Return the name of the directory entry corresponding to this directory in the parent directory: */
		return directory->parent->entries[directory->parentIndex].name;
		}
	}

std::string ZipArchiveDirectory::getPath(void) const
	{
	/* Get the path of the directory: */
	std::string path;
	directory->getPath(path,0);
	
	/* Remove the trailing slash if the directory is not the root: */
	if(directory->parent!=0)
		path.erase(path.end()-1);
	
	return path;
	}

std::string ZipArchiveDirectory::getPath(const char* relativePath) const
	{
	std::string result;
	
	/* Check if the given path is absolute: */
	if(relativePath[0]=='/')
		{
		/* Return the relative path: */
		result=relativePath;
		}
	else
		{
		/* Concatenate the directory's path and the given relative path: */
		directory->getPath(result,strlen(relativePath));
		result.append(relativePath);
		}
	
	/* Normalize and return the result path: */
	normalizePath(result,1);
	return result;
	}

bool ZipArchiveDirectory::hasParent(void) const
	{
	return directory->parent!=0;
	}

DirectoryPtr ZipArchiveDirectory::getParent(void) const
	{
	/* Check for the special case of the root directory: */
	if(directory->parent==0)
		return 0;
	else
		return new ZipArchiveDirectory(archive,directory->parent);
	}

void ZipArchiveDirectory::rewind(void)
	{
	/* Reset the current entry iterator to the end of the entries array so that the next call to readNextEntry returns the first entry: */
	currentEntry=directory->entries.end();
	}

bool ZipArchiveDirectory::readNextEntry(void)
	{
	/* Increment the current entry iterator or wrap it around at the end of the directory: */
	if(currentEntry!=directory->entries.end())
		++currentEntry;
	else
		currentEntry=directory->entries.begin();
	
	return currentEntry!=directory->entries.end();
	}

const char* ZipArchiveDirectory::getEntryName(void) const
	{
	return currentEntry->name;
	}

Misc::PathType ZipArchiveDirectory::getEntryType(void) const
	{
	if(currentEntry->filePos!=~Offset(0))
		return Misc::PATHTYPE_FILE;
	else
		return Misc::PATHTYPE_DIRECTORY;
	}

Misc::PathType ZipArchiveDirectory::getPathType(const char* relativePath) const
	{
	/* Find the relative path's target in the ZIP archive's directory tree: */
	std::pair<ZipArchive::Directory*,unsigned int> entry=directory->findPath(relativePath);
	
	/* Check for the special case of the root directory: */
	if(entry.first==0&&entry.second==1)
		return Misc::PATHTYPE_DIRECTORY;
	else if(entry.first==0)
		return Misc::PATHTYPE_DOES_NOT_EXIST;
	else
		{
		/* Check the type of the found directory entry: */
		if(entry.first->entries[entry.second].filePos==~Offset(0))
			return Misc::PATHTYPE_DIRECTORY;
		else
			return Misc::PATHTYPE_FILE;
		}
	}

FilePtr ZipArchiveDirectory::openFile(const char* fileName,File::AccessMode accessMode) const
	{
	/* Check the requested access mode: */
	if(accessMode==IO::File::WriteOnly||accessMode==IO::File::ReadWrite)
		throw IO::File::OpenError(Misc::printStdErrMsg("IO::ZipArchiveDirectory::openFile: Cannot write to file %s",fileName));
	
	/* Find the file name in the ZIP archive's directory tree: */
	std::pair<ZipArchive::Directory*,unsigned int> entry=directory->findPath(fileName);
	
	/* Check if it exists and is a file: */
	if(entry.first!=0)
		{
		if(entry.first->entries[entry.second].filePos!=~Offset(0))
			{
			/* Open and return the file: */
			ZipArchive::FileID fileId;
			fileId.filePos=entry.first->entries[entry.second].filePos;
			fileId.compressedSize=entry.first->entries[entry.second].sizes.compressed;
			fileId.uncompressedSize=entry.first->entries[entry.second].sizes.uncompressed;
			return archive->openFile(fileId);
			}
		else
			throw IO::File::OpenError(Misc::printStdErrMsg("IO::ZipArchiveDirectory::openFile: File %s is a directory",fileName));
		}
	else if(entry.second==1)
		throw IO::File::OpenError(Misc::printStdErrMsg("IO::ZipArchiveDirectory::openFile: File %s is a directory",fileName));
	else
		throw IO::File::OpenError(Misc::printStdErrMsg("IO::ZipArchiveDirectory::openFile: File %s does not exist",fileName));
	}

DirectoryPtr ZipArchiveDirectory::openDirectory(const char* directoryName) const
	{
	/* Find the directory name in the ZIP archive's directory tree: */
	std::pair<ZipArchive::Directory*,unsigned int> entry=directory->findPath(directoryName);
	
	/* Check if the directory exists: */
	if(entry.first!=0&&entry.first->entries[entry.second].filePos==~Offset(0))
		{
		/* Return the found directory: */
		return new ZipArchiveDirectory(archive,entry.first->entries[entry.second].child);
		}
	else if(entry.first==0&&entry.second==1)
		{
		/* Return the root directory: */
		return new ZipArchiveDirectory(archive,&archive->root);
		}
	else
		throw OpenError(directoryName);
	}

/**************************************
Methods of class ZipArchive::Directory:
**************************************/

ZipArchive::Directory::Directory(ZipArchive::Directory* sParent)
	:parent(sParent)
	{
	}

ZipArchive::Directory::~Directory(void)
	{
	/* Go through all directory entries to delete entry names and subdirectories: */
	for(std::vector<Entry>::iterator eIt=entries.begin();eIt!=entries.end();++eIt)
		{
		/* Delete the entry's name: */
		delete[] eIt->name;
		
		/* Check if the entry is a subdirectory: */
		if(eIt->filePos==~Offset(0))
			{
			/* Delete the subdirectory: */
			delete eIt->child;
			}
		}
	}

bool ZipArchive::Directory::addPath(const char* path,const ZipArchive::FileID& fileId)
	{
	/* Find the end of the first path component: */
	const char* nameEnd;
	for(nameEnd=path;*nameEnd!='\0'&&*nameEnd!='/';++nameEnd)
		;
	size_t nameLen=nameEnd-path;
	
	/* Check if an entry matching the first path component already exists: */
	std::vector<Entry>::iterator eIt;
	for(eIt=entries.begin();eIt!=entries.end()&&(strncmp(eIt->name,path,nameLen)!=0||eIt->name[nameLen]!='\0');++eIt)
		;
	if(eIt==entries.end())
		{
		/* Create a new directory entry of the given name: */
		Entry newEntry;
		newEntry.name=new char[nameLen+1];
		memcpy(newEntry.name,path,nameLen);
		newEntry.name[nameLen]='\0';
		
		/* Check if the new entry is a file or directory: */
		if(path[nameLen]=='\0')
			{
			/* Create a file entry: */
			newEntry.filePos=fileId.filePos;
			newEntry.sizes.compressed=fileId.compressedSize;
			newEntry.sizes.uncompressed=fileId.uncompressedSize;
			}
		else
			{
			/* Create a directory entry: */
			newEntry.filePos=~Offset(0);
			newEntry.child=new ZipArchive::Directory(this);
			}
		
		entries.push_back(newEntry);
		
		return true;
		}
	else if(path[nameLen]=='/'&&eIt->filePos==~Offset(0))
		{
		/* Recursively add the rest of the path to the found subdirectory: */
		return eIt->child->addPath(path+nameLen+1,fileId);
		}
	else
		return false;
	}

void ZipArchive::Directory::finalize(void)
	{
	/* Sort the directory entries by name: */
	std::sort(entries.begin(),entries.end(),Entry::compare);
	
	/* Fix the parent directory indices of all subdirectories, and finalize them recursively: */
	unsigned int numEntries=entries.size();
	for(unsigned int i=0;i<numEntries;++i)
		if(entries[i].filePos==~Offset(0))
			{
			entries[i].child->parentIndex=i;
			entries[i].child->finalize();
			}
	}

void ZipArchive::Directory::getPath(std::string& path,size_t suffixLen) const
	{
	if(parent==0)
		{
		/* Create the result string: */
		++suffixLen;
		path.clear();
		path.reserve(suffixLen);
		path.push_back('/');
		}
	else
		{
		/* Add the length of this directory's name to the suffix length and recurse into the parent directory: */
		suffixLen+=strlen(parent->entries[parentIndex].name)+1;
		parent->getPath(path,suffixLen);
		
		/* Append the name of this directory to the path: */
		path.append(parent->entries[parentIndex].name);
		path.push_back('/');
		}
	}

std::pair<ZipArchive::Directory*,unsigned int> ZipArchive::Directory::findPath(const char* path)
	{
	/* Check if the given path is absolute: */
	Directory* currentDir=this;
	if(path[0]=='/')
		{
		/* Start searching from the root directory: */
		while(currentDir->parent!=0)
			currentDir=currentDir->parent;
		
		/* Skip the initial slash: */
		++path;
		}
	
	/* Skip any '/' or "./" at the beginning of the path: */
	while(path[0]=='/'||(path[0]=='.'&&path[1]=='/'))
		{
		if(*path=='.')
			++path;
		++path;
		}
	
	/* Traverse through the directory tree to find the file one prefix at a time: */
	const char* prefixStart=path;
	while(*prefixStart!='\0')
		{
		/* Extract the current prefix: */
		const char* prefixEnd;
		for(prefixEnd=prefixStart+1;*prefixEnd!='\0'&&*prefixEnd!='/';++prefixEnd)
			;
		size_t prefixLen=prefixEnd-prefixStart;
		
		/* Find the beginning of the next prefix: */
		const char* nextPrefixStart=prefixEnd;
		while(nextPrefixStart[0]=='/'||(nextPrefixStart[0]=='.'&&nextPrefixStart[1]=='/'))
			{
			if(*nextPrefixStart=='.')
				++nextPrefixStart;
			++nextPrefixStart;
			}
		
		/* Check for the ".." special case: */
		if(prefixLen==2&&prefixStart[0]=='.'&&prefixStart[1]=='.')
			{
			/* Continue searching in the parent directory: */
			if(currentDir->parent==0)
				return std::pair<Directory*,unsigned int>(0,0);
			currentDir=currentDir->parent;
			}
		else
			{
			/* Find the current prefix in the current directory's entries via binary search with the invariant l->name<=prefix<r->name: */
			std::vector<Directory::Entry>::const_iterator l=currentDir->entries.begin();
			std::vector<Directory::Entry>::const_iterator r=currentDir->entries.end();
			bool found=false;
			if(r-l==1)
				found=strncmp(l->name,prefixStart,prefixLen)==0&&l->name[prefixLen]=='\0';
			else
				{
				do
					{
					/* Compare the middle element to the current prefix: */
					std::vector<Directory::Entry>::const_iterator m=l+(r-l)/2;
					int comp=strncmp(m->name,prefixStart,prefixLen);
					if(comp==0&&m->name[prefixLen]!='\0')
						comp=1;
					found=comp==0;
					if(comp<=0)
						l=m;
					else
						r=m;
					}
				while(r-l>1);
				}
			if(!found)
				return std::pair<Directory*,unsigned int>(0,0);
			
			/* Check if the path is completely processed: */
			if(*nextPrefixStart=='\0')
				{
				/* Return the found directory entry: */
				return std::pair<Directory*,unsigned int>(currentDir,l-currentDir->entries.begin());
				}
			else
				{
				/* Check if the found entry is a subdirectory: */
				if(l->filePos==~Offset(0))
					{
					/* Continue searching in the subdirectory: */
					currentDir=l->child;
					}
				else
					return std::pair<Directory*,unsigned int>(0,0);
				}
			}
		
		/* Go to the next prefix: */
		prefixStart=nextPrefixStart;
		}
	
	/* Return a special code if the relative path was the root directory: */
	return std::pair<Directory*,unsigned int>(0,1);
	}

/**********************************************
Methods of class ZipArchive::DirectoryIterator:
**********************************************/

void ZipArchive::DirectoryIterator::getEntry(void)
	{
	/* Store the name of the current directory entry: */
	size_t nameLen=strlen(directory->entries[entryIndex].name);
	if(fileNameBufferSize<pathEnd+nameLen+1)
		{
		/* Make room in the file name buffer: */
		do
			{
			fileNameBufferSize*=2;
			}
		while(fileNameBufferSize<pathEnd+nameLen+1);
		char* newFileName=new char[fileNameBufferSize];
		memcpy(newFileName,fileName,pathEnd);
		delete[] fileName;
		fileName=newFileName;
		}
	memcpy(fileName+pathEnd,directory->entries[entryIndex].name,nameLen+1);
	
	/* Store the current directory entry's file ID: */
	filePos=directory->entries[entryIndex].filePos;
	if(filePos!=~Offset(0))
		{
		compressedSize=directory->entries[entryIndex].sizes.compressed;
		uncompressedSize=directory->entries[entryIndex].sizes.uncompressed;
		}
	else
		compressedSize=uncompressedSize=0;
	}

ZipArchive::DirectoryIterator::DirectoryIterator(const ZipArchive& archive)
	:directory(&archive.root),entryIndex(0),
	 fileNameBufferSize(64),fileName(new char[fileNameBufferSize])
	{
	/* Put the root directory name into the file name buffer: */
	fileName[0]='/';
	fileName[1]='\0';
	pathEnd=1;
	
	/* Check if the iterator is already finished: */
	if(directory->entries.empty())
		{
		/* Invalidate the iterator: */
		directory=0;
		}
	else
		{
		/* Get the root directory's first entry: */
		getEntry();
		}
	}

ZipArchive::DirectoryIterator::DirectoryIterator(const ZipArchive::DirectoryIterator& source)
	:FileID(source),
	 directory(source.directory),entryIndex(source.entryIndex),
	 fileNameBufferSize(source.fileNameBufferSize),fileName(fileNameBufferSize!=0?new char[fileNameBufferSize]:0),
	 pathEnd(source.pathEnd)
	{
	if(fileNameBufferSize!=0)
		memcpy(fileName,source.fileName,fileNameBufferSize);
	}

ZipArchive::DirectoryIterator& ZipArchive::DirectoryIterator::operator=(const ZipArchive::DirectoryIterator& source)
	{
	if(this!=&source)
		{
		FileID::operator=(source);
		directory=source.directory;
		entryIndex=source.entryIndex;
		if(source.fileNameBufferSize==0)
			{
			delete[] fileName;
			fileNameBufferSize=0;
			fileName=0;
			}
		else if(source.fileNameBufferSize>fileNameBufferSize)
			{
			delete[] fileName;
			fileNameBufferSize=source.fileNameBufferSize;
			fileName=new char[fileNameBufferSize];
			}
		if(fileNameBufferSize!=0)
			memcpy(fileName,source.fileName,fileNameBufferSize);
		pathEnd=source.pathEnd;
		}
	return *this;
	}

ZipArchive::DirectoryIterator& ZipArchive::DirectoryIterator::operator++(void)
	{
	/* Bail out if iterator is invalid: */
	if(directory==0)
		return *this;
	
	/* Check if the current directory entry is a non-empty subdirectory: */
	if(filePos==~Offset(0)&&!directory->entries[entryIndex].child->entries.empty())
		{
		/* Traverse into the subdirectory: */
		pathEnd+=strlen(directory->entries[entryIndex].name);
		fileName[pathEnd]='/';
		++pathEnd;
		directory=directory->entries[entryIndex].child;
		entryIndex=0;
		}
	else
		{
		/* Go to the next entry in the current subdirectory: */
		++entryIndex;
		}
	
	/* Check if the current directory has been completed: */
	while(directory!=0&&entryIndex==directory->entries.size())
		{
		/* Go back to the parent directory: */
		if(directory->parent!=0)
			pathEnd-=strlen(directory->parent->entries[directory->parentIndex].name)+1;
		else
			pathEnd=1;
		entryIndex=directory->parentIndex+1;
		directory=directory->parent;
		}
	
	/* Get the next directory entry if the iterator is still valid: */
	if(directory!=0)
		getEntry();
	
	return *this;
	}

/**********************************************
Methods of class ZipArchive::FileNotFoundError:
**********************************************/

ZipArchive::FileNotFoundError::FileNotFoundError(const char* sFileName)
	:std::runtime_error(Misc::printStdErrMsg("IO::ZipArchive::findFile: File %s not found in archive",sFileName)),
	 fileName(new char[strlen(sFileName)+1])
	{
	strcpy(fileName,sFileName);
	}

ZipArchive::FileNotFoundError::~FileNotFoundError(void) throw()
	{
	delete[] fileName;
	}

/***************************
Methods of class ZipArchive:
***************************/

int ZipArchive::initArchive(void)
	{
	/* Set the archive file's endianness: */
	archive->setEndianness(Misc::LittleEndian);
	
	/* Check the first local file header's signature, to check if it's a zip file in the first place: */
	unsigned int signature=archive->read<Misc::UInt32>();
	if(signature!=0x04034b50U)
		return -1;
	
	/* Read backwards from end of file until end-of-directory signature is found: */
	Offset archiveSize=archive->getSize();
	Offset readPos=archiveSize;
	Offset firstReadPos=readPos>Offset(70000)?readPos-Offset(70000):Offset(0); // If no signature is found after this pos, there is none
	unsigned char readBuffer[256];
	unsigned char* rbPtr=readBuffer;
	int state=0;
	while(state!=4)
		{
		/* Get the previous byte from the buffer: */
		if(rbPtr==readBuffer)
			{
			/* Read another chunk of data from the file: */
			size_t readSize=sizeof(readBuffer);
			if(size_t(readPos-firstReadPos)<readSize)
				readSize=size_t(readPos-firstReadPos);
			if(readSize==0) // Haven't found the signature, and there's nothing more to read
				return -2;
			readPos-=Offset(readSize);
			archive->setReadPosAbs(readPos);
			archive->read(readBuffer,readSize);
			rbPtr=readBuffer+readSize;
			}
		--rbPtr;
		
		/* Process the byte through the state machine: */
		switch(state)
			{
			case 0: // Nothing matching
				if(*rbPtr==0x06U)
					state=1;
				break;
			
			case 1: // Read 0x06
				if(*rbPtr==0x05U)
					state=2;
				else if(*rbPtr!=0x06U)
					state=0;
				break;
			
			case 2: // Read 0x0605
				if(*rbPtr==0x4bU)
					state=3;
				else if(*rbPtr==0x06U)
					state=1;
				else
					state=0;
				break;
			
			case 3: // Read 0x06054b
				if(*rbPtr==0x50U)
					state=4;
				else if(*rbPtr==0x06U)
					state=1;
				else
					state=0;
			}
		}
	Offset endOfCentralDirPos=readPos+Offset(rbPtr-readBuffer);
	
	/* Read the end-of-central-directory entry: */
	archive->setReadPosAbs(endOfCentralDirPos);
	unsigned int eocdSignature=archive->read<Misc::UInt32>();
	if(eocdSignature!=0x06054b50U)
		return -3;
	
	/* Skip irrelevant bits: */
	archive->skip<Misc::UInt16>(4);
	
	/* Read the relevant bits: */
	unsigned int eocdCDSize=archive->read<Misc::UInt32>();
	unsigned int eocdCDOffset=archive->read<Misc::UInt32>();
	unsigned short eocdCommentLength=archive->read<Misc::UInt16>();
	
	/* Remember the directory offset and size: */
	Offset directoryPos=Offset(eocdCDOffset);
	size_t directorySize=size_t(eocdCDSize);
	
	/* Check again if this was really the end-of-directory marker: */
	if(directoryPos+Offset(directorySize)!=endOfCentralDirPos||endOfCentralDirPos+Offset(sizeof(Misc::UInt32)*3+sizeof(Misc::UInt16)*5+eocdCommentLength)!=archiveSize)
		return -3;
	
	/*************************************************
	Read the ZIP archive's entire directory hierarchy:
	*************************************************/
	
	/* Read all central directory entries and add them into the directory tree: */
	bool directoryOk=true;
	size_t fileNameBufferSize=256;
	char* fileNameBuffer=new char[fileNameBufferSize];
	archive->setReadPosAbs(directoryPos);
	do
		{
		/* Read the next central directory entry header: */
		unsigned int entryHeader=archive->read<Misc::UInt32>();
		if(entryHeader==0x05054b50U||entryHeader==0x06054b50U) // Digital signature entry or end-of-central-directory entry
			break;
		else if(entryHeader==0x02014b50U) // File entry
			{
			/* Read the rest of the header: */
			FileID newFileId;
			archive->skip<Misc::UInt16>(6);
			archive->skip<Misc::UInt32>(1);
			newFileId.compressedSize=size_t(archive->read<Misc::UInt32>());
			newFileId.uncompressedSize=size_t(archive->read<Misc::UInt32>());
			unsigned short fileNameLength=archive->read<Misc::UInt16>();
			unsigned short extraFieldLength=archive->read<Misc::UInt16>();
			unsigned short fileCommentLength=archive->read<Misc::UInt16>();
			archive->skip<Misc::UInt16>(2);
			archive->skip<Misc::UInt32>(1);
			newFileId.filePos=Offset(archive->read<Misc::UInt32>());
			
			/* Read the file name: */
			if(fileNameBufferSize<size_t(fileNameLength+1))
				{
				delete[] fileNameBuffer;
				do
					{
					fileNameBufferSize*=2;
					}
				while(fileNameBufferSize<size_t(fileNameLength+1));
				fileNameBuffer=new char[fileNameBufferSize];
				}
			archive->read(fileNameBuffer,fileNameLength);
			fileNameBuffer[fileNameLength]='\0';
			
			/* Skip extra field and file comment: */
			archive->skip<char>(extraFieldLength);
			archive->skip<char>(fileCommentLength);
			
			/* Add the new file to the directory tree: */
			directoryOk=root.addPath(fileNameBuffer,newFileId);
			}
		else
			directoryOk=false;
		}
	while(directoryOk);
	delete[] fileNameBuffer;
	
	if(!directoryOk)
		{
		/* Signal failure: */
		return -3;
		}
	
	/* Finalize the directory tree: */
	root.finalize();
	
	/* Signal success: */
	return 0;
	}

ZipArchive::ZipArchive(const char* archiveFileName)
	:archive(new StandardFile(archiveFileName,File::ReadOnly)),
	 root(0)
	{
	/* Initialize the archive and handle errors: */
	switch(initArchive())
		{
		case -1:
			Misc::throwStdErr("IO::ZipArchive: %s is not a valid ZIP archive",archiveFileName);
			break;
		
		case -2:
			Misc::throwStdErr("IO::ZipArchive: Unable to locate central directory in ZIP archive %s",archiveFileName);
			break;
		
		case -3:
			Misc::throwStdErr("IO::ZipArchive: Invalid central directory in ZIP archive %s",archiveFileName);
			break;
		}
	}

ZipArchive::ZipArchive(SeekableFilePtr sArchive)
	:archive(sArchive),
	 root(0)
	{
	/* Initialize the archive and handle errors: */
	switch(initArchive())
		{
		case -1:
			Misc::throwStdErr("IO::ZipArchive: Source file is not a valid ZIP archive");
			break;
		
		case -2:
			Misc::throwStdErr("IO::ZipArchive: Unable to locate central directory in ZIP archive");
			break;
		
		case -3:
			Misc::throwStdErr("IO::ZipArchive: Invalid central directory in ZIP archive");
			break;
		}
	}

ZipArchive::~ZipArchive(void)
	{
	}

ZipArchive::FileID ZipArchive::findFile(const char* fileName) const
	{
	/* Find the file name in the directory tree: */
	std::pair<Directory*,unsigned int> entry=const_cast<Directory*>(&root)->findPath(fileName); // const_cast is OK here because the method doesn't actually change the object
	
	/* Check if it exists and is a file: */
	if(entry.first!=0&&entry.first->entries[entry.second].filePos!=~Offset(0))
		{
		/* Return the file's ID: */
		FileID fileId;
		fileId.filePos=entry.first->entries[entry.second].filePos;
		fileId.compressedSize=entry.first->entries[entry.second].sizes.compressed;
		fileId.uncompressedSize=entry.first->entries[entry.second].sizes.uncompressed;
		return fileId;
		}
	else
		throw FileNotFoundError(fileName);
	}

FilePtr ZipArchive::openFile(const ZipArchive::FileID& fileId)
	{
	/* Read the file's header: */
	archive->setReadPosAbs(fileId.filePos);
	if(archive->read<Misc::UInt32>()!=0x04034b50U)
		Misc::throwStdErr("IO::ZipArchive::openFile: Invalid file header signature");
	
	/* Read file header information: */
	archive->skip<Misc::UInt16>(2);
	unsigned short compressionMethod=archive->read<Misc::UInt16>();
	archive->skip<Misc::UInt16>(2);
	archive->skip<Misc::UInt32>(1);
	unsigned int compressedSize=archive->read<Misc::UInt32>();
	archive->skip<Misc::UInt32>(1);
	unsigned short fileNameLength=archive->read<Misc::UInt16>();
	unsigned short extraFieldLength=archive->read<Misc::UInt16>();
	
	/* Skip file name and extra field: */
	archive->skip<char>(fileNameLength);
	archive->skip<char>(extraFieldLength);
	
	/* Create and return the result file: */
	return new ZipArchiveStreamingFile(archive,compressionMethod,archive->getReadPos(),compressedSize);
	}

SeekableFilePtr ZipArchive::openSeekableFile(const ZipArchive::FileID& fileId)
	{
	/* Read the file's header: */
	archive->setReadPosAbs(fileId.filePos);
	if(archive->read<Misc::UInt32>()!=0x04034b50U)
		Misc::throwStdErr("IO::ZipArchive::openSeekableFile: Invalid file header signature");
	
	/* Read file header information: */
	archive->skip<Misc::UInt16>(2);
	unsigned short compressionMethod=archive->read<Misc::UInt16>();
	archive->skip<Misc::UInt16>(2);
	archive->skip<Misc::UInt32>(1);
	unsigned int compressedSize=archive->read<Misc::UInt32>();
	unsigned int uncompressedSize=archive->read<Misc::UInt32>();
	unsigned short fileNameLength=archive->read<Misc::UInt16>();
	unsigned short extraFieldLength=archive->read<Misc::UInt16>();
	
	/* Skip file name and extra field: */
	archive->skip<char>(fileNameLength);
	archive->skip<char>(extraFieldLength);
	
	/* Create the result file: */
	FixedMemoryFile* result=new FixedMemoryFile(uncompressedSize);
	if(compressionMethod==0)
		{
		/* Directly read the uncompressed data: */
		archive->read<char>(static_cast<char*>(result->getMemory()),compressedSize);
		}
	else
		{
		/* Read the compressed data: */
		Bytef* compressed=new Bytef[compressedSize];
		archive->read<Bytef>(compressed,compressedSize);
		
		/* Uncompress the data: */
		z_stream stream;
		stream.zalloc=0;
		stream.zfree=0;
		stream.opaque=0;
		if(inflateInit2(&stream,-MAX_WBITS)!=Z_OK)
			{
			delete[] compressed;
			delete result;
			Misc::throwStdErr("IO::ZipArchive::openSeekableFile: Internal zlib error");
			}
		stream.next_in=compressed;
		stream.avail_in=compressedSize;
		stream.next_out=static_cast<Bytef*>(result->getMemory());
		stream.avail_out=uncompressedSize;
		if(inflate(&stream,Z_FINISH)!=Z_STREAM_END)
			{
			delete[] compressed;
			delete result;
			Misc::throwStdErr("IO::ZipArchive::openSeekableFile: Internal zlib error");
			}
		delete[] compressed;
		if(inflateEnd(&stream)!=Z_OK)
			{
			delete result;
			Misc::throwStdErr("IO::ZipArchive::openSeekableFile: Internal zlib error");
			}
		}
	
	return result;
	}

DirectoryPtr ZipArchive::openRootDirectory(void)
	{
	/* Return a new directory object: */
	return new ZipArchiveDirectory(this,&root);
	}

DirectoryPtr ZipArchive::openDirectory(const char* directoryName)
	{
	/* Find the directory name in the directory tree: */
	std::pair<Directory*,unsigned int> entry=root.findPath(directoryName);
	
	/* Check if the directory exists: */
	if(entry.first!=0&&entry.first->entries[entry.second].filePos==~Offset(0))
		{
		/* Return the found directory: */
		return new ZipArchiveDirectory(this,entry.first->entries[entry.second].child);
		}
	else if(entry.first==0&&entry.second==1)
		{
		/* Return the root directory: */
		return new ZipArchiveDirectory(this,&root);
		}
	else
		throw FileNotFoundError(directoryName);
	}

}
