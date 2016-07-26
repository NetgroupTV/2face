#include <iostream>
#include "HTmap.hpp"
#include "CBF.hpp"
using namespace std;

int main() {
    cout << "Hello, World!" << endl;

    int hbf=4;
    int sbf=32;
    // num_hash, num_buckets
    CBF cbf1(hbf,sbf);
    printf("***:BF:\n");
    printf("***:Num hash: %d\n",hbf);
    printf("***:Buckets: %d\n",sbf);
    printf("***:Total size (bits): %d\n",sbf);
    printf("***:---------------------------\n");

    // create the table;
    int num_way=4;
    int num_buckets=1;
    int ht_size=1024;
    HTmap<int64_t,int> cuckoo(num_way,num_buckets,ht_size,1000);
    printf("\n***Cuckoo table \n");
    printf("***:way: %d\n",num_way);
    printf("***:num_buckets: %d\n",num_buckets);
    printf("***:Total table size: %d\n",cuckoo.get_size());
    printf("***:---------------------------\n");

    // cuckoo.clear();
    //cbf1.clear();

    // insert in cbf
    //cbf1.insert(key);

    // insert in cuckoo HTmap
    //if(!(cuckoo[key]=line))
    //{
    //    verprintf(" Table full (key: %u)\n",key);
    //}

    //query:
    // bool flagc = (cuckoo.count(key)>0);
    //if (cbf1.check(key))

    return 0;
}