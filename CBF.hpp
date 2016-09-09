#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <vector>
//#include <cstdint> // include this header for uint64_t

using namespace std;

template <typename key_type> class CBF {
        unsigned char*  mem;        // CBF memory
        unsigned        num_buckets; // size of CBF memory
        unsigned        num_hash; // number of hashes
        unsigned        n;   	     // number of inserted item
        unsigned        num_zeros;  // number of zeroes in the filter

        public:
        CBF();
        CBF(int h, int m);
        virtual ~CBF();

        void setsize(int h, int m);
        void clear();
        bool erase(key_type item);
        bool insert(key_type item);
        bool check(key_type item);
        unsigned get_nzero() {return num_zeros;}
        void dump();

    };
