/***********************************************************************
Doom3FileManager - Class to read files from sets of pk3/pk4 files and
patch directories.
Copyright (c) 2007-2014 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <SceneGraph/Internal/Doom3FileManager.h>

#include <ctype.h>
#include <string.h>
#include <utility>
#include <algorithm>
#include <Misc/ThrowStdErr.h>
#include <IO/SeekableFilter.h>

namespace SceneGraph {

namespace {

/**************
Helper classes:
**************/

typedef std::pair<std::string,unsigned int> PakFileName; // Type for pak file names and indices

class PakFileCompare // Class to compare pak file names according to numerical order
	{
	/* Methods: */
	public:
	bool operator()(const PakFileName& pfn1,const PakFileName& pfn2) const
		{
		return pfn1.second<pfn2.second;
		}
	};

}

class Doom3FileManagerDirectory:public IO::Directory // Class to traverse the directory structure of a file manager
	{
	/* Elements: */
	private:
	Doom3FileManager& fileManager; // Reference to the file manager that created this directory
	std::string pathName; // Absolute path name of this directory
	Doom3FileManager::PakFileTree::NodeIterator directoryIt; // Iterator to traverse this directory
	
	/* Constructors and destructors: */
	public:
	Doom3FileManagerDirectory(Doom3FileManager& sFileManager,const std::string& sPathName);
	
	/* Methods from IO::Directory: */
	virtual std::string getName(void) const;
	virtual std::string getPath(void) const;
	virtual std::string getPath(const char* relativePath) const;
	virtual bool hasParent(void) const;
	virtual IO::DirectoryPtr getParent(void) const;
	virtual void rewind(void);
	virtual bool readNextEntry(void);
	virtual const char* getEntryName(void) const;
	virtual Misc::PathType getEntryType(void) const;
	virtual Misc::PathType getPathType(const char* relativePath) const;
	virtual IO::FilePtr openFile(const char* fileName,IO::File::AccessMode accessMode =IO::File::ReadOnly) const;
	virtual IO::DirectoryPtr openDirectory(const char* directoryName) const;
	};

/******************************************
Methods of class Doom3FileManagerDirectory:
******************************************/

Doom3FileManagerDirectory::Doom3FileManagerDirectory(Doom3FileManager& sFileManager,const std::string& sPathName)
	:fileManager(sFileManager)
	{
	/* Prepend an initial slash to the path name if there is none: */
	if(sPathName.empty()||sPathName[0]!='/')
		pathName.push_back('/');
	pathName.append(sPathName);
	
	/* Normalize the path name: */
	normalizePath(pathName,1);
	
	/* Get an iterator to the directory: */
	directoryIt=fileManager.pakFileTree.findInteriorNode(pathName.c_str()+1); // Skip initial slash
	if(!directoryIt.isValid())
		throw OpenError(pathName.c_str());
	}

std::string Doom3FileManagerDirectory::getName(void) const
	{
	return std::string(getLastComponent(pathName,1),pathName.end());
	}

std::string Doom3FileManagerDirectory::getPath(void) const
	{
	return pathName;
	}

std::string Doom3FileManagerDirectory::getPath(const char* relativePath) const
	{
	/* Assemble and normalize the absolute path name: */
	std::string result=pathName;
	if(result.length()>1)
		result.push_back('/');
	result.append(relativePath);
	normalizePath(result,1);
	
	return result;
	}

bool Doom3FileManagerDirectory::hasParent(void) const
	{
	return pathName.length()>1;
	}

IO::DirectoryPtr Doom3FileManagerDirectory::getParent(void) const
	{
	/* Check for the special case of the root directory: */
	if(pathName.length()==1)
		return 0;
	else
		{
		/* Find the last component in the absolute path name: */
		std::string::const_iterator lastCompIt=getLastComponent(pathName,1);
		
		/* Strip off the last slash unless it's the prefix: */
		if(lastCompIt-pathName.begin()>1)
			--lastCompIt;
		
		/* Open and return the directory corresponding to the path name prefix before the last slash: */
		return new Doom3FileManagerDirectory(fileManager,std::string(pathName.begin(),lastCompIt));
		}
	}

void Doom3FileManagerDirectory::rewind(void)
	{
	directoryIt.rewind();
	}

bool Doom3FileManagerDirectory::readNextEntry(void)
	{
	++directoryIt;
	return !directoryIt.eod();
	}

const char* Doom3FileManagerDirectory::getEntryName(void) const
	{
	return directoryIt.getName().c_str();
	}

Misc::PathType Doom3FileManagerDirectory::getEntryType(void) const
	{
	if(directoryIt.isInterior())
		return Misc::PATHTYPE_DIRECTORY;
	else if(directoryIt.isLeaf())
		return Misc::PATHTYPE_FILE;
	else
		return Misc::PATHTYPE_DOES_NOT_EXIST;
	}

Misc::PathType Doom3FileManagerDirectory::getPathType(const char* relativePath) const
	{
	/* Shunt for now: */
	throw std::runtime_error("Doom3FileManagerDirectory::getPathType: Not implemented");
	}

