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

#ifndef IO_ZIPARCHIVE_INCLUDED
#define IO_ZIPARCHIVE_INCLUDED

#include <string.h>
#include <utility>
#include <vector>
#include <stdexcept>
#include <Misc/RefCounted.h>
#include <IO/File.h>
#include <IO/SeekableFile.h>
#include <IO/Directory.h>

/* Forward declarations: */
namespace IO {
class ZipArchiveDirectory;
}

namespace IO {

class ZipArchive:public Misc::RefCounted
	{
	/* Embedded classes: */
	public:
	typedef IO::SeekableFile::Offset Offset; // Type for file positions
	
	class FileNotFoundError:public std::runtime_error // Class to signal errors in findFile method
		{
		/* Elements: */
		public:
		char* fileName; // Name of file that was not found
		
		/* Constructors and destructors: */
		public:
		FileNotFoundError(const char* sFileName);
		virtual ~FileNotFoundError(void) throw();
		};
	
	class FileID;
	
	private:
	struct Directory // Structure to represent interior nodes in the ZIP archive's directory tree
		{
		friend class ZipArchiveDirectory;
		
		/* Embedded classes: */
		public:
		struct Entry // Structure to represent directory entries (files or subdirectories)
			{
			/* Elements: */
			public:
			char* name; // Pointer to the entry's name
			Offset filePos; // Starting position of local file header inside ZIP archive; ~0x0 identifies a directory
			union
				{
				struct
					{
					size_t compressed,uncompressed; // Size of file; 0 for a directory
					} sizes;
				Directory* child; // Pointer to structure representing subdirectory
				};
			
			/* Methods: */
			static bool compare(const Entry& e1,const Entry& e2)
				{
				return strcmp(e1.name,e2.name)<0;
				}
			};
		
		/* Elements: */
		Directory* parent; // Pointer to parent directory; NULL for ZIP archive's root directory
		unsigned int parentIndex; // Index of this directory in the parent's entry array
		std::vector<Entry> entries; // List of directory entries; sorted after directory creation is complete
		
		/* Constructors and destructors: */
		Directory(Directory* sParent); // Creates an empty directory node with the given parent directory
		~Directory(void); // Destroys the directory and its subdirectories

		/* Methods: */
		bool addPath(const char* path,const FileID& fileId); // Adds the file or directory of the given directory-relative path to this directory; returns true if path was added successfully
		void finalize(void); // Finalizes this directory and all its subdirectories by sorting entries by name and fixing subdirectory back-pointers
		void getPath(std::string& path,size_t suffixLen) const; // Returns the absolute path name of this directory terminated with a '/'; reserves enough space to append a suffix of the given length
		std::pair<Directory*,unsigned int> findPath(const char* path); // Returns a pointer to a directory and an index into that directory's entry array corresponding to the given relative path; returns (0, 0) if path does not exist
		};
	
	public:
	class FileID // Class to identify files in a ZIP archive
		{
		friend class ZipArchiveDirectory;
		friend class ZipArchive;
		friend struct Directory;
		
		/* Elements: */
		private:
		Offset filePos; // Position of file inside archive
		size_t compressedSize; // Compressed file size
		size_t uncompressedSize; // Uncompressed file size
		
		/* Constructors and destructors: */
		public:
		FileID(void) // Creates invalid file ID
			:filePos(~Offset(0)),
			 compressedSize(0),uncompressedSize(0)
			{
			}
		
		/* Methods: */
		bool isValid(void) const // Returns true if the identifier points to an existing file
			{
			return filePos!=~Offset(0);
			}
		size_t getCompressedFileSize(void) const // Returns compressed file size
			{
			return compressedSize;
			}
		size_t getFileSize(void) const // Returns uncompressed file size
			{
			return uncompressedSize;
			}
		};
	
	class DirectoryIterator:public FileID // Class to traverse a ZIP archive's directory tree
		{
		friend class ZipArchive;
		
		/* Elements: */
		private:
		const Directory* directory; // Pointer to current directory
		unsigned int entryIndex; // Index of current directory entry
		size_t fileNameBufferSize; // Allocated size for file name
		char* fileName; // File name of current directory entry
		size_t pathEnd; // First character of entry component of file name
		
		/* Private methods: */
		void getEntry(void); // Retrieves the name and file ID of the currently referenced directory entry
		
		/* Constructors and destructors: */
		public:
		DirectoryIterator(void) // Creates an invalid directory entry
			:directory(0),entryIndex(0),
			 fileNameBufferSize(0),fileName(0),pathEnd(0)
			{
			};
		DirectoryIterator(const ZipArchive& archive); // Creates an iterator for the root directory of the given ZIP archive
		DirectoryIterator(const DirectoryIterator& source);
		DirectoryIterator& operator=(const DirectoryIterator& source);
		~DirectoryIterator(void)
			{
			delete[] fileName;
			}
		
		/* Methods: */
		public:
		bool isValid(void) const // Returns true if the iterator points to a valid directory entry
			{
			return directory!=0;
			}
		const char* getFileName(void) const // Returns file name as NUL-terminated string
			{
			return fileName;
			};
		bool isDirectory(void) const // Returns true if the current directory entry is a subdirectory
			{
			return filePos==~Offset(0);
			}
		DirectoryIterator& operator++(void); // Increments the directory iterator to the next directory entry in depth-first order
		};
	
	friend class DirectoryIterator;
	friend class ZipArchiveDirectory;
	
	/* Elements: */
	private:
	SeekableFilePtr archive; // File object to access the ZIP archive
	Directory root; // The ZIP archive's root directory
	
	/* Private methods: */
	int initArchive(void); // Initializes the ZIP archive file structures; returns error code
	
	/* Constructors and destructors: */
	public:
	ZipArchive(const char* archiveFileName); // Opens a ZIP archive of the given file name using a standard file abstraction
	ZipArchive(SeekableFilePtr sArchive); // Reads a ZIP archive from an already-opened file
	~ZipArchive(void); // Closes the ZIP archive
	
	/* Methods: */
	FileID findFile(const char* fileName) const; // Returns a file identifier for a file of the given name; throws exception if file does not exist
	FilePtr openFile(const FileID& fileId); // Returns a file for streaming reading
	SeekableFilePtr openSeekableFile(const FileID& fileId); // Returns a file for seekable reading
	DirectoryPtr openRootDirectory(void); // Returns a directory object representing the root directory
	DirectoryPtr openDirectory(const char* directoryName); // Returns a directory object representing the given directory name
	};

typedef Misc::Autopointer<ZipArchive> ZipArchivePtr; // Type for reference-counted pointers to zip archives

}

#endif
