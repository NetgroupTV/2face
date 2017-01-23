#include "utils.h"
#include <tuple>
#include <vector>
#include <iostream>
#include <cstdint> // include this header for uint64_t
#include <cstring>

using namespace std;

//int tot_movements=0;
//int verbose=0;

template <typename key_type, typename value_type> class HTmap;  // forward declare
template <typename key_type, typename value_type> class HTIterator; // forward declare

template <typename key_type, typename value_type> class HTmap {
		bool        ***present_table;    //  present flag memory
		pair<key_type,value_type>  ***table;      // entries stored in the HT

		int m;                         // size of a table
		int b;     	               // number of slots in a bucket
		int K;     	               // number of way
		int num_item;                  // number of inserted item
		int tmax;
		bool victim_flag;
		key_type victim_key;
		value_type victim_value;

        public:
                //HTmap();
                //HTmap(int way, int buckets, int hsize);
                HTmap(int way, int buckets, int hsize,int t);
                ~HTmap();
                void clear();
                //void expand();
                bool insert(key_type key,value_type value);
                std::vector<int> fullinsert(key_type key,value_type value);
                bool direct_insert(key_type key,value_type value,int i, int ii, bool aa);
                std::vector<int>  direct_query(key_type key,int i);

                //LHS operator[]
                value_type& operator[](key_type key);
                //RHS operator[]
                const value_type operator[](key_type key) const  {return HTmap::query(key); }
                value_type query(key_type key);
                // return the value, the position (i,ii,p) and the number of accesses
                //tuple<value_type,int,int,int,int> fullquery(key_type key);
                std::vector<int> fullquery(key_type key);
                key_type  get_key(int i, int ii, int p);
                int count(key_type key);
                bool remove(key_type key);
                bool erase(key_type key) {return HTmap::remove(key);}
                long unsigned int size() {return (long unsigned int) num_item;}
                int get_nitem() {return num_item;}
                int get_size() {return K*b*m;}


       // iterators:
       //http://web.stanford.edu/class/cs107l/handouts/04-Custom-Iterators.pdf
       //http://www.cs.northwestern.edu/~riesbeck/programming/c++/stl-iterator-define.html

       //use private data of HTIterator class
       friend class HTIterator<key_type,value_type>;
       typedef HTIterator<key_type,value_type> iterator;

       iterator begin() {

           if(get_nitem()==0) return iterator(*this,0,0,0);
           int x=0;
           int y=0;
           int z=0;
           while(!present_table[x][y][z]){
               z++;
               if (z==m)  {y++; z=0;}
               if (y==b)  {x++; y=0; z=0;}
               if (x==K)  {x=0; y=0; z=0;}
           }
           return iterator(*this,x,y,z);
       }

       iterator end() {
           if(get_nitem()==0) return iterator(*this,0,0,0);
           int x=K-1;
           int y=b-1;
           int z=m-1;
           while(!present_table[x][y][z]){
               z--;
               if (z==-1) {y--; z=m-1;}
               if (y==-1) {x--; y=b-1; z=m-1;}
               if (x==-1) {x=K-1; y=b-1; z=m-1;}
           }
           return iterator(*this,x,y,z);
       }
};

template <typename key_type, typename value_type>
class HTIterator{
       private:
       HTmap<key_type,value_type> & myHT;
       int x,y,z;

       public:
       HTIterator(HTmap<key_type,value_type> & _ht, int _x, int _y, int _z) //constructor
           :myHT(_ht),x(_x),y(_y),z(_z) {}

       ~HTIterator(){} //distructor
       bool operator==(const HTIterator& other) const {
           if ((x==other.x) && (y==other.y) && (z==other.z))
               return true;
            return false;
       }
       bool operator!=(const HTIterator& other) const {
           if ((x==other.x) && (y==other.y) && (z==other.z))
               return false;
            return true;
       }

       pair<key_type,value_type> & operator*();
       HTIterator & operator++();
       HTIterator operator++(int);

};

template <typename key_type, typename value_type>
pair<key_type,value_type>& HTIterator<key_type,value_type>::operator*()
{
return myHT.table[x][y][z];
}

