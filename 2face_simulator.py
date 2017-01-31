#!/usr/bin/python 

#import cbf
import HTmap
import HTBFmap
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import json
import operator


cache_hash_idx = 100
# simulatore che conta numero accessi a memoria esterna
# al variare delle tracce di ingresso e al variare delle 
# strutture dati usate: 
# A: cuckoo_4_2
# B: cache + cuckoo_4_2 
# C: CBF + cuckoo_4_2
# D: cache + CBF + cuckoo_4_2
# E: 2face


class Cache:
    def __init__(self, size):
        self.mem = []
        self.size = size
        self.read_count = 0
        self.write_count = 0

        for i in range(0,size):
            self.mem.append((None,0))

    def query(self, key):
        self.read_count += 1
        p = HTmap.myhashstring(key, cache_hash_idx, self.size)
        element = self.mem[p]
        if not element[0]:
        #cache miss
            return ("miss", None, 0)

        elif element[0] != key:
        #cache conflict
            return ("conflict", element[0], element[1])

        else:
        #cache hit
            return ("hit", element[0], element[1])

    def insert(self, key, value):
        self.write_count += 1
        p = HTmap.myhashstring(key,cache_hash_idx,self.size)
        self.mem[p]=(key, value)


class memA:
    def __init__(self, ht_size, number_of_hash_tables):
        self.memory_access_count = 0
        self.memory_read_count = 0
        self.number_of_hash_tables=number_of_hash_tables
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.HT.clear()

    def count(self, key):
        if self.HT.count(key):
            ret = self.HT.fullquery(key)
            #ret is a vector of 5 integers
            #ret[0]=value; ret[1]=ht idx; ret[2]=bucket; ret[3]=line idx; ret[4]=num of mem access
            new_val = ret[0] + 1
            if (self.HT.insert(key, new_val)==False):
                print 'HT full!!!'
            #assuming that when I read I also get the location address of the element in HT. Thus the value update counts 1
            self.memory_access_count += ret[4]+1 
            self.memory_read_count += ret[4]
            return new_val
        else:
            #self.memory_read_count +=self.number_of_hash_tables 
            ret = self.HT.fullinsert(key, 1)
            if (ret[4]==1000):
                print 'HT full!!!'
            self.memory_access_count += ret[4]
            #print 'memA:', self.memory_access_count
            #print 'memA:', self.memory_read_count
            return 1

    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()

    def mem_report(self):
        print "Number of items in HT: " + str(self.HT.get_nitem())
        print "HT size: " + str(self.HT.get_size())
        print "HT load factor: ",  100*self.HT.get_nitem()/self.HT.get_size(), "%"
        print "memA: Number of memory accesses: " + str(self.memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)

class memB:
    def __init__(self, ht_size, cache_size, number_of_hash_tables):
        self.memory_access_count = 0
        self.memory_read_count = 0
        self.number_of_hash_tables=number_of_hash_tables
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.cache = Cache(cache_size) 
        self.HT.clear()

    def count(self, key):
        #check if key in cache
        cache_ret = self.cache.query(key)
        
        if cache_ret[0] == "miss":
            #check if key in HT
            if self.HT.count(key):
                ret = self.HT.fullquery(key)
                self.memory_access_count += ret[4]
                self.memory_read_count += ret[4]
                new_val = ret[0]+1
                self.cache.insert(key, new_val)
                return new_val 
            else:
                ret = self.HT.fullquery(key)
                self.memory_read_count += ret[4]
                self.cache.insert(key, 1)
                return 1

        elif cache_ret[0] == "conflict":
            #insert old cache element in HT
            ret2 = self.HT.fullinsert(cache_ret[1], cache_ret[2])
            if (ret2[4]==1000):
                print 'HT full!!!'
            self.memory_access_count += ret2[4]

            #check if key in HT
            if self.HT.count(key):
                #YES -> move new element in cache
                ht_ret = self.HT.fullquery(key)
                self.memory_access_count += ht_ret[4]
                self.memory_read_count += ht_ret[4]
                new_val = ht_ret[0]+1
                self.cache.insert(key, new_val)
                return new_val
            else:
                #NO -> insert (key, 1) in cache
                self.cache.insert(key, 1)
                return 1
        
        else: #cache hit
            new_val = cache_ret[2]+1
            self.cache.insert(key, new_val)
            return new_val

    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()

    def mem_report(self):
        print "Number of items in HT: " + str(self.HT.get_nitem())
        print "HT size: " + str(self.HT.get_size())
        print "mem ("+str(ratio) + "%): Number of memory accesses: " + str(self.memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)
        #print "cache read: %d write: %d"%(self.cache.read_count, self.cache.write_count)

