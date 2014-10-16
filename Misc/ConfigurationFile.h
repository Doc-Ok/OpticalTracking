/***********************************************************************
ConfigurationFile - Class to handle permanent storage of configuration
data in human-readable text files.
Copyright (c) 2002-2013 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef MISC_CONFIGURATIONFILE_INCLUDED
#define MISC_CONFIGURATIONFILE_INCLUDED

#include <list>
#include <stdexcept>
#include <string>
#include <Misc/ValueCoder.h>

/* Forward declarations: */
namespace Misc {
class File;
class ConfigurationFileSection;
class ConfigurationFile;
}

namespace Misc {

class ConfigurationFileBase
	{
	friend class ConfigurationFileSection;
	friend class ConfigurationFile;
	
	/* Embedded classes: */
	public:
	class MalformedConfigFileError:public std::runtime_error // Exception class to report malformed configuration files
		{
		/* Constructors and destructors: */
		public:
		MalformedConfigFileError(const std::string& error,int lineNumber,const std::string& configFileName);
		};
	
	class SectionNotFoundError:public std::runtime_error // Exception class to report missing sections
		{
		/* Constructors and destructors: */
		public:
		SectionNotFoundError(const std::string& absoluteSectionPath);
		};
	
	class TagNotFoundError:public std::runtime_error // Exception class to report missing tags
		{
		/* Constructors and destructors: */
		public:
		TagNotFoundError(const std::string& tagName,const std::string& absoluteSectionPath);
		};
	
	protected:
	class Section // Class representing a section of configuration data
		{
		/* Embedded classes: */
		public:
		class TagValue // Class representing tag/value pairs
			{
			/* Elements: */
			public:
			std::string tag;
			std::string value; // Value encoded as std::string

			/* Constructors and destructors: */
			TagValue(const std::string& sTag,const std::string& sValue) // Creates a std::string value
				:tag(sTag),value(sValue)
				{
				}
			};
		
		/* Elements: */
		Section* parent; // Pointer to parent section (null if root section)
		std::string name; // Section name
		Section* sibling; // Pointer to next section under common parent
		Section* firstSubsection; // Pointer to first subsection
		Section* lastSubsection; // Pointer to last subsection
		std::list<TagValue> values; // List of values in this section
		bool edited; // Flag if the section has been changed since the last save
		
		/* Constructors and destructors: */
		Section(Section* sParent,const std::string& sName); // Creates an empty section
		template <class PipeParam>
		Section(Section* sParent,PipeParam& pipe); // Reads a section and its subsections from a pipe
		~Section(void);
		
		/* Methods: */
		void clear(void); // Removes all subsections and tag/value pairs from the section
		Section* addSubsection(const std::string& subsectionName); // Adds a subsection to a section
		void removeSubsection(const std::string& subsectionName); // Removes the given subsection from the section; does nothing if subsection does not exist
		void addTagValue(const std::string& newTag,const std::string& newValue); // Adds a new tag/value pair to the section
		void removeTag(const std::string& tag); // Removes the given tag from the section; does nothing if tag does not exist
		bool isEdited(void) const; // Checks if this section (or any of its subsections) has been edited since the last save
		void clearEditFlag(void); // Clears edit flag for this section and all of its subsections
		void save(File& file,int sectionLevel); // Writes all subsections and tag/value pairs to a file
		template <class PipeParam>
		void writeToPipe(PipeParam& pipe) const; // Writes a section and its subsections to a pipe
		
		/* Section navigation methods: */
		std::string getPath(void) const; // Returns absolute path to this section
		const Section* getSection(const char* relativePath,const char** pathSuffix =0) const; // Returns section reached from this section following given path; throws exception if section does not exist
		Section* getSection(const char* relativePath,const char** pathSuffix =0); // Returns section reached from this section following given path; tries its best to create sections that do not exist
		
		/* Tag value retrieval methods: */
		bool hasTag(const char* relativeTagPath) const; // Returns true if the relative tag path exists
		const std::string* findTagValue(const char* relativeTagPath) const; // Retrieves pointer to value of relative tag path; returns null pointer if tag does not exist
		const std::string& retrieveTagValue(const char* relativeTagPath) const; // Retrieves value of relative tag path; throws exception if tag does not exist
		std::string retrieveTagValue(const char* relativeTagPath,const std::string& defaultValue) const; // Retrieves value of relative tag path; returns default value if tag does not already exist
		const std::string& retrieveTagValue(const char* relativeTagPath,const std::string& defaultValue); // Retrieves value of relative tag path; tries its best to create tag if it does not already exist
		void storeTagValue(const char* relativeTagPath,const std::string& newValue); // Stores the value under the relative tag path; tries its best to create tag if it does not already exist
		};
	