template <typename key_type, typename value_type>
HTIterator<key_type,value_type> & HTIterator<key_type,value_type>::operator++()
{
    if(myHT.get_nitem()==0) return *this;
    do
    {
        z++;
        if (z==myHT.m)  {y++; z=0;}
        if (y==myHT.b)  {x++; y=0; z=0;}
        if (x==myHT.K) {x=0; y=0; z=0;}
    }
    while(!myHT.present_table[x][y][z]);

    return *this;
}

template <typename key_type, typename value_type>
HTIterator<key_type,value_type> HTIterator<key_type,value_type>::operator++(int)
{
     HTIterator temp = *this ;
     this++;
     return temp;
}

/*
 * Constructor
 */


//inline uint64 CityHash64WithSeed(int64_t key, uint64_t seed)
//{
// return CityHash64WithSeed((const char *)&key,8,seed);
//}

//int rot(int64_t key, int i)
//{
//    return (key << i)| (key) >>(64-i);
//}
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


/*int myhash(int64_t key, int i, int s)
{
    uint64_t  val0;
    uint64_t  val1;
    uint64_t   val;
    int ss=s;

    val0=CityHash64WithSeed(key,3015) % ss;
    val1=CityHash64WithSeed(key,7793) % ss;
    if (val1==val0) {
        val1 = (val1 +1) % ss;
    }
    if (i==0) val=val0;
    if (i==1) val=val1;
    if (i>1)  val=CityHash64WithSeed(rot(key,i),2137*i) % ss;
    //if (i==1) ss=s/2;
    //val=std::hash<int>()(key+i);
    //val=(_mm_crc32_u64(i*378551,key));
    return (val %ss);
}*/


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

//int myhash(const std::pair<int,int> s, int i, int m)
//{
//    unsigned int h1 = hashg(s.first,i,m);
//    unsigned int h2 = hashg(s.second,i,m);
//    return ((h1 ^ ( h2 << 1 )) % m);
//}

template <typename key_type, typename value_type>
HTmap<key_type,value_type>::HTmap(int way, int buckets, int hsize,int t)
{
  tmax=t;
  m = hsize; // size of memory
  b = buckets;
  K = way;
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
  clear();
}

/*template <typename key_type, typename value_type>
HTmap<key_type,value_type>::HTmap(int way, int buckets, int hsize)
{
    HTmap<key_type,value_type>::HTmap(way,buckets,hsize,1000);
}


template <typename key_type, typename value_type>
HTmap<key_type,value_type>::HTmap()
{
    HTmap<key_type,value_type>::HTmap(2,4,1024*1024,1000);
}
*/
/*
 * Distructor
 */
template <typename key_type, typename value_type>
HTmap<key_type,value_type>::~HTmap()
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
}


/*
 * Clear
 */

template <typename key_type, typename value_type>
void HTmap<key_type,value_type>::clear()
{
	num_item=0;
        victim_flag=false;
        for (int i = 0;  i <K;  i++) {
            for (int ii = 0;  ii <b;  ii++)
                for (int iii = 0;  iii <m;  iii++){
                present_table[i][ii][iii]=false;
                }
    }
}


/*template <typename key_type, typename value_type>
HTmap<key_type,value_type>::expand()
{
  printf("expand table\n");
  //copy the content in a temp table
  bool*** temp_present_table = new bool**[K];
  pair<key_type,value_type>*** temp_table = new pair<key_type,value_type>**[K];
  for (int i = 0;  i <K;  i++) {
      temp_present_table[i] = new bool*[b];
      temp_table[i]= new pair<key_type,value_type>*[b];
      for (int ii = 0;  ii <b;  ii++){
          temp_present_table[i][ii] = new bool[m];
          temp_table[i][ii]= new pair<key_type,value_type>[m];
          for (int iii = 0;  iii <m;  iii++){
          temp_present_table[i][ii][iii] = present_table[i][ii][iii];
          temp_table[i][ii][iii]= table[i][ii][iii];
          }
      }
  }
  //delete the old table
  HTmap<key_type,value_type>::~HTmap();
  //create a new table
  HTmap<key_type,value_type>::HTmap(K,b,2*m);
  clear();
  //copy the content in the new table
  for (int i = 0;  i <K;  i++) {
      for (int ii = 0;  ii <b;  ii++){
          for (int iii = 0;  iii <m;  iii++){
          if (temp_present_table[i][ii][iii])
          HTmap<key_type,value_type>::insert(temp_table[i][ii][iii]= table[i][ii][iii]);
          }
      }
  }
}
*/