IO::FilePtr Doom3FileManagerDirectory::openFile(const char* fileName,IO::File::AccessMode accessMode) const
	{
	if(accessMode==IO::File::WriteOnly||accessMode==IO::File::ReadWrite)
		Misc::throwStdErr("Doom3FileManagerDirectory::openFile: Cannot write to file %s",fileName);
	
	/* Assemble the absolute path name of the given file: */
	std::string filePath=pathName;
	if(filePath.length()>1)
		filePath.push_back('/');
	filePath.append(fileName);
	normalizePath(filePath,1);
	
	/* Open and return the file: */
	return fileManager.getFile(filePath.c_str());
	}

IO::DirectoryPtr Doom3FileManagerDirectory::openDirectory(const char* directoryName) const
	{
	/* Assemble the absolute path name of the given directory: */
	std::string directoryPath=pathName;
	if(directoryPath.length()>1)
		directoryPath.push_back('/');
	directoryPath.append(directoryName);
	normalizePath(directoryPath,1);
	
	/* Open and return the directory: */
	return fileManager.getDirectory(directoryPath.c_str());
	}

/*********************************
Methods of class Doom3FileManager:
*********************************/

Doom3FileManager::Doom3FileManager(void)
	{
	}

Doom3FileManager::Doom3FileManager(IO::DirectoryPtr baseDirectory,const char* pakFilePrefix)
	{
	addPakFiles(baseDirectory,pakFilePrefix);
	}

Doom3FileManager::~Doom3FileManager(void)
	{
	/* Delete all pak files: */
	for(std::vector<PakFile*>::iterator pfIt=pakFiles.begin();pfIt!=pakFiles.end();++pfIt)
		delete (*pfIt);
	}

void Doom3FileManager::addPakFile(IO::FilePtr pakFile)
	{
	/* Check if the pak file is seekable: */
	IO::SeekableFilePtr seekablePakFile=pakFile;
	if(seekablePakFile==0)
		{
		/* Wrap the pak file in a seekable filter: */
		seekablePakFile=new IO::SeekableFilter(pakFile);
		}
	
	/* Open a new pak archive and add it to the list: */
	PakFile* pak=new PakFile(seekablePakFile);
	pakFiles.push_back(pak);
	
	/* Read the pak archive's directory and add all files to the pak file tree: */
	for(PakFile::DirectoryIterator dIt(*pak);dIt.isValid();++dIt)
		{
		/* Check if the current entry is a file: */
		if(!dIt.isDirectory())
			{
			/* Insert the file into the directory structure: */
			pakFileTree.insertLeaf(dIt.getFileName(),PakFileHandle(pak,dIt));
			}
		}
	}

void Doom3FileManager::addPakFiles(IO::DirectoryPtr baseDirectory,const char* pakFilePrefix)
	{
	/* Find all <pakFilePrefix>???.pk[0-9] files in the base directory: */
	size_t prefixLen=strlen(pakFilePrefix);
	std::vector<PakFileName> pakFileNames;
	baseDirectory->rewind();
	while(baseDirectory->readNextEntry())
		{
		/* Check if the entry name matches the pattern: */
		const char* name=baseDirectory->getEntryName();
		bool match=strncasecmp(name,pakFilePrefix,prefixLen)==0;
		unsigned int index=0;
		const char* extPtr;
		for(extPtr=name+prefixLen;match&&*extPtr!='\0'&&*extPtr!='.';++extPtr)
			{
			if(*extPtr>='0'&&*extPtr<='9')
				index=index*10U+(unsigned int)(*extPtr-'0');
			else
				match=false;
			}
		match=match&&strncasecmp(extPtr,".pk",3)==0;
		match=match&&isdigit(extPtr[3]);
		
		if(match)
			{
			/* Store the entry name: */
			pakFileNames.push_back(PakFileName(name,index));
			}
		}
	
	/* Sort the pak file list: */
	PakFileCompare pfc;
	std::sort(pakFileNames.begin(),pakFileNames.end(),pfc);
	
	/* Add all pak files in alphabetical (numerical) order: */
	for(std::vector<PakFileName>::iterator pfnIt=pakFileNames.begin();pfnIt!=pakFileNames.end();++pfnIt)
		addPakFile(baseDirectory->openFile(pfnIt->first.c_str()));
	}

IO::FilePtr Doom3FileManager::getFile(const char* fileName)
	{
	/* Search the file in the pak file tree: */
	PakFileTree::LeafID leafId=pakFileTree.findLeaf(fileName);
	if(!leafId.isValid())
		throw ReadError(fileName);
	
	/* Get the file's pak file handle: */
	const PakFileHandle& pfh=pakFileTree.getLeafValue(leafId);
	
	/* Read and return the file: */
	return pfh.pakFile->openFile(pfh.fileID);
	}

IO::SeekableFilePtr Doom3FileManager::getSeekableFile(const char* fileName)
	{
	/* Search the file in the pak file tree: */
	PakFileTree::LeafID leafId=pakFileTree.findLeaf(fileName);
	if(!leafId.isValid())
		throw ReadError(fileName);
	
	/* Get the file's pak file handle: */
	const PakFileHandle& pfh=pakFileTree.getLeafValue(leafId);
	
	/* Read and return the file: */
	return pfh.pakFile->openSeekableFile(pfh.fileID);
	}

IO::DirectoryPtr Doom3FileManager::getDirectory(const char* directoryName)
	{
	return new Doom3FileManagerDirectory(*this,directoryName);
	}

}
