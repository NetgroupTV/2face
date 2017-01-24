#include "utils.h"
#include "CBF.hpp"
#include <tuple>
#include <vector>
#include <iostream>
#include <cstdint> // include this header for uint64_t
#include <cstring>

using namespace std;

//int tot_movements=0;
//int verbose=0;

template <typename key_type, typename value_type> class HTBFmap {
    bool        ***present_table;    //  present flag memory
    pair<key_type,value_type>  ***table;      // entries stored in the HT

    //CBF array[cbf_size][K]
    CBF<key_type>** cbf_array;
    int cbf_size;
    int m;                     // size of a table
    int b;     	               // number of slots in a bucket
    int K;     	               // number of way
    int num_item;              // number of inserted item
    int tmax;
    bool victim_flag;
    key_type victim_key;
    value_type victim_value;

        public:
                //HTBFmap();
                //HTBFmap(int way, int buckets, int hsize);
                HTBFmap(int way, int buckets, int hsize,int t, int bf_size);
                ~HTBFmap();
                void clear();
                //void expand();
                bool insert(key_type key,value_type value);
                std::vector<int> fullinsert(key_type key,value_type value);

                //LHS operator[]
                value_type& operator[](key_type key);
                //RHS operator[]
                const value_type operator[](key_type key) const  {return HTBFmap::query(key); }
                value_type query(key_type key);
                // return the value, the position (i,ii,p) and the number of accesses
                //tuple<value_type,int,int,int,int> fullquery(key_type key);
                std::vector<int> fullquery(key_type key);
                key_type  get_key(int i, int ii, int p);
                int count(key_type key);
                bool remove(key_type key);
                bool erase(key_type key) {return HTBFmap::remove(key);}
                long unsigned int size() {return (long unsigned int) num_item;}
                int get_nitem() {return num_item;}
                int get_size() {return K*b*m;}
};

/*
 * Constructor
 */


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

template <typename key_type, typename value_type>
HTBFmap<key_type,value_type>::HTBFmap(int way, int buckets, int hsize,int t, int bf_size)
{
  tmax=t;
  m = hsize; // size of memory
  b = buckets;
  K = way;
  cbf_size=bf_size;
  num_item=0;   	// number of inserted item

  //allocation of HT memory Kxbxm
  present_table = new bool**[K];
  table = new pair<key_type,value_type>**[K];
  for (int i = 0;  i <K;  i++) {
      present_table[i] = new bool*[b];
      table[i]= new pair<key_type,value_type>*[b];
      for (int ii = 0;  ii <b;  ii++){
          present_table[i][ii] = new bool[m];
          table[i][ii]= new pair<key_type,value_type>[m];
      }
  }

  // alloca un CBF array di dimensioni bf_size,way,num_hash
  cbf_array = new CBF<key_type>*[K];
  for (int i = 0;  i <K;  i++) {
      cbf_array[i] = new CBF<key_type>[bf_size];
      for (int ii = 0;  ii <bf_size;  ii++) {
          //cbf_array[i][ii].setsize(4,4096);
          cbf_array[i][ii].setsize(4,32);
      }
  }
  clear();
}

/*
 * Distructor
 */
template <typename key_type, typename value_type>
HTBFmap<key_type,value_type>::~HTBFmap()
{
    for (int i = 0;  i <K;  i++){
        for (int ii = 0;  ii < b;  ii++){
            delete[] present_table[i][ii];
            delete[] table[i][ii];
        }
        delete[] present_table[i];
        delete[] table[i];
    }
    delete[] present_table;
    delete[] table;
/*
  // dealloca il CBF array
    for (int i = 0;  i <K;  i++) {
        for (int ii = 0;  i <cbf_size;  ii++) {
            //~cbf_array[i][ii];
        }
        delete[] cbf_array[i];
    }
    delete[] cbf_array;
*/
}


/*
 * Clear
 */

template <typename key_type, typename value_type>
void HTBFmap<key_type,value_type>::clear()
{
	num_item=0;
        victim_flag=false;
        for (int i = 0;  i <K;  i++) {
            for (int ii = 0;  ii <b;  ii++)
                for (int iii = 0;  iii <m;  iii++){
                present_table[i][ii][iii]=false;
                }
    }
  // pulisce il CBF array
    // dealloca il CBF array
    for (int i = 0;  i <K;  i++) {
        for (int ii = 0;  ii <cbf_size;  ii++) {
            cbf_array[i][ii].clear();
        }
    }
}


/*
 * Insert
 */