/*
 * Insert
 */
template <typename key_type, typename value_type>
bool HTmap<key_type,value_type>::insert(key_type key,value_type value)
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
        key=new_key;
        value=new_value;
    }
    verprintf("insertion failed\n");
    //if (verbose==1) cout << "key:<" << key.first <<","<< key.second <<">" <<endl;
    //if (verbose==1) cout << "value: " << value <<endl;

    victim_flag=true;
    victim_key=key;
    victim_value=value;
    return false;
}

/*
 * Direct Insert
 */
template <typename key_type, typename value_type>
bool HTmap<key_type,value_type>::direct_insert(key_type key,value_type value,int i, int ii, bool auto_bucket)
{
    //return false if the key is in the victim cache
    if ((key==victim_key) && (victim_flag)) {
        printf("ERROR: the key is in the victim cache\n");
        exit(1);
        return false;
    }

    int p = myhash<key_type>(key,i,m);


    if (auto_bucket) {
        //return false if the place is not free
        if (present_table[i][ii][p]) {
            printf("ERROR: the place [%d][%d] is not free \n",i,ii);
            return false;
        }

        present_table[i][ii][p] = true;
        table[i][ii][p]={key,value};
        num_item++;
        return true;
    }
    else {
        for (int jj = 0;  jj <b;  jj++) {
            if ((present_table[i][jj][p]) && (table[i][jj][p].first== key)){
                table[i][jj][p].second=value;
                return true;
            }
        }
        printf("ERROR:item not present\n");
        return false;
    }
}


//LHS operator[]
template <typename key_type, typename value_type>
value_type& HTmap<key_type,value_type>::operator[](key_type key) {


    if (HTmap<key_type,value_type>::count(key)==0){ //insert if not exist
        HTmap<key_type,value_type>::insert(key,victim_value);
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
 * Direct Query
 */

template <typename key_type, typename value_type>
std::vector<int> HTmap<key_type,value_type>::direct_query(key_type key,int i)
{
    std::vector<int> v;
    if ((key==victim_key) && (victim_flag)) {
        v.push_back(1);
        v.push_back(victim_value); 
        return v;
    }

    for (int ii = 0;  ii <b;  ii++){
        int p = myhash<key_type>(key,i,m);
        //verprintf("query item in table[%d][%d] for p=%d and f=%d\n",p,jj,p,fingerprint);
        //verprintf("result is: %d\n",table[p][jj]);
        //printf("query item in table[%d][%d] for p=%d\n",i,ii,p);
        //std::cout << table[i][ii][p].first << std::endl;
        //printf("result is: %d\n",table[i][ii][p].second);
        if ((present_table[i][ii][p]) &&  (table[i][ii][p].first== key)) {
            //printf("match\n");
            v.push_back(1);
            v.push_back(table[i][ii][p].second); 
            return v;
        }
    }

    v.push_back(0);
    v.push_back(victim_value);
    return v;
}

/*
 * Query
 */
template <typename key_type, typename value_type>
value_type HTmap<key_type,value_type>::query(key_type key)
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
vector<int> HTmap<key_type,value_type>::fullinsert(key_type key,value_type value)
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
    for (int i = 0;  i <K;  i++){
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

    // check if we need to grow the map
    //if(90*HTmap<key_type,value_type>::get_size()<100*HTmap<key_type,value_type>::get_nitem()) {
    //    HTmap<key_type,value_type>::expand();
    //}

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
        key=new_key;
        value=new_value;
    }
    verprintf("insertion failed\n");
    printf("HTmap:: insertion failed\n");
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
vector<int> HTmap<key_type,value_type>::fullquery(key_type key)
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
    for (int i = 0;  i <K;  i++) {
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
key_type  HTmap<key_type,value_type>::get_key(int i, int ii, int p)
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
int HTmap<key_type,value_type>::count(key_type key)
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
bool HTmap<key_type,value_type>::remove(key_type key) {
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
