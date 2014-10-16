/***********************************************************************
PlyFileStructures - Data structures to read 3D polygon files in PLY
format.
Copyright (c) 2004-2011 Oliver Kreylos
***********************************************************************/

#ifndef PLYFILESTRUCTURES_INCLUDED
#define PLYFILESTRUCTURES_INCLUDED

#include <vector>
#include <IO/File.h>

/* Forward declarations: */
namespace IO {
class ValueSource;
}

/*********************************************
Enumerated type for basic PLY file data types:
*********************************************/

enum PLYDataType
	{
	PLY_SINT8,PLY_UINT8,PLY_SINT16,PLY_UINT16,PLY_SINT32,PLY_UINT32,PLY_FLOAT32,PLY_FLOAT64
	};

/**********************************************
Base class for data values read from PLY files:
**********************************************/

class PLYDataValue
	{
	/* Constructors and destructors: */
	public:
	virtual ~PLYDataValue(void)
		{
		}
	
	/* Methods: */
	virtual PLYDataValue* clone(void) const =0;
	virtual size_t getFileSize(void) const =0;
	virtual size_t getMemorySize(void) const =0;
	virtual void read(IO::File& binaryPlyFile) =0;
	virtual void read(IO::ValueSource& asciiPlyFile) =0;
	virtual int getInt(void) const =0;
	virtual unsigned int getUnsignedInt(void) const =0;
	virtual double getDouble(void) const =0;
	};

/*****************************
Class for PLY file properties:
*****************************/

class PLYProperty
	{
	/* Embedded classes: */
	public:
	enum PropertyType
		{
		SCALAR,LIST
		};
	class Value
		{
		/* Elements: */
		private:
		PropertyType propertyType; // Type of the value's property
		PLYDataValue* scalar; // Pointer to scalar value for scalar properties
		PLYDataValue* listSize; // List size value for list properties
		std::vector<PLYDataValue*> listElements; // Vector of pointers to list elements for list properties
		
		/* Constructors and destructors: */
		public:
		Value(const PLYProperty& property); // Creates empty value structure for the given property
		private:
		Value(const Value& source); // Prohibit copy constructor
		Value& operator=(const Value& source); // Prohibit assignment operator
		public:
		~Value(void);
		
		/* Methods: */
		size_t getFileSize(void) const // Returns value's size in binary files (returns minimal file size for list values)
			{
			if(propertyType==SCALAR)
				return scalar->getFileSize();
			else
				return listSize->getFileSize();
			}
		void skip(IO::ValueSource& plyFile); // Skips value from ASCII PLY file (never used)
		void skip(IO::File& plyFile); // Skips value from binary PLY file
		template <class PlyFileParam>
		void read(PlyFileParam& plyFile) // Reads value from binary or ASCII PLY file
			{
			if(propertyType==SCALAR)
				{
				/* Read scalar: */
				scalar->read(plyFile);
				}
			else
				{
				/* Read list size: */
				listSize->read(plyFile);
				unsigned int listSizeValue=listSize->getUnsignedInt();
				
				/* Ensure the list storage is long enough: */
				unsigned int currentListSize=listElements.size();
				while(currentListSize<listSizeValue)
					{
					listElements.push_back(listElements[0]->clone());
					++currentListSize;
					}
				
				/* Read all list elements: */
				for(unsigned int i=0;i<listSizeValue;++i)
					listElements[i]->read(plyFile);
				}
			}
		const PLYDataValue* getScalar(void) const
			{
			return scalar;
			}
		const PLYDataValue* getListSize(void) const
			{
			return listSize;
			}
		const PLYDataValue* getListElement(unsigned int listElementIndex) const
			{
			return listElements[listElementIndex];
			}
		};
	
	/* Elements: */
	private:
	PropertyType propertyType; // Type of this property (scalar/list)
	PLYDataType scalarType; // Data type for scalar properties
	PLYDataType listSizeType; // Data type for list sizes for list properties
	PLYDataType listElementType; // Data type for list elements for list properties
	std::string name; // Property name
	
	/* Private methods: */
	static PLYDataType parseDataType(const std::string& tag);
	
	/* Constructors and destructors: */
	public:
	PLYProperty(IO::ValueSource& plyFile);
	
	/* Methods: */
	PropertyType getPropertyType(void) const // Returns property's type
		{
		return propertyType;
		}
	PLYDataType getScalarType(void) const // Returns scalar property's scalar type
		{
		return scalarType;
		}
	PLYDataType getListSizeType(void) const // Returns list property's size type
		{
		return listSizeType;
		}
	PLYDataType getListElementType(void) const // Returns list property's element type
		{
		return listElementType;
		}
	std::string getName(void) const // Returns property's name
		{
		return name;
		}
	};

/***************************
Class for PLY file elements:
***************************/