template <typename key_type, typename value_type>
bool HTBFmap<key_type,value_type>::insert(key_type key,value_type value)
{
    //update value if exist
    if ((key==victim_key) && (victim_flag)) {
        victim_value=value;
        return true;
    }
    for (int i = 0;  i <K;  i++){
        int p = myhash<key_type>(key,i,m);
        for (int ii = 0;  ii <b;  ii++)
            if ((present_table[i][ii][p]) && (table[i][ii][p].first== key)) {
                table[i][ii][p].second=value;
                return true;
            }
    }

    // try cuckoo
    for (int t = 0;  t <= tmax;  t++) {

        // search for empty places
        for (int i = 0;  i <K;  i++){
            int p = myhash<key_type>(key,i,m);
            for (int ii = 0;  ii <b;  ii++)
                if (!present_table[i][ii][p]) {  //insert in an empty place
                    present_table[i][ii][p] = true;
                    table[i][ii][p]={key,value};
                    num_item++;
                    //inserisci key nel CBF i in posizione z=hash(key,100)
                    int z=myhash<key_type>(key,100,cbf_size);
                    cbf_array[i][z].insert(key);
                    return true;
                }
        }

        // finally play the cuckoo;
        int j = rand() % K;
        int jj = rand() % b;
        int p = myhash<key_type>(key,j,m);
        key_type new_key = table[j][jj][p].first;
        value_type new_value = table[j][jj][p].second;
        table[j][jj][p]={key,value};
                    
        //rimuovi new_key dal CBF j
        int z=myhash<key_type>(new_key,100,cbf_size);
        cbf_array[j][z].erase(new_key);

        //inserisci key in CBF j
        z=myhash<key_type>(key,100,cbf_size);
        cbf_array[j][z].insert(key);


        key=new_key;
        value=new_value;
    }
    verprintf("insertion failed\n");
    printf("HTBFmap:: insertion failed\n");
    //if (verbose==1) cout << "key:<" << key.first <<","<< key.second <<">" <<endl;
    //if (verbose==1) cout << "value: " << value <<endl;

    victim_flag=true;
    victim_key=key;
    victim_value=value;
    return false;
}

//LHS operator[]
template <typename key_type, typename value_type>
value_type& HTBFmap<key_type,value_type>::operator[](key_type key) {


    if (HTBFmap<key_type,value_type>::count(key)==0){ //insert if not exist
        HTBFmap<key_type,value_type>::insert(key,victim_value);
    }

    //update value
    if ((key==victim_key) && (victim_flag)) {
        //return result;
        return victim_value;
    }
    for (int i = 0;  i <K;  i++){
        int p = myhash<key_type>(key,i,m);
        for (int ii = 0;  ii <b;  ii++)
            if ((present_table[i][ii][p]) &&  (table[i][ii][p].first== key)) {
                return table[i][ii][p].second;
            }
    }
    printf("ERROR in operator[]\n");
    exit(1);
    return victim_value;
}


/*
 * Query
 */
template <typename key_type, typename value_type>
value_type HTBFmap<key_type,value_type>::query(key_type key)
{
    if ((key==victim_key) && (victim_flag)) return victim_value;
    for (int i = 0;  i <K;  i++) {
        for (int ii = 0;  ii <b;  ii++){
            int p = myhash<key_type>(key,i,m);
            //verprintf("query item in table[%d][%d] for p=%d and f=%d\n",p,jj,p,fingerprint);
            //verprintf("result is: %d\n",table[p][jj]);
            if ((present_table[i][ii][p]) &&  (table[i][ii][p].first== key)) {
                return table[i][ii][p].second;
            }
        }
    }
    return victim_value;
}
/*
 * Full Insert
 */