#class BFarray:
#    def __init__(self, bf_size, number_of_hash_tables):
#        self.mem = []
#        self.read_count = 0
#        self.write_count = 0
#        self.bf_size = bf_size
#        self.number_of_hash_tables = number_of_hash_tables
#
#        for j in range(bf_size):
#            row = []
#            for i in range(number_of_hash_tables):
#                obj = cbf.CBFstring(4, 32)
#                row.append(obj)
#            self.mem.append(row)
#
#    def query(self, key):
#        h = HTmap.myhashstring(key, cache_hash_idx, self.bf_size)
#        row = self.mem[h]
#        res = []
#        for i in range(self.number_of_hash_tables):
#            self.read_count += 1
#            bf_obj = row[i]
#            res.append(bf_obj.check(key))
#
#        return res
#
#    def insert(self, key, idx):
#        h = HTmap.myhashstring(key, cache_hash_idx, self.bf_size)
#        row = self.mem[h]
#        self.write_count += 1
#        row[idx].insert(key)


class memC:

    def __init__(self, ht_size, bf_size, number_of_hash_tables):
        self.memory_access_count = 0
        self.memory_read_count   = 0 
        self.HT =  HTBFmap.HTBFstring(number_of_hash_tables, 2, ht_size, 1000,bf_size)
        #self.BFarray = BFarray(bf_size, number_of_hash_tables)
        self.HT.clear()
    
    def count(self, key):
        if self.HT.count(key):
            ret = self.HT.fullquery(key)
            #ret is a vector of 5 integers
            #ret[0]=value; ret[1]=ht idx; ret[2]=bucket; ret[3]=line idx; ret[4]=num of mem access
            new_val = ret[0] + 1
            self.HT.insert(key, new_val) 
            #assuming that when I read I also get the location address of the element in HT. Thus the value update counts 1
            self.memory_access_count += ret[4]+1 
            self.memory_read_count += ret[4]
            return new_val
        else:
            #count read access not pruned by BFs
            ret = self.HT.fullquery(key)
            #self.memory_read_count += ret[4]
            
            ret = self.HT.fullinsert(key, 1)
            if (ret[4]==1000):
                print 'HT full!!!'
            self.memory_access_count += ret[4]
            #print 'memC:', self.memory_access_count
            #print 'memC:', self.memory_read_count
            return 1

    #def count(self, key):
    #    #compute hash(key)

    #    #bf_ret = (a,b,c,di, ...., numb_of_ht) a=0 if not in first HT, 1 otherwise, etc.... 
    #    bf_ret = self.BFarray.query(key) 

    #    found = False
    #    for idx, val in enumerate(bf_ret):
    #        if val:
    #            ht_ret = self.HT.direct_query(key, idx)
    #            self.memory_access_count += 1

    #            if ht_ret[0]==1:
    #                print 'reciao'
    #                found = True
    #                new_val = ht_ret[1]+1
    #                self.HT.direct_insert(key, new_val, idx, 0, False)
    #                return new_val

    #    if not found:
    #        print 'ciao'
    #        ht_ret = self.HT.fullinsert(key, 1)
    #        self.memory_access_count += ht_ret[4]
    #        #self.BFarray.insert(key, ht_ret[1])
    #        return 1


    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()
        #self.BFarray.clear()

    def mem_report(self):
        #print "Number of items in HT: " + str(self.HTBF.get_nitem())
        #print "Number of memory accesses: " + str(self.memory_access_count)
        print "mem ("+str(ratio) + "%): Number of memory accesses: " + str(self.memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)
        #print "cache read: %d write: %d"%(self.BFarray.read_count, self.BFarray.write_count)


