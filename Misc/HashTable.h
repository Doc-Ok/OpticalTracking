/***********************************************************************
HashTable - Class for storing and finding values (bucketed version)
Copyright (c) 1998-2011 Oliver Kreylos

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

#ifndef MISC_HASHTABLE_INCLUDED
#define MISC_HASHTABLE_INCLUDED

#include <new>
#include <stdexcept>
#include <Misc/PoolAllocator.h>
#include <Misc/StandardHashFunction.h>

namespace Misc {

template <class Source,class Dest>
class HashTableEntry
	{
	/* Elements: */
	private:
	Source source; // Source (key) value
	Dest dest; // Destination value
	
	/* Constructors and destructors: */
	public:
	HashTableEntry(const Source& sSource) // Creates entry with uninitialized/default destination value
		:source(sSource)
		{
		}
	HashTableEntry(const Source& sSource,const Dest& sDest) // Elementwise constructor
		:source(sSource),dest(sDest)
		{
		}
	
	/* Methods: */
	const Source& getSource(void) const // Returns source (key) as non-modifiable L-value
		{
		return source;
		}
	const Dest& getDest(void) const // Returns value as non-modifiable L-value
		{
		return dest;
		}
	Dest& getDest(void) // Ditto as modifiable L-value
		{
		return dest;
		}
	HashTableEntry& operator=(const Dest& newDest) // Assigns a new value
		{
		dest=newDest;
		return *this;
		}
	};

template <class Source>
class HashTableEntry<Source,void> // Specialized hash table entry with no value
	{
	/* Elements: */
	private:
	Source source; // Source (key) value
	
	/* Constructors and destructors: */
	public:
	HashTableEntry(const Source& sSource)
		:source(sSource)
		{
		}
	
	/* Methods: */
	const Source& getSource(void) const // Returns source (key) as non-modifiable L-value
		{
		return source;
		}
	};

/***********************************************************************
Usage prerequisites:
- class Source must provide operator!=
- class HashFunction must provide static size_t hash(const Source&
  source,size_t tableSize)
***********************************************************************/