template <typename key_type, typename value_type>
vector<int> HTBFmap<key_type,value_type>::fullinsert(key_type key,value_type value)
{
    vector<int> v;
    int num_lookup=0;
    //update value if exist
    if ((key==victim_key) && (victim_flag)) {
        victim_value=value;
        v.push_back(victim_value); 
        v.push_back(-1); 
        v.push_back(-1); 
        v.push_back(-1); 
        v.push_back(0); 
        return v;
    }
    int z=myhash<key_type>(key,100,cbf_size);
    for (int i = 0;  i <K;  i++) {
        //se CBF(key) == 0 continue
        if (cbf_array[i][z].check(key)==0) {
            continue;
        }
        int p = myhash<key_type>(key,i,m);
        num_lookup++;
        for (int ii = 0;  ii <b;  ii++)
            if ((present_table[i][ii][p]) && (table[i][ii][p].first== key)) {
                table[i][ii][p].second=value;
                v.push_back(table[i][ii][p].second); 
                v.push_back(i); 
                v.push_back(ii); 
                v.push_back(p); 
                v.push_back(num_lookup); 
                return v;
            }
    }

    // try cuckoo
    for (int t = 0;  t <= tmax;  t++) {

        // search for empty places
        for (int i = 0;  i <K;  i++){
            int p = myhash<key_type>(key,i,m);
            num_lookup++;
            for (int ii = 0;  ii <b;  ii++)
                if (!present_table[i][ii][p]) {  //insert in an empty place
                    present_table[i][ii][p] = true;
                    table[i][ii][p]={key,value};
                    num_item++;
                    v.push_back(table[i][ii][p].second); 
                    v.push_back(i); 
                    v.push_back(ii); 
                    v.push_back(p); 
                    v.push_back(num_lookup); 
                    
                    //inserisci key nel CBF i
                    int z=myhash<key_type>(key,100,cbf_size);
                    cbf_array[i][z].insert(key);
                    
                    return v;
                }
        }

        // finally play the cuckoo;
        int j = rand() % K;
        int jj = rand() % b;
        int p = myhash<key_type>(key,j,m);
        key_type new_key = table[j][jj][p].first;
        value_type new_value = table[j][jj][p].second;
        table[j][jj][p]={key,value};

        //rimuovi new_key dal CBF j
        int z=myhash<key_type>(new_key,100,cbf_size);
        cbf_array[j][z].erase(new_key);

        //inserisci key in CBF j
        z=myhash<key_type>(key,100,cbf_size);
        cbf_array[j][z].insert(key);

        key=new_key;
        value=new_value;
    }
    verprintf("insertion failed\n");
    printf("HTBFmap:: insertion failed\n");
    //if (verbose==1) cout << "key:<" << key.first <<","<< key.second <<">" <<endl;
    //if (verbose==1) cout << "value: " << value <<endl;

    victim_flag=true;
    victim_key=key;
    victim_value=value;
    v.push_back(victim_value); 
    v.push_back(-1); 
    v.push_back(-1); 
    v.push_back(-1); 
    v.push_back(tmax); 
    return v;
}
 
/*
 * Full Query
 */
template <typename key_type, typename value_type>
vector<int> HTBFmap<key_type,value_type>::fullquery(key_type key)
{
    vector<int> v;
    int num_lookup=0;
    if ((key==victim_key) && (victim_flag)) 
    {
        v.push_back(victim_value); 
        v.push_back(-1); 
        v.push_back(-1); 
        v.push_back(-1); 
        v.push_back(0); 
        //return std::make_tuple(victim_value,-1,-1,-1,0);
        return v;
    }
    int z=myhash<key_type>(key,100,cbf_size);
    for (int i = 0;  i <K;  i++) {
        //se CBF(key) == 0 continue
        if (cbf_array[i][z].check(key)==0) {
            continue;
        }
        num_lookup++;
        for (int ii = 0;  ii <b;  ii++){
            int p = myhash<key_type>(key,i,m);
            //verprintf("query item in table[%d][%d] for p=%d and f=%d\n",p,jj,p,fingerprint);
            //verprintf("result is: %d\n",table[p][jj]);
            if ((present_table[i][ii][p]) &&  (table[i][ii][p].first== key)) {
                    v.push_back(table[i][ii][p].second); 
                    v.push_back(i); 
                    v.push_back(ii); 
                    v.push_back(p); 
                    v.push_back(num_lookup); 
                    //return make_tuple(table[i][ii][p].second,i,ii,p,num_lookup);
                    return v;
            }
        }
    } 
    v.push_back(victim_value); 
    v.push_back(-1); 
    v.push_back(-1); 
    v.push_back(-1); 
    v.push_back(num_lookup); 
    //return std::make_tuple(victim_value,-1,-1,-1,num_lookup);
    return v;
}


/*
 * Get key from index
 */
template <typename key_type, typename value_type>
key_type  HTBFmap<key_type,value_type>::get_key(int i, int ii, int p)
{
    if (present_table[i][ii][p])
        return table[i][ii][p].first;
    else
        return victim_key;
}

/*
 * Count
 */
template <typename key_type, typename value_type>
int HTBFmap<key_type,value_type>::count(key_type key)
{
    if ((key==victim_key) && (victim_flag)) {
        verprintf("match item in victim cache \n");
        return 1;
    }
    verprintf("query item in HT \n");
    for (int i = 0;  i <K;  i++) {
        for (int ii = 0;  ii <b;  ii++){
            int p = myhash<key_type>(key,i,m);
            verprintf("query item in table[%d][%d] for p=%d\n",i,ii,p);
            if ((present_table[i][ii][p]) &&  (table[i][ii][p].first== key)) {
                return 1;
            }
        }
    }
    return 0;
}


template <typename key_type, typename value_type>
bool HTBFmap<key_type,value_type>::remove(key_type key) {
    if ((key==victim_key) && (victim_flag)){
            victim_flag=false;
            return true;
    }
    for (int i = 0;  i <K;  i++)
        for (int ii = 0;  ii <b;  ii++){
            int p = myhash<key_type>(key,i,m);
            if ((present_table[i][ii][p]) &&  (table[i][ii][p].first== key)) {
                //printf("remove key %ld from [%d][%d]\n",key,i,ii);
                present_table[i][ii][p] = false;
                num_item--;
                return true;
            }
    }
return false;
}