class PLYElement
	{
	/* Embedded classes: */
	public:
	typedef std::vector<PLYProperty> PropertyList;
	class Value
		{
		/* Embedded classes: */
		public:
		typedef std::vector<PLYProperty::Value*> PropertyValueList;
		
		/* Elements: */
		private:
		PropertyValueList propertyValues; // Vector of values for the properties of this element
		
		/* Constructors and destructors: */
		public:
		Value(const PLYElement& element);
		private:
		Value(const Value& source); // Prohibit copy constructor
		Value& operator=(const Value& source); // Prohibit assignment operator
		public:
		~Value(void);
		
		/* Methods: */
		public:
		size_t getFileSize(void) const // Returns the total file size of all property values (returns minimal file size for any included list values)
			{
			size_t result=0;
			for(PropertyValueList::const_iterator pvIt=propertyValues.begin();pvIt!=propertyValues.end();++pvIt)
				result+=(*pvIt)->getFileSize();
			return result;
			}
		template <class PlyFileParam>
		void skip(PlyFileParam& plyFile) // Skips element value from binary or ASCII PLY file
			{
			for(PropertyValueList::iterator pvIt=propertyValues.begin();pvIt!=propertyValues.end();++pvIt)
				(*pvIt)->skip(plyFile);
			}
		template <class PlyFileParam>
		void read(PlyFileParam& plyFile) // Reads element value from binary or ASCII PLY file
			{
			for(PropertyValueList::iterator pvIt=propertyValues.begin();pvIt!=propertyValues.end();++pvIt)
				(*pvIt)->read(plyFile);
			}
		const PLYProperty::Value& getValue(unsigned int propertyIndex) const // Returns value of one of the element's properties
			{
			return *propertyValues[propertyIndex];
			}
		};
	
	/* Elements: */
	private:
	std::string name; // Name of this element
	size_t numValues; // Number of values of this element in the file
	PropertyList properties; // Vector of properties of this element
	
	/* Constructors and destructors: */
	public:
	PLYElement(std::string sName,size_t sNumValues)
		:name(sName),numValues(sNumValues)
		{
		}
	
	/* Methods: */
	bool isElement(const char* elementName) const // Returns true if the element's name matches the given string
		{
		return name==elementName;
		}
	size_t getNumValues(void) const
		{
		return numValues;
		}
	void addProperty(IO::ValueSource& plyFile)
		{
		properties.push_back(PLYProperty(plyFile));
		}
	bool hasListProperty(void) const // Returns true if the element has at least one list property
		{
		bool result=false;
		for(PropertyList::const_iterator pIt=properties.begin();!result&&pIt!=properties.end();++pIt,++result)
			result=pIt->getPropertyType()==PLYProperty::LIST;
		return result;
		}
	size_t getNumProperties(void) const
		{
		return properties.size();
		}
	PropertyList::const_iterator propertiesBegin(void) const
		{
		return properties.begin();
		}
	PropertyList::const_iterator propertiesEnd(void) const
		{
		return properties.end();
		}
	unsigned int getPropertyIndex(const char* propertyName) const
		{
		unsigned int result=0;
		for(PropertyList::const_iterator pIt=properties.begin();pIt!=properties.end();++pIt,++result)
			if(pIt->getName()==propertyName)
				break;
		return result;
		}
	};

/**************************************
Helper class to parse PLY file headers:
**************************************/

struct PLYFileHeader // Structure containing relevant information from a PLY file's header
	{
	/* Embedded classes: */
	public:
	enum FileType
		{
		Unknown,Ascii,Binary
		};
	
	/* Elements: */
	private:
	bool valid; // Flag if the file is a valid PLY file (as much as determined by parsing the header)
	FileType fileType; // ASCII or binary
	Misc::Endianness fileEndianness; // Endianness of binary PLY file
	std::vector<PLYElement> elements; // List of elements in the file, in the order in which they appear in the file
	
	/* Constructors and destructors: */
	public:
	PLYFileHeader(IO::File& plyFile);
	
	/* Methods: */
	bool isValid(void) const // Returns true if the header described a valid PLY file
		{
		return valid;
		}
	FileType getFileType(void) const // Returns the file's type
		{
		return fileType;
		}
	Misc::Endianness getFileEndianness(void) const // Returns the endianness for binary PLY files
		{
		return fileEndianness;
		}
	size_t getNumElements(void) const // Returns the number of elements in the PLY file
		{
		return elements.size();
		}
	const PLYElement& getElement(size_t index) const // Returns the element of the given index
		{
		return elements[index];
		}
	};

/****************
Global functions:
****************/

void skipElement(const PLYElement& element,IO::File& plyFile); // Skips all values associated with the given element in the given binary PLY file
void skipElement(const PLYElement& element,IO::ValueSource& plyFile); // Skips all values associated with the given element in the given ASCII PLY file

#endif
