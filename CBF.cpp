#include "CBF.hpp"
#include "city.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

CBF::CBF(int h, int m)
{
	//allocation of BF memory
	mem = new unsigned char[m];
	n=0;
        num_buckets=m;
	num_hash=h;
}

/*
 * Distructor
 */
CBF::~CBF()
{
	delete[] mem;
}


/*
 * Clear
 */

void CBF::clear()
{
	for(unsigned i=0; i<num_buckets; i++) mem[i]=0;
        n=0;
	num_zeros=num_buckets;
}

/*
 * Insert
 */
bool CBF::insert(int64_t item)
{
    if (!CBF::check(item)) n++;
       //printf("debug BF:input to insert: ");
        //for (int i=0; i<len;i++) printf("%02x ",in[i]);
        for(unsigned i=0; i<num_hash; i++) {
		int index=myhash(item,i+2,num_buckets);
		mem[index]++;
	}
	return 0;
}

/*
 * Delete
 */
bool CBF::erase(int64_t item)
{
    if (!CBF::check(item)) {
        printf("ERROR: try to remove an item (%ld) not present in the BF\n",item);
        return false;
    }
    n--;
    //printf("debug BF:input to insert: ");
    //for (int i=0; i<len;i++) printf("%02x ",in[i]);
    //printf("\n");
    for(unsigned i=0; i<num_hash; i++) {
        unsigned index=myhash(item,i+2,num_buckets);
        mem[index]--;
    }
    return true;
}
/*
 * Check
 */
bool CBF::check(int64_t item)
{
	unsigned int index;
        //printf("debug BF:input to check: ",in);
        //for (int i=0; i<len;i++) printf("%02x ",in[i]);
        //printf("\n");
        for(unsigned i=0; i<num_hash; i++){
		index=myhash(item,i+2,num_buckets);
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
void CBF::dump()
{
	for(unsigned i=0; i<num_buckets; i++)
		printf ("dump: mem[%d]= %d\n",i, mem[i]);
}