class memD:
    def __init__(self, ht_size, cache_size, number_of_hash_tables,ratio):
        self.memory_access_count = 0
        self.memory_read_count = 0
        self.number_of_hash_tables=number_of_hash_tables
        my_bf_size=((100-ratio)*cache_size)/100
        self.HT =  HTBFmap.HTBFstring(number_of_hash_tables, 2, ht_size, 1000,my_bf_size)
        my_cache_size=(ratio*cache_size)/100
        self.cache = Cache(my_cache_size) 
        self.HT.clear()

    def count(self, key):
        #check if key in cache
        cache_ret = self.cache.query(key)
        
        if cache_ret[0] == "miss":
            #check if key in HT
            if self.HT.count(key):
                ret = self.HT.fullquery(key)
                self.memory_access_count += ret[4]
                self.memory_read_count += ret[4]
                new_val = ret[0]+1
                self.cache.insert(key, new_val)
                return new_val 
            else:
                #count read access not pruned by BFs
                ret = self.HT.fullquery(key)
                self.memory_read_count += ret[4]
                self.cache.insert(key, 1)
                return 1

        elif cache_ret[0] == "conflict":
            #insert old cache element in HT
            ret2 = self.HT.fullinsert(cache_ret[1], cache_ret[2])
            if (ret2[4]==1000):
                print 'HT full!!!'
            self.memory_access_count += ret2[4]

            #check if key in HT
            if self.HT.count(key):
                #YES -> move new element in cache
                ht_ret = self.HT.fullquery(key)
                self.memory_access_count += ht_ret[4]
                self.memory_read_count += ht_ret[4]
                new_val = ht_ret[0]+1
                self.cache.insert(key, new_val)
                return new_val
            else:
                #NO -> insert (key, 1) in cache
                self.cache.insert(key, 1)
                return 1
        
        else: #cache hit
            new_val = cache_ret[2]+1
            self.cache.insert(key, new_val)
            return new_val

    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()

    def mem_report(self):
        #print "Number of items in HT: " + str(self.HT.get_nitem())
        #print "HT size: " + str(self.HT.get_size())
        print "mem ("+str(ratio) + "%): Number of memory accesses: " + str(self.memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)
        #print "cache read: %d write: %d"%(self.cache.read_count, self.cache.write_count)




input_traces = {   
#       "test": "test.txt",
    "campus": "campus.5.txt"
#       "wand" : "wand.5.txt",
#       "wand2" : "wand.10M.5.txt",
#       "caida": "caida.5.2.txt",
}


if len(sys.argv) != 4:
    print "usage: 2face_simulator.py <cache_size> <ht_size> <ratio>"
    sys.exit()

cache_size = int(sys.argv[1])
ht_size = int(sys.argv[2])
ratio = int(sys.argv[3])
print "With command line: 2face_simulator.py", sys.argv[1], sys.argv[2], sys.argv[3] 


#BF_SIZE = CACHE_SIZE
bf_size = cache_size

for tname,tpath in input_traces.iteritems():
    print "opening %s in file %s"%(tname, tpath)
    f = open(tpath, "r")
    packets = {}
    count = {}
    num_of_packets = 0

    testA = memA(ht_size, 4)
    #testB = memB(ht_size, cache_size, 4)
    #testC = memC(ht_size, bf_size, 4)
    
    if (ratio==0):
        testD = memC(ht_size,cache_size,4)
    elif (ratio==100):
        testD = memB(ht_size,cache_size,4)
    else:
        testD = memD(ht_size, cache_size, 4, ratio)

    while True:
        l = f.readline()
        if not l:
            break
        fields = l.split("\n")[0].split(" ")
        k ="%s %s %s %s %s"%(fields[1], fields[2], fields[3], fields[4], fields[5])
        num_of_packets += 1

        #print k 
        #print 
        a=testA.count(k)
        #b=testB.count(k)
        #c=testC.count(k)
        d=testD.count(k)
        #print b
        #print c
        if (a!=d):
            print "ERROR"
            print "HT size is", testC.HT.get_size()
            print "items in HT:", testC.HT.get_nitem()
            sys.exit()

    print "--------------------------"
    print "Trace " + tname
    print "Number of packets " + str(num_of_packets)
    print "cache ratio is:", cache_size/(0.0+testA.HT.get_size()) 
    print "TestA report:"
    testA.mem_report()

    #print "TestB report:"
    #testB.mem_report() 

    #print "TestC report:"
    #testC.mem_report() 
    
    print "TestD report:"
    testD.mem_report() 
    print "--------------------------"
sys.exit()