	public:
	class SectionValueCoder // Base class giving type-level access to tag values of a specified section
		{
		/* Elements: */
		protected:
		Section* baseSection; // Pointer to section used for relative path name resolution
		
		/* Constructors and destructors: */
		SectionValueCoder(Section* sBaseSection)
			:baseSection(sBaseSection)
			{
			}
		
		/* Methods: */
		public:
		
		/* Subsection iteration methods: */
		bool isValid(void) const // Returns false if the section does not exist
			{
			return baseSection!=0;
			}
		std::string getName(void) const // Returns name of section under its parent section
			{
			return baseSection->name;
			}
		friend bool operator==(const SectionValueCoder& sIt1,const SectionValueCoder& sIt2) // Equality operator
			{
			return sIt1.baseSection==sIt2.baseSection;
			}
		friend bool operator!=(const SectionValueCoder& sIt1,const SectionValueCoder& sIt2) // Inequality operator
			{
			return sIt1.baseSection!=sIt2.baseSection;
			}
		SectionValueCoder& operator++(void) // Moves to next subsection under same parent section (pre-increment)
			{
			baseSection=baseSection->sibling;
			return *this;
			}
		SectionValueCoder operator++(int) // Ditto (post-increment)
			{
			SectionValueCoder result=*this;
			baseSection=baseSection->sibling;
			return result;
			}
		SectionValueCoder beginSubsections(void) const // Returns pointer to first subsection
			{
			return SectionValueCoder(baseSection->firstSubsection);
			}
		SectionValueCoder endSubsections(void) const // Returns iterator one past last subsection
			{
			return SectionValueCoder(0);
			}
		
		/* String access methods: */
		bool hasTag(const char* tag) const // Returns true if the given tag exists
			{
			return baseSection->hasTag(tag);
			}
		const std::string& retrieveString(const char* tag) const // Retrieves string value; throws exception if tag does not exist
			{
			return baseSection->retrieveTagValue(tag);
			}
		std::string retrieveString(const char* tag,const std::string& defaultValue) const // Retrieves a string value; returns default if tag does not exist
			{
			return baseSection->retrieveTagValue(tag,defaultValue);
			}
		const std::string& retrieveString(const char* tag,const std::string& defaultValue) // Retrieves a string value; returns default (and adds tag) if tag does not exist
			{
			return baseSection->retrieveTagValue(tag,defaultValue);
			}
		void storeString(const char* tag,const std::string& newValue) // Stores a string value; adds tag if tag does not exist
			{
			baseSection->storeTagValue(tag,newValue);
			}
		
		/* Typed access methods with specified value coder class: */
		template <class ValueParam,class ValueCoderParam>
		ValueParam retrieveValueWC(const char* tag,ValueCoderParam& coder) const // Retrieves a value; throws exception if tag does not exist
			{
			std::string value=baseSection->retrieveTagValue(tag);
			return coder.decode(value.data(),value.data()+value.size());
			}
		template <class ValueParam,class ValueCoderParam>
		ValueParam retrieveValueWC(const char* tag,const ValueParam& defaultValue,ValueCoderParam& coder) const // Retrieves a value; returns default if tag does not exist
			{
			const std::string* value=baseSection->findTagValue(tag);
			return value!=0?coder.decode(value->data(),value->data()+value->size()):defaultValue;
			}
		template <class ValueParam,class ValueCoderParam>
		ValueParam retrieveValueWC(const char* tag,const ValueParam& defaultValue,ValueCoderParam& coder) // Retrieves a value; returns default (and adds tag) if tag does not exist
			{
			const std::string* value=baseSection->findTagValue(tag);
			if(value!=0)
				return coder.decode(value->data(),value->data()+value->size());
			else
				{
				baseSection->storeTagValue(tag,coder.encode(defaultValue));
				return defaultValue;
				}
			}
		template <class ValueParam,class ValueCoderParam>
		void storeValueWC(const char* tag,const ValueParam& newValue,const ValueCoderParam& coder) // Stores a value; adds tag if tag does not exist
			{
			baseSection->storeTagValue(tag,coder.encode(newValue));
			}
		
