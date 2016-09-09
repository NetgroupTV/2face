#!/usr/bin/python 

import cbf
import HTmap
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
# D: 2face


class __cache:
    self.mem = []
    self.size = 0
    self.read_count = 0
    self.write_count = 0

    def __init__(self, size):
        self.size = size
        for i in range(0,size):
            self.mem.append(None,0)

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
    self.HT = None
    self.memory_access_count = 0

    def __init__(self, ht_size, number_of_hash_tables):
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.HT.clear()
        print "type A FDB instantiated"
    
    def count(self, key):
        if self.HT.count(key):
            ret = self.HT.fullquery(key)
            #ret is a vector of 5 integers
            #ret[0]=value; ret[1]=ht idx; ret[2]=bucket; ret[3]=line idx; ret[4]=num of mem access
            new_val = ret[0] + 1
            self.HTinsert(key, new_val) 
            #assuming that when I read I also get the location address of the element in HT. Thus the value update counts 1
            self.memory_access_count += ret[4]+1 
            return new_val
        else:
            ret = self.HT.fullinsert(key, 1)
            self.memory_access_count += ret[4]
            return 1

    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()


class memB:
    self.HT = None
    self.cache = None
    self.memory_access_count = 0

    def __init__(self, ht_size, cache_size, number_of_hash_tables):
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.cache = __cache(cache_size) 
        self.HT.clear()

    def count(self, key):
        #check if key in cache
        cache_ret = self.cache.query(key)
        
        if cache_ret[0] == "miss":
            #check if key in HT
            if self.HT.count(key):
                ret = self.HT.fullquery(key)
                self.memory_access_count += ret[4]
                new_val = ret[0]+1
                self.cache.insert(key, new_val)
                return new_val 
            else:
                self.cache.insert(key, 1)
                return 1

        elif cache_ret[0] == "conflict":
            #insert old cache element in HT
            ret2 = self.HT.fullinsert(cache_ret[1], cache_ret[2])
            self.memory_access_count += ret2[4]

            #check if key in HT
            if self.HT.count(key):
                #YES -> move new element in cache
                ht_ret = self.HT.fullquery(key)
                self.memory_access_count += ht_ret[4]
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


class memC:
    self.HT = None
    self.BF = None 
    self.memory_access_count = 0
    self.bf_size =

    def __init__(self, ht_size, bf_size, number_of_hash_tables):
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.BFarray = __BFarray(bf_size, number_of_hash_tables)
        self.HT.clear()
        self.bf_size = bf_size

    def count(self, key):
        #compute hash(key)
        h = HTmap.myhashstring(key, cache_hash_idx, self.bf_size)

        #bf_ret = (a,b,c,di, ...., numb_of_ht) a=0 if not in first HT, 1 otherwise, etc.... 
        bf_ret = self.BFarray.query(h, key) 

        found = False
        for idx, val in enumerate(bf_ret):
            if val:
                ht_ret = self.HT.direct_query(key, idx)
                self.memory_access_count += 1

                if ht_ret[0]:
                    found = True
                    new_val = ht_ret[1]+1
                    self.HT.direct_insert(key, idx, new_val)
                    return new_val

        if not found:
            ht_ret = self.HT.fullinsert(key, 1)
            self.memory_access_count += ret2[4]
            return 1


    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()
        self.BFarray.clear()

    def memory_report(self): 
        pass



input_traces = {   
       "campus": "../campus.5.txt",
#       "wand" : "../wand.5.txt",
#       "caida": "../caida.5.2.txt",
}




for tname,tpath in input_traces.iteritems():
    print "opening %s in file %s"%(tname, tpath)
    f = open(tpath, "r")
    packets = {}
    count = {}

    while True:
        l = f.readline()
        if not l:
            break
        fields = l.split(" ")
        k ="%s %s %s %s %s"%(fields[1], fields[2], fields[3], fields[4], fields[5])

    
