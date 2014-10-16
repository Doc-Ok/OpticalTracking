/***********************************************************************
OneTimeQueue - Class to enumerate objects in a FIFO fashion, with any
object enumerated exactly once. Derived from HashTable.
Copyright (c) 1998-2005 Oliver Kreylos

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

#ifndef MISC_ONETIMEQUEUE_INCLUDED
#define MISC_ONETIMEQUEUE_INCLUDED

#include <new>
#include <Misc/ChunkedArray.h>
#include <Misc/StandardHashFunction.h>

namespace Misc {

/***********************************************************************
Usage prerequisites:
- class Source must provide operator!=
- class HashFunction must provide static size_t hash(const Source&
  source,size_t tableSize)
***********************************************************************/

template <class ContentParam,class HashFunctionParam =StandardHashFunction<ContentParam> >
class OneTimeQueue
	{
	/* Embedded classes: */
	public:
	typedef ContentParam Content; // Type of elements stored in queue
	typedef HashFunctionParam HashFunction;
	typedef size_t size_type;
	
	private:
	class HashBucketItem // Class to store lists of entries for each hash bucket
		{
		/* Elements: */
		public:
		Content entry; // Entry stored in hash bucket item
		HashBucketItem* queueSucc; // Pointer to next element in queue
		HashBucketItem* succ; // Pointer to next item in hash bucket (or null)
		
		/* Constructors and destructors: */
		HashBucketItem(const Content& sEntry) // Creates unlinked hash bucket item
			:entry(sEntry),queueSucc(0),succ(0)
			{
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
	
	/* Elements: */
	size_t tableSize; // Current table size
	float waterMark; // Maximum table usage ratio
	float growRate; // Rate the table grows at
	HashBucket* hashBuckets; // Array of hash buckets
	size_t usedEntries; // Number of entries currently used
	size_t maxEntries; // Maximum number of entries at current table size
	ChunkedArray<HashBucketItem> hashBucketItems; // Array of hash bucket items
	HashBucketItem* queueHead; // Pointer to first hash bucket item in queue
	HashBucketItem* queueTail; // Pointer to last hash bucket item in queue
	
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
				size_t newBucketIndex=HashFunction::hash(item->entry,newTableSize);
				
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
	OneTimeQueue(size_t sTableSize,float sWaterMark =0.9f,float sGrowRate =1.7312543)
		:tableSize(sTableSize),waterMark(sWaterMark),growRate(sGrowRate),
		 hashBuckets(new HashBucket[tableSize]),
		 usedEntries(0),maxEntries((size_t)(tableSize*waterMark)),
		 queueHead(0),queueTail(0)
		{
		}
	private:
	OneTimeQueue(const OneTimeQueue& source); // Prohibit copy constructor
	OneTimeQueue& operator=(const OneTimeQueue& source); // Prohibit assignment operator
	public:
	~OneTimeQueue(void)
		{
		/* Destroy all used hash table entries: */
		clear();
		
		/* Delete hash buckets: */
		delete[] hashBuckets;
		}
	
	/* Methods: */
	void setTableSize(size_t newTableSize)
		{
		growTable(newTableSize);
		}
	size_t getNumEntries(void) const
		{
		return usedEntries;
		}
	bool empty(void) const
		{
		return queueHead==0;
		}
	const Content& front(void) const
		{
		return queueHead->entry;
		}
	Content& front(void)
		{
		return queueHead->entry;
		}
	void push(const Content& newEntry)
		{
		/* Calculate the new entry's hash bucket index: */
		size_t index=HashFunction::hash(newEntry,tableSize);
		
		/* Compare items in the hash bucket until match is found: */
		HashBucketItem* pred=0;
		HashBucketItem* item=hashBuckets[index].firstItem;
		while(item!=0&&item->entry!=newEntry)
			{
			pred=item;
			item=item->succ;
			}
		
		/* Only push entry if it was not already in the hash table: */
		if(item==0)
			{
			/* Insert new entry: */
			hashBucketItems.push_back(newEntry);
			HashBucketItem* newItem=&hashBucketItems.back();
			if(pred!=0)
				pred->succ=newItem;
			else
				hashBuckets[index].firstItem=newItem;
			++usedEntries;
			
			/* Append it to the queue: */
			if(queueTail!=0)
				queueTail->queueSucc=newItem;
			else
				queueHead=newItem;
			queueTail=newItem;
			
			/* Grow hash table if necessary: */
			if(usedEntries>maxEntries)
				growTable((size_t)(tableSize*growRate)+1);
			}
		}
	void pop(void)
		{
		/* Remove head of the queue: */
		if(queueTail==queueHead)
			queueTail=0;
		HashBucketItem* succ=queueHead->queueSucc;
		queueHead->queueSucc=0;
		queueHead=succ;
		}
	void clear(void)
		{
		/* Destroy all used hash table entries: */
		hashBucketItems.clear();
		
		/* Reset all hash buckets: */
		for(size_t i=0;i<tableSize;++i)
			hashBuckets[i].firstItem=0;
		
		usedEntries=0;
		queueHead=0;
		queueTail=0;
		}
	};

}

#endif