		/*******************************************************************
		Note: For some reason, overloading of template methods does not work
		in g++ version 3.2.0.
		*******************************************************************/
		
		#if !defined(__GNUC__) || (__GNUC__ != 3 || __GNUC_MINOR__ != 2 || __GNUC_PATCHLEVEL__ != 0)
		/* Typed access methods with default value coder class: */
		template <class ValueParam>
		ValueParam retrieveValue(const char* tag) const // Ditto
			{
			std::string value=baseSection->retrieveTagValue(tag);
			return ValueCoder<ValueParam>::decode(value.data(),value.data()+value.size());
			}
		template <class ValueParam>
		ValueParam retrieveValue(const char* tag,const ValueParam& defaultValue) const // Ditto
			{
			const std::string* value=baseSection->findTagValue(tag);
			return value!=0?ValueCoder<ValueParam>::decode(value->data(),value->data()+value->size()):defaultValue;
			}
		template <class ValueParam>
		ValueParam retrieveValue(const char* tag,const ValueParam& defaultValue) // Ditto
			{
			const std::string* value=baseSection->findTagValue(tag);
			if(value!=0)
				return ValueCoder<ValueParam>::decode(value->data(),value->data()+value->size());
			else
				{
				baseSection->storeTagValue(tag,ValueCoder<ValueParam>::encode(defaultValue));
				return defaultValue;
				}
			}
		template <class ValueParam>
		void storeValue(const char* tag,const ValueParam& newValue) // Ditto
			{
			baseSection->storeTagValue(tag,ValueCoder<ValueParam>::encode(newValue));
			}
		#endif
		};
	
	class SectionIterator // Class for navigating a configuration file's section hierarchy
		{
		friend class ConfigurationFileBase;
		friend class ConfigurationFileSection;
		
		/* Elements: */
		private:
		Section* baseSection; // Base section for resolving relative tag names
		
		/* Constructors and destructors: */
		public:
		SectionIterator(void) // Constructs illegal section iterator
			:baseSection(0)
			{
			}
		private:
		SectionIterator(Section* sBaseSection) // Constructs iterator pointing to given section
			:baseSection(sBaseSection)
			{
			}
		
		/* Methods: */
		public:
		friend bool operator==(const SectionIterator& sIt1,const SectionIterator& sIt2) // Equality operator
			{
			return sIt1.baseSection==sIt2.baseSection;
			}
		friend bool operator!=(const SectionIterator& sIt1,const SectionIterator& sIt2) // Inequality operator
			{
			return sIt1.baseSection!=sIt2.baseSection;
			}
		std::string getName(void) const // Returns name of base section
			{
			return baseSection->name;
			}
		Section& operator*(void) const // Returns base section
			{
			return *baseSection;
			}
		Section* operator->(void) const // Returns pointer to base section for dereferencing
			{
			return baseSection;
			}
		SectionIterator& operator++(void) // Moves to next subsection under same parent section (pre-increment)
			{
			baseSection=baseSection->sibling;
			return *this;
			}
		SectionIterator operator++(int) // Ditto (post-increment)
			{
			SectionIterator result=*this;
			baseSection=baseSection->sibling;
			return result;
			}
		SectionIterator beginSubsections(void) const // Returns iterator to first subsection
			{
			return SectionIterator(baseSection->firstSubsection);
			}
		SectionIterator endSubsections(void) const // Returns iterator one past last subsection
			{
			return SectionIterator(0);
			}
		SectionIterator getSection(const char* relativePath) const // Returns iterator to section indicated by relative path; does not create subsections
			{
			return SectionIterator(baseSection->getSection(relativePath));
			}
		};
	
	/* Elements: */
	protected:
	std::string fileName; // File name of configuration file
	Section* rootSection; // Pointer to root section of configuration file
	
	/* Constructors and destructors: */
	public:
	ConfigurationFileBase(void); // Creates an empty unnamed configuration file
	ConfigurationFileBase(const char* sFileName); // Opens an existing configuration file
	template <class PipeParam>
	ConfigurationFileBase(PipeParam& pipe); // Reads a configuration file from a pipe
	private:
	ConfigurationFileBase(const ConfigurationFile& source); // Prohibit copy constructor
	ConfigurationFileBase& operator=(const ConfigurationFile& source); // Prohibit assignment operator
	public:
	~ConfigurationFileBase(void);
	