template <class Source,class Dest,class HashFunction =StandardHashFunction<Source> >
class HashTable
	{
	/* Embedded classes: */
	public:
	typedef HashTableEntry<Source,Dest> Entry; // Type for hash table entries
	
	class EntryNotFoundError:public std::runtime_error // Class for exceptions when requested hash table entry does not exist
		{
		/* Elements: */
		public:
		Source entrySource; // Requested non-existent entry source value
		
		/* Constructors and destructors: */
		EntryNotFoundError(const Source& sEntrySource)
			:std::runtime_error("Requested entry not found in hash table"),
			 entrySource(sEntrySource)
			{
			}
		virtual ~EntryNotFoundError(void) throw()
			{
			}
		};
	
	private:
	class HashBucketItem:public Entry // Class to store lists of entries for each hash bucket
		{
		/* Elements: */
		public:
		HashBucketItem* succ; // Pointer to next item in hash bucket (or null)
		
		/* Constructors and destructors: */
		HashBucketItem(const Entry& sEntry) // Creates unlinked hash bucket item
			:Entry(sEntry),succ(0)
			{
			}
		
		/* Methods: */
		HashBucketItem& setEntry(const Entry& source) // Copies a hash table entry into a hash bucket item
			{
			Entry::operator=(source);
			return *this;
			}
		};
	
	class HashBucket // Class to store entries with the same hash value
		{
		/* Elements: */
		public:
		HashBucketItem* firstItem; // Pointer to first item in bucket (or null if bucket is empty)
		
		/* Constructors and destructors: */
		HashBucket(void) // Creates empty hash bucket
			:firstItem(0)
			{
			}
		};
	
	public:
	class Iterator
		{
		friend class HashTable;
		
		/* Elements: */
		private:
		HashTable* table; // Pointer to table this iterator is pointing into
		size_t bucketIndex; // Index of current hash bucket
		HashBucketItem* bucketItem; // Pointer to current entry in hash bucket
		
		/* Constructors and destructors: */
		public:
		Iterator(void) // Creates invalid iterator
			:table(0),bucketIndex(0),bucketItem(0)
			{
			}
		private:
		Iterator(HashTable* sTable) // Creates iterator to first entry in hash table
			:table(sTable),bucketIndex(0),bucketItem(table->hashBuckets[bucketIndex].firstItem)
			{
			while(bucketItem==0)
				{
				++bucketIndex;
				if(bucketIndex==table->tableSize)
					break; // End of table reached
				bucketItem=table->hashBuckets[bucketIndex].firstItem;
				}
			}
		Iterator(HashTable* sTable,size_t sBucketIndex,HashBucketItem* sBucketItem) // Elementwise constructor
			:table(sTable),bucketIndex(sBucketIndex),bucketItem(sBucketItem)
			{
			}
		
		/* Methods: */
		public:
		bool isFinished(void) const
			{
			return bucketIndex>=table->tableSize;
			}
		friend bool operator==(const Iterator& it1,const Iterator& it2)
			{
			return it1.bucketItem==it2.bucketItem;
			}
		friend bool operator!=(const Iterator& it1,const Iterator& it2)
			{
			return it1.bucketItem!=it2.bucketItem;
			}
		Entry& operator*(void) const
			{
			return *bucketItem;
			}
		Entry* operator->(void) const
			{
			return bucketItem;
			}
		Iterator& operator++(void)
			{
			/* Go to next item in same hash bucket if possible: */
			bucketItem=bucketItem->succ;
			
			/* Go to next non-empty hash bucket if last item in current bucket is passed: */
			while(bucketItem==0)
				{
				++bucketIndex;
				if(bucketIndex==table->tableSize)
					break; // End of table reached
				bucketItem=table->hashBuckets[bucketIndex].firstItem;
				}
			return *this;
			}
		};
	
	class ConstIterator
		{
		friend class HashTable;
		
		/* Elements: */
		private:
		const HashTable* table; // Pointer to table this iterator is pointing into
		size_t bucketIndex; // Index of current hash bucket
		const HashBucketItem* bucketItem; // Pointer to current entry in hash bucket
		
		/* Constructors and destructors: */
		public:
		ConstIterator(void) // Creates invalid iterator
			:table(0),bucketIndex(0),bucketItem(0)
			{
			}
		private:
		ConstIterator(const HashTable* sTable) // Creates iterator to first entry in hash table
			:table(sTable),bucketIndex(0),bucketItem(table->hashBuckets[bucketIndex].firstItem)
			{
			while(bucketItem==0)
				{
				++bucketIndex;
				if(bucketIndex==table->tableSize)
					break; // End of table reached
				bucketItem=table->hashBuckets[bucketIndex].firstItem;
				}
			}
		ConstIterator(const HashTable* sTable,size_t sBucketIndex,const HashBucketItem* sBucketItem) // Elementwise constructor
			:table(sTable),bucketIndex(sBucketIndex),bucketItem(sBucketItem)
			{
			}
		
		/* Methods: */
		public:
		bool isFinished(void) const
			{
			return bucketIndex>=table->tableSize;
			}
		friend bool operator==(const ConstIterator& it1,const ConstIterator& it2)
			{
			return it1.bucketItem==it2.bucketItem;
			}
		friend bool operator!=(const ConstIterator& it1,const ConstIterator& it2)
			{
			return it1.bucketItem!=it2.bucketItem;
			}
		const Entry& operator*(void) const
			{
			return *bucketItem;
			}
		const Entry* operator->(void) const
			{
			return bucketItem;
			}
		ConstIterator& operator++(void)
			{
			/* Go to next item in same hash bucket if possible: */
			bucketItem=bucketItem->succ;
			
			/* Go to next non-empty hash bucket if last item in current bucket is passed: */
			while(bucketItem==0)
				{
				++bucketIndex;
				if(bucketIndex==table->tableSize)
					break; // End of table reached
				bucketItem=table->hashBuckets[bucketIndex].firstItem;
				}
			return *this;
			}
		};
	
	friend class Iterator;
	friend class ConstIterator;
	
	/* Elements: */
	size_t tableSize; // Current table size
	float waterMark; // Maximum table usage ratio
	float growRate; // Rate the table grows at
	HashBucket* hashBuckets; // Array of hash buckets
	size_t usedEntries; // Number of entries currently used
	size_t maxEntries; // Maximum number of entries at current table size
	PoolAllocator<HashBucketItem> hashBucketItemAllocator; // Memory allocator for hash bucket items
	
	/* Private methods: */
	void growTable(size_t newTableSize) // Grows the table without deleting current entries
		{
		/* Allocate new hash buckets: */
		HashBucket* newHashBuckets=new HashBucket[newTableSize];
		
		/* Move all entries to the new table: */
		for(size_t i=0;i<tableSize;++i)
			{
			/* Move all items in this hash bucket to the new table: */
			while(hashBuckets[i].firstItem!=0)
				{
				/* Remove item from old hash bucket: */
				HashBucketItem* item=hashBuckets[i].firstItem;
				hashBuckets[i].firstItem=item->succ;
				
				/* Calculate item's new hash bucket index: */
				size_t newBucketIndex=HashFunction::hash(item->getSource(),newTableSize);
				
				/* Add item to new hash bucket: */
				item->succ=newHashBuckets[newBucketIndex].firstItem;
				newHashBuckets[newBucketIndex].firstItem=item;
				}
			}
		
		/* Install the new hash table: */
		tableSize=newTableSize;
		delete[] hashBuckets;
		hashBuckets=newHashBuckets;
		maxEntries=(size_t)(tableSize*waterMark);
		}
	
	/* Constructors and destructors: */
	public:
	HashTable(size_t sTableSize,float sWaterMark =0.9f,float sGrowRate =1.7312543)
		:tableSize(sTableSize),waterMark(sWaterMark),growRate(sGrowRate),
		 hashBuckets(new HashBucket[tableSize]),
		 usedEntries(0),maxEntries((size_t)(tableSize*waterMark))
		{
		}
	~HashTable(void)
		{
		/* Destroy all used hash table entries: */
		for(size_t i=0;i<tableSize;++i)
			{
			while(hashBuckets[i].firstItem!=0)
				{
				HashBucketItem* succ=hashBuckets[i].firstItem->succ;
				
				/* Destroy item: */
				hashBuckets[i].firstItem->~HashBucketItem();
				hashBucketItemAllocator.free(hashBuckets[i].firstItem);
				
				hashBuckets[i].firstItem=succ;
				}
			}
		
		/* Delete hash buckets: */
		delete[] hashBuckets;
		}
	
	/* Methods: */
	void setTableSize(size_t newTableSize)
		{
		growTable(newTableSize);
		}
	void clear(void)
		{
		/* Destroy all used hash table entries: */
		for(size_t i=0;i<tableSize;++i)
			{
			while(hashBuckets[i].firstItem!=0)
				{
				HashBucketItem* succ=hashBuckets[i].firstItem->succ;
				
				/* Destroy item: */
				hashBuckets[i].firstItem->~HashBucketItem();
				hashBucketItemAllocator.free(hashBuckets[i].firstItem);
				
				hashBuckets[i].firstItem=succ;
				}
			}
		
		usedEntries=0;
		}
	size_t getNumEntries(void) const // Returns the number of entries currently in the hash table
		{
		return usedEntries;
		}
	bool setEntry(const Entry& newEntry)
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(newEntry.getSource(),tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		HashBucketItem* pred=0;
		HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=newEntry.getSource())
			{
			pred=item;
			item=item->succ;
			}
		
		if(item!=0)
			{
			/* Set value of existing entry: */
			item->setEntry(newEntry);
			}
		else
			{
			/* Insert new entry: */
			HashBucketItem* newItem=new(hashBucketItemAllocator.allocate()) HashBucketItem(newEntry);
			if(pred!=0)
				pred->succ=newItem;
			else
				hashBuckets[index].firstItem=newItem;
			++usedEntries;
			
			/* Grow hash table if necessary: */
			if(usedEntries>maxEntries)
				growTable((size_t)(tableSize*growRate)+1);
			}
		
		return item!=0;
		}
	void removeEntry(const Source& findSource) // Removes entry
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(findSource,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		HashBucketItem* pred=0;
		HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=findSource)
			{
			pred=item;
			item=item->succ;
			}
		
		if(item!=0)
			{
			/* Remove item from bucket: */
			if(pred!=0)
				pred->succ=item->succ;
			else
				hashBuckets[index].firstItem=item->succ;
				
			/* Destroy item: */
			item->~HashBucketItem();
			hashBucketItemAllocator.free(item);
				
			--usedEntries;
			}
		}
	bool isEntry(const Source& findSource) const
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(findSource,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		const HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=findSource)
			item=item->succ;
		
		return item!=0;
		}
	bool isEntry(const Entry& entry) const // Wrapper for isEntry function
		{
		return isEntry(entry.getSource());
		}
	const Entry& getEntry(const Source& findSource) const // Returns reference to entry; throws exception if entry is not found
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(findSource,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		const HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=findSource)
			item=item->succ;
		
		/* Throw an exception if the requested entry does not exist: */
		if(item==0)
			throw EntryNotFoundError(findSource);
		
		return *item;
		}
	Entry& getEntry(const Source& findSource) // Ditto
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(findSource,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=findSource)
			item=item->succ;
		
		/* Throw an exception if the requested entry does not exist: */
		if(item==0)
			throw EntryNotFoundError(findSource);
		
		return *item;
		}
	Entry& operator[](const Source& source) // Returns reference to entry; inserts new entry if source is not found
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(source,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		HashBucketItem* pred=0;
		HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=source)
			{
			pred=item;
			item=item->succ;
			}
		
		if(item==0)
			{
			/* Insert new entry with default destination: */
			item=new(hashBucketItemAllocator.allocate()) HashBucketItem(Entry(source));
			if(pred!=0)
				pred->succ=item;
			else
				hashBuckets[index].firstItem=item;
			++usedEntries;
			
			/* Grow hash table if necessary: */
			if(usedEntries>maxEntries)
				growTable((size_t)(tableSize*growRate)+1);
			}
		
		return *item;
		}
	Iterator begin(void)
		{
		return Iterator(this); // Create iterator to first entry
		}
	ConstIterator begin(void) const
		{
		return ConstIterator(this); // Create iterator to first entry
		}
	Iterator end(void)
		{
		return Iterator(this,tableSize,0); // Create iterator past end of table
		}
	ConstIterator end(void) const
		{
		return ConstIterator(this,tableSize,0); // Create iterator past end of table
		}
	Iterator findEntry(const Source& findSource)
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(findSource,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=findSource)
			item=item->succ;
		
		if(item!=0)
			return Iterator(this,index,item); // Return valid iterator
		else
			return Iterator(this,tableSize,0); // Return end iterator
		}
	ConstIterator findEntry(const Source& findSource) const
		{
		/* Calculate the searched entry's hash bucket index: */
		size_t index=HashFunction::hash(findSource,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		const HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->getSource()!=findSource)
			item=item->succ;
		
		if(item!=0)
			return ConstIterator(this,index,item); // Return valid iterator
		else
			return ConstIterator(this,tableSize,0); // Return end iterator
		}
	void removeEntry(const Iterator& it) // Removes entry pointed to by iterator
		{
		if(it.table==this&&it.bucketIndex<tableSize&&it.bucketItem!=0)
			{
			/* Find entry in its bucket: */
			HashBucketItem* pred=0;
			HashBucketItem* item=hashBuckets[it.bucketIndex].firstItem;
			while(item!=0&&item!=it.bucketItem)
				{
				pred=item;
				item=item->succ;
				}
			
			/* Remove item from bucket: */
			if(pred!=0)
				pred->succ=item->succ;
			else
				hashBuckets[it.bucketIndex].firstItem=item->succ;
				
			/* Destroy item: */
			item->~HashBucketItem();
			hashBucketItemAllocator.free(item);
				
			--usedEntries;
			}
		}
	};

}

#endif
