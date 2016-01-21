/* Copyright (C)  2005-2008 Infobright Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2.0 as
published by the Free  Software Foundation.

This program is distributed in the hope that  it will be useful, but
WITHOUT ANY WARRANTY; without even  the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License version 2.0 for more details.

You should have received a  copy of the GNU General Public License
version 2.0  along with this  program; if not, write to the Free
Software Foundation,  Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA  */

#ifndef __COMPRESSION_DICTIONARY_H
#define __COMPRESSION_DICTIONARY_H

#include "RangeCoder.h"
#include "defs.h"
#include "common/bhassert.h"

// Data structure which holds a dictionary of numerical values used for compression
// and realises the mappings: 
//   key --> range (using a hash map) 
//   count --> key (using an array for every count)
// NBUCK usually should be a prime.
// MAXKEYS must be smaller than SHRT_MAX = 32767.
template <class T = _uint64>
class Dictionary
{
public:
#ifndef SOLARIS
	static const uint MAXTOTAL = RangeCoder::MAX_TOTAL;
#endif
	static const uint NBUCK;
	static const ushort MAXKEYS;

	struct KeyRange {
		T key;
		uint count;
		uint low;			// lows are set when all keys are inserted
	};

private:
	KeyRange* keys;			// [MAXKEYS]
	short nkeys;

	// Hash table to index 'keys' array according to the 'key' field
	short* buckets;			// [NBUCK] indices into 'keys'; -1 means empty bucket
	short* next;			// [MAXKEYS] next[k] is the next element in a bucket after key no. k, or -1
	uint hash(T key)		// hash function
	{ BHASSERT_WITH_NO_PERFORMANCE_IMPACT(NBUCK >= 65536); return (ushort)key; }

	KeyRange** order;		// [MAXKEYS]

	// For decompression
	short* cnt2val;			// [MAXTOTAL] cnt2val[c] is an index of the key for cumulative count 'c'
	uint tot_shift;			// total = 1 << tot_shift
	
	static int compare(const void* p1, const void* p2);		// for sorting keys by descending 'count'
	bool compress, decompress;	// says if internal structures are set to perform compression or decompression
	void Clear();

public:
	Dictionary(bool fullalloc = true);		// fullalloc can be false if the Dictionary is not used for en/decoding
	~Dictionary();

	// Insert(): if 'key' is already in dictionary, increase its count by 'count'.
	// Otherwise insert the key and set count to 'count'.
	void InitInsert()						{ Clear(); }
	bool Insert(T key, uint count = 1);			// returns false if too many keys
	
	KeyRange* GetKeys(short& n)				{ n = nkeys; return keys; }
	void SetLows();								// set lows/highs of keys
	
	void Save(RangeCoder* dest, T maxkey);		// maxkey - the largest key or something bigger
	void Load(RangeCoder* src, T maxkey);		// maxkey must be the same as for Save()

	bool Encode(RangeCoder* dest, T key);		// returns true when ESC was encoded ('key' is not in dictionary)
	bool Decode(RangeCoder* src, T& key);
};


template<class T> inline bool Dictionary<T>::Insert(T key, uint count)
{
	uint b = hash(key);
	short k = buckets[b];
	while((k >= 0) && (keys[k].key != key))
		k = next[k];
	
	if(k < 0) {
		if(nkeys >= MAXKEYS) return false;
		keys[nkeys].key = key;
		keys[nkeys].count = count;
		next[nkeys] = buckets[b];			// TODO: time - insert new keys at the END of the list
		buckets[b] = nkeys++;
	}
	else keys[k].count += count;
	return true;
}

template<class T> inline bool Dictionary<T>::Encode(RangeCoder* dest, T key)
{
	BHASSERT_WITH_NO_PERFORMANCE_IMPACT(compress);
	
	// find the 'key' in the hash
	uint b = hash(key);
	short k = buckets[b];
	while((k >= 0) && (keys[k].key != key))
		k = next[k];
	BHASSERT_WITH_NO_PERFORMANCE_IMPACT(k >= 0);			// TODO: handle ESC encoding
	
	dest->EncodeShift(keys[k].low, keys[k].count, tot_shift);
	return false;
}

template<class T> inline bool Dictionary<T>::Decode(RangeCoder* src, T& key)
{
	BHASSERT_WITH_NO_PERFORMANCE_IMPACT(decompress);
	uint count = src->GetCountShift(tot_shift);
	short k = cnt2val[count];		// TODO: handle ESC decoding
	key = keys[k].key;
	src->DecodeShift(keys[k].low, keys[k].count, tot_shift);
	return false;
}


template<> inline uint Dictionary<uchar>::hash(uchar key) {
	BHASSERT_WITH_NO_PERFORMANCE_IMPACT(NBUCK >= 256); 
	return key;
}


#endif