	/* Methods: */
	void load(const char* newFileName); // Loads contents of given configuration file
	void reload(void) // Reloads contents of original configuration file
		{
		load(fileName.c_str());
		}
	void merge(const char* mergeFileName); // Merges in contents of given configuration file
	void mergeCommandline(int& argc,char**& argv); // Merges and removes "-tag value" pairs given on command line
	void saveAs(const char* newFileName); // Saves the current in-memory state of the configuration file to the given file name
	void save(void) // Saves the current in-memory state of the configuration file
		{
		/* Check if the configuration was edited: */
		if(rootSection->isEdited())
			saveAs(fileName.c_str());
		}
	template <class PipeParam>
	void readFromPipe(PipeParam& pipe); // Reads a configuration file from a pipe
	template <class PipeParam>
	void writeToPipe(PipeParam& pipe) const; // Writes the in-memory representation of the configuration file to a pipe
	
	/* Section iterator management methods: */
	SectionIterator getRootSection(void) // Returns iterator to root section
		{
		return SectionIterator(rootSection);
		}
	};

class ConfigurationFileSection:public ConfigurationFileBase::SectionValueCoder
	{
	friend class ConfigurationFile;
	
	/* Constructors and destructors: */
	public:
	ConfigurationFileSection(void) // Creates invalid section pointer
		:ConfigurationFileBase::SectionValueCoder(0)
		{
		}
	private:
	ConfigurationFileSection(ConfigurationFileBase::Section* sBaseSection) // Creates pointer to given section
		:ConfigurationFileBase::SectionValueCoder(sBaseSection)
		{
		}
	public:
	ConfigurationFileSection(const ConfigurationFileBase::SectionValueCoder& source) // Creates from base class
		:ConfigurationFileBase::SectionValueCoder(source)
		{
		}
	
	/* Methods: */
	bool isValid(void) const // Returns true if the section pointer is valid
		{
		return baseSection!=0;
		}
	std::string getPath(void) const; // Returns absolute path to current section
	void setSection(const char* relativePath); // Changes current section
	ConfigurationFileSection getSection(const char* relativePath) const; // Returns pointer to given section relative to current section
	void clear(void); // Removes all subsections and tag/value pairs from the current section
	void removeSubsection(const std::string& subsectionName); // Removes the given subsection from the current section
	void removeTag(const std::string& tagName); // Removes the given tag from the current section
	};

class ConfigurationFile:public ConfigurationFileBase,public ConfigurationFileBase::SectionValueCoder
	{
	/* Constructors and destructors: */
	public:
	ConfigurationFile(void) // Creates empty unnamed configuration file
		:ConfigurationFileBase::SectionValueCoder(rootSection)
		{
		}
	ConfigurationFile(const char* sFileName) // Reads a configuration file from the given file
		:ConfigurationFileBase(sFileName),
		 ConfigurationFileBase::SectionValueCoder(rootSection)
		{
		}
	template <class PipeParam>
	ConfigurationFile(PipeParam& pipe) // Reads a configuration file from a pipe
		:ConfigurationFileBase(pipe),
		 ConfigurationFileBase::SectionValueCoder(rootSection)
		{
		}
	
	/* Overloaded methods from ConfigurationFileBase: */
	void load(const char* newFileName); // Loads contents of given configuration file, and resets current section to new root section
	void reload(void) // Reloads contents of original configuration file, and resets current section to root section
		{
		load(fileName.c_str());
		}
	
	/* New methods: */
	std::string getCurrentPath(void) const; // Returns absolute path to current section
	void setCurrentSection(const char* relativePath); // Sets the current section to the given relative path
	ConfigurationFileSection getCurrentSection(void) const; // Returns pointer to current section
	ConfigurationFileSection getSection(const char* relativePath); // Returns pointer to the given relative path
	void list(void) const; // Lists all subsections and tags in current section
	};

}

#ifndef MISC_CONFIGURATIONFILE_IMPLEMENTATION
#include <Misc/ConfigurationFile.icpp>
#endif

#endif
