#include "CBF.hpp"
#include "city.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <cmath>

template <typename T>  
uint64 CityHash(T key, uint64_t seed) 
{
    char* k = reinterpret_cast<char*>(&key);
    return CityHash64WithSeed(k,sizeof(key),seed);
}

template <typename T>  
uint64 CityHash(std::string key, uint64_t seed) 
{
    return CityHash64WithSeed(key.c_str(),key.length(),seed);
}

template <typename T> int myhash(T key, int i, int s) {
    uint64_t  val0;
    uint64_t  val1;
    uint64_t   val;
    int ss=s;

    val0=CityHash<T>(key,3015) % ss;
    val1=CityHash<T>(key,7793) % ss;
    if (val1==val0) {
        val1 = (val1 +1) % ss;
    }
    if (i==0) val=val0;
    if (i==1) val=val1;
    if (i>1)  val=CityHash<T>(key,2137+i) % ss;
    return (val %ss);


}

template <typename key_type>
CBF<key_type>::CBF(int h, int m)
{
	//allocation of BF memory
	mem = new unsigned char[m];
	n=0;
        num_buckets=m;
	num_hash=h;
        CBF::clear();
}

/*
 * Distructor
 */
template <typename key_type>
CBF<key_type>::~CBF()
{
	delete[] mem;
}


/*
 * Clear
 */

template <typename key_type>
void CBF<key_type>::clear()
{
	for(unsigned i=0; i<num_buckets; i++) mem[i]=0;
        n=0;
	num_zeros=num_buckets;
}

/*
 * Insert
 */
template <typename key_type>
bool CBF<key_type>::insert(key_type item)
{
    if (!CBF<key_type>::check(item)) n++;
       //printf("debug BF:input to insert: ");
        //for (int i=0; i<len;i++) printf("%02x ",in[i]);
        for(unsigned i=0; i<num_hash; i++) {
		int index=myhash<key_type>(item,i+2,num_buckets);
		mem[index]++;
	}
	return 0;
}

/*
 * Delete
 */
template <typename key_type>
bool CBF<key_type>::erase(key_type item)
{
    if (!CBF<key_type>::check(item)) {
        //printf("ERROR: try to remove an item (%ld) not present in the BF\n",item);
        printf("ERROR: try to remove an item not present in the BF\n");
        return false;
    }
    n--;
    //printf("debug BF:input to insert: ");
    //for (int i=0; i<len;i++) printf("%02x ",in[i]);
    //printf("\n");
    for(unsigned i=0; i<num_hash; i++) {
        unsigned index=myhash<key_type>(item,i+2,num_buckets);
        mem[index]--;
    }
    return true;
}
/*
 * Check
 */
template <typename key_type>
bool CBF<key_type>::check(key_type item)
{
	unsigned int index;
        //printf("debug BF:input to check: ",in);
        //for (int i=0; i<len;i++) printf("%02x ",in[i]);
        //printf("\n");
        for(unsigned i=0; i<num_hash; i++){
		index=myhash<key_type>(item,i+2,num_buckets);
		//if (mem[index]==0)
                //    printf("MISS!\n");
                if (mem[index]==0) return false;
	}
	//printf("HIT!\n");
	return true;
}

/*
 * Dump
 */
template <typename key_type>
void CBF<key_type>::dump()
{
	for(unsigned i=0; i<num_buckets; i++)
		printf ("dump: mem[%d]= %d\n",i, mem[i]);
}
