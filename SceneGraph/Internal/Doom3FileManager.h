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

#ifndef SCENEGRAPH_INTERNAL_DOOM3FILEMANAGER_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3FILEMANAGER_INCLUDED

#include <string>
#include <vector>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>
#include <IO/SeekableFile.h>
#include <IO/Directory.h>
#include <IO/ZipArchive.h>
#include <SceneGraph/Internal/Doom3NameTree.h>

/* Forward declarations: */
namespace SceneGraph {
class Doom3FileManagerDirectory;
}

namespace SceneGraph {

class Doom3FileManager
	{
	friend class Doom3FileManagerDirectory;
	
	/* Embedded classes: */
	private:
	typedef IO::ZipArchive PakFile; // Doom 3 pak files are just ZIP archives in disguise
	
	struct PakFileHandle // Structure containing data necessary to read a file from a pak archive
		{
		/* Elements: */
		PakFile* pakFile; // The pak archive containing the file
		PakFile::FileID fileID; // Handle to access the file inside the pak archive
		
		/* Constructors and destructors: */
		PakFileHandle(PakFile* sPakFile,const PakFile::FileID& sFileID)
			:pakFile(sPakFile),fileID(sFileID)
			{
			};
		};
	
	typedef Doom3NameTree<PakFileHandle> PakFileTree; // Structure to store all files in a pak archive
	
	class DummyNameFilter // Class for name filters that always pass
		{
		/* Methods: */
		public:
		static bool filter(const std::string& fileName)
			{
			return true;
			};
		};
	
	class ExtensionFilter // Class to filter names by extension
		{
		/* Elements: */
		private:
		std::string extension;
		
		/* Constructors and destructors: */
		public:
		ExtensionFilter(const char* sExtension)
			:extension(sExtension)
			{
			};
		
		/* Methods: */
		bool filter(const std::string& fileName) const
			{
			/* Check the file extension against the filter: */
			const char* extPtr=0;
			for(const char* fnPtr=fileName.c_str();*fnPtr!='\0';++fnPtr)
				if(*fnPtr=='.')
					extPtr=fnPtr;
			return extPtr!=0&&extension==extPtr+1;
			};
		};
	
	template <class ClientFunctorParam,class NameFilterParam>
	class DirectorySearcher // Helper class to allow clients to search the directory tree
		{
		/* Elements: */
		private:
		char pathName[2048]; // Full pathname of the current file
		char* pathEndPtr; // Pointer after the end of the current directory prefix
		ClientFunctorParam& clientFunctor;
		const NameFilterParam& nameFilter;
		
		/* Constructors and destructors: */
		public:
		DirectorySearcher(ClientFunctorParam& sClientFunctor,const NameFilterParam& sNameFilter)
			:pathEndPtr(pathName),clientFunctor(sClientFunctor),nameFilter(sNameFilter)
			{
			};
		
		/* Methods: */
		void enterInteriorNode(std::string name)
			{
			/* Add the directory name to the end of the current path: */
			memcpy(pathEndPtr,name.c_str(),name.length());
			pathEndPtr+=name.length();
			*pathEndPtr='/';
			++pathEndPtr;
			};
		void leaveInteriorNode(std::string name)
			{
			/* Remove the directory name from the end of the current path: */
			pathEndPtr-=name.length()+1;
			};
		void operator()(std::string name,const PakFileHandle& pfh)
			{
			if(nameFilter.filter(name))
				{
				/* Append the file name to the end of the current path: */
				memcpy(pathEndPtr,name.c_str(),name.length()+1); // Includes the NUL terminator!
				
				/* Call the client functor: */
				clientFunctor(pathName);
				}
			};
		};
	
	public:
	class ReadError:public std::runtime_error // Exception class to report file reading errors
		{
		/* Constructors and destructors: */
		public:
		ReadError(const char* fileName)
			:std::runtime_error(Misc::printStdErrMsg("Doom3FileManager::readFile: File %s not found",fileName))
			{
			}
		};
	
	/* Elements: */
	private:
	std::vector<PakFile*> pakFiles; // The list of pk3/pk4 files
	PakFileTree pakFileTree; // The tree containing the pak archive's files
	
	/* Constructors and destructors: */
	public:
	Doom3FileManager(void); // Creates an empty file manager
	Doom3FileManager(IO::DirectoryPtr baseDirectory,const char* pakFilePrefix); // Creates a file manager by loading all pk3/pk4 files that match the given prefix in the given directory
	~Doom3FileManager(void); // Destroys the file manager
	
	/* Methods: */
	void addPakFile(IO::FilePtr pakFile); // Adds a pk3/pk4 file to the file manager
	void addPakFiles(IO::DirectoryPtr baseDirectory,const char* pakFilePrefix); // Adds all pk3/pk4 files that match the given prefix from the given base directory
	template <class ClientFunctorParam>
	void searchFileTree(ClientFunctorParam& cf) const // Searches the entire file tree and calls the client functor for each file
		{
		DummyNameFilter dnf;
		DirectorySearcher<ClientFunctorParam,DummyNameFilter> ds(cf,dnf);
		pakFileTree.traverseTree(ds);
		}
	template <class ClientFunctorParam>
	void searchFileTree(ClientFunctorParam& cf,const char* extension) const // Searches the entire file tree and calls the client functor for each file that matches the given extension (case-sensitive)
		{
		ExtensionFilter ef(extension);
		DirectorySearcher<ClientFunctorParam,ExtensionFilter> ds(cf,ef);
		pakFileTree.traverseTree(ds);
		}
	template <class ClientFunctorParam,class NameFilterParam>
	void searchFileTree(ClientFunctorParam& cf,const NameFilterParam& nf) const // Searches the entire file tree and calls the client functor for each file that matches the name filter
		{
		DirectorySearcher<ClientFunctorParam,NameFilterParam> ds(cf,nf);
		pakFileTree.traverseTree(ds);
		}
	IO::FilePtr getFile(const char* fileName); // Returns a file as a streaming reader; throws ReadError if file not found
	IO::SeekableFilePtr getSeekableFile(const char* fileName); // Returns a file as a seekable reader; throws ReadError if file not found
	IO::DirectoryPtr getDirectory(const char* directoryName); // Returns a directory object to traverse the file manager's directory tree
	};

}

#endif
