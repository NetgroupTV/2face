#!/usr/bin/python 

#import cbf
import HTmap
import os
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
        self.tot_memory_access_count = 0
        self.number_of_hash_tables=number_of_hash_tables
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.HT.clear()

    def sample(self):
        tmp = self.memory_access_count
        self.tot_memory_access_count += self.memory_access_count
        self.memory_access_count = 0

        return (tmp, self.tot_memory_access_count)

    def query(self, key):
        ret = self.HT.fullquery(key)
        self.memory_access_count += ret[4]+1 
        self.memory_read_count += ret[4]
        return ret[0]


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
        print "memA: Total number of memory accesses: " + str(self.tot_memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)

class memB:
    def __init__(self, ht_size, cache_size, number_of_hash_tables):
        self.memory_access_count = 0
        self.memory_read_count = 0
        self.tot_memory_access_count = 0
        self.number_of_hash_tables=number_of_hash_tables
        self.HT =  HTmap.HTstring(number_of_hash_tables, 2, ht_size, 1000)
        self.cache = Cache(cache_size) 
        self.HT.clear()

    def sample(self):
        tmp = self.memory_access_count
        self.tot_memory_access_count += self.memory_access_count
        self.memory_access_count = 0

        return (tmp, self.tot_memory_access_count)

    def sync_cache_and_ht(self):
        for i in self.cache.mem:
            if i[0]:
                self.HT.fullinsert(i[0],i[1])


    def query(self, key):
        cache_ret = self.cache.query(key)
        if cache_ret[0] == "miss":
            ret = self.HT.fullquery(key)
            self.memory_access_count += ret[4]
            self.memory_read_count += ret[4]
            self.cache.insert(key, ret[0])
            return ret[0]
        
        elif cache_ret[0] == "conflict":
            #YES -> move new element in cache
            ht_ret = self.HT.fullquery(key)
            self.memory_access_count += ht_ret[4]
            self.memory_read_count += ht_ret[4]
            self.cache.insert(key, ht_ret[0])
            return ht_ret[0]
        else: #cache hit
            return cache_ret[2]

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
        print "memB: Total number of memory accesses: " + str(self.tot_memory_access_count)
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
        self.tot_memory_access_count = 0
        self.HT =  HTBFmap.HTBFstring(number_of_hash_tables, 2, ht_size, 1000,bf_size)
        #self.BFarray = BFarray(bf_size, number_of_hash_tables)
        self.HT.clear()

    def sync_cache_and_ht(self):
        return
    
    def sample(self):
        tmp = self.memory_access_count
        self.tot_memory_access_count += self.memory_access_count
        self.memory_access_count = 0

        return (tmp, self.tot_memory_access_count)

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

    def query(self, key):
        ret =self.HT.fullquery(key)
        self.memory_access_count += ret[4]+1 
        self.memory_read_count += ret[4]

        return ret[0]

    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()
        #self.BFarray.clear()

    def mem_report(self):
        #print "Number of items in HT: " + str(self.HTBF.get_nitem())
        #print "Number of memory accesses: " + str(self.memory_access_count)
        print "mem ("+str(ratio) + "%): Number of memory accesses: " + str(self.memory_access_count)
        print "mem ("+str(ratio) + "%): Total number of memory accesses: " + str(self.tot_memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)
        #print "cache read: %d write: %d"%(self.BFarray.read_count, self.BFarray.write_count)


class memD:
    def __init__(self, ht_size, cache_size, number_of_hash_tables,ratio):
        self.memory_access_count = 0
        self.memory_read_count = 0
        self.tot_memory_access_count = 0
        self.number_of_hash_tables=number_of_hash_tables
        my_bf_size=((100-ratio)*cache_size)/100
        self.HT =  HTBFmap.HTBFstring(number_of_hash_tables, 2, ht_size, 1000,my_bf_size)
        my_cache_size=(ratio*cache_size)/100
        self.cache = Cache(my_cache_size) 
        self.HT.clear()

    def sync_cache_and_ht(self):
        for i in self.cache.mem:
            if i[0]:
                self.HT.fullinsert(i[0],i[1])

    def sample(self):
        tmp = self.memory_access_count
        self.tot_memory_access_count += self.memory_access_count
        self.memory_access_count = 0

        return (tmp, self.tot_memory_access_count)


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

    def query(self, key):
        cache_ret = self.cache.query(key)
        
        if cache_ret[0] == "miss":
            #check if key in HT
            ret = self.HT.fullquery(key)
            self.memory_access_count += ret[4]
            self.memory_read_count += ret[4]
            self.cache.insert(key, ret[0])
            return ret[0] 

        elif cache_ret[0] == "conflict":
            ht_ret = self.HT.fullquery(key)
            self.memory_access_count += ht_ret[4]
            self.memory_read_count += ht_ret[4]
            self.cache.insert(key, ht_ret[0])
            return ht_ret[0]
    
        else: #cache hit
            return cache_ret[2]


    def clear(self):
        self.memory_access_count = 0
        self.HT.clear()

    def mem_report(self):
        #print "Number of items in HT: " + str(self.HT.get_nitem())
        #print "HT size: " + str(self.HT.get_size())
        print "mem ("+str(ratio) + "%): Number of memory accesses: " + str(self.memory_access_count)
        print "mem ("+str(ratio) + "%): Total number of memory accesses: " + str(self.tot_memory_access_count)
        #print "Number of read memory accesses: " + str(self.memory_read_count)
        #print "cache read: %d write: %d"%(self.cache.read_count, self.cache.write_count)


if len(sys.argv) != 8:
    print "usage: 2face_simulator.py <cache_size> <ht_load> <ratio> <trace_file> <key_type> <app_type> <sample_pkt_count>"
    print "(1) cache size if a % of the hash table size; (2) ratio between BF size and cache size in the hybrid solution"
    print "where key type can be: \n1: <src_ip>\n2: <dst_ip>\n3: <src_ip, dst_ip>\n4: 5 tuple"
    print "where app type can be: \n0: dynamic\n100: full pre loaded\nx\%: x\% pre loaded " 
    sys.exit()

cache_size_perc= float(sys.argv[1])
ht_load = int(sys.argv[2])
ratio = int(sys.argv[3])
trace_path = sys.argv[4] 
key_type = int(sys.argv[5])
app_type = int(sys.argv[6])
sample_period_pkts = int(sys.argv[7])
print "With command line: 2face_simulator.py"," ".join(sys.argv[1:])

bf_size = cache_size

tname = os.path.basename(trace_path).split(".")[0]
tpath = trace_path

print "opening %s in file %s"%(tname, tpath)
f = open(tpath, "r")
packets = {}
count = {}
num_of_packets = 1

first_line = f.readline()
n_list = first_line.split()
N = n_list(key_type) 
ht_size = (N*100) / (8 * ht_load) 

cache_size = (ht_size * 8 * cache_size_percent) / 100 

testA = memA(ht_size, 4)

if (ratio==0):
    testD = memC(ht_size,cache_size,4)
elif (ratio==100):
    testD = memB(ht_size,cache_size,4)
else:
    testD = memD(ht_size, cache_size, 4, ratio)

if app_type == 0:
    print "starting dynamic app"
elif app_type == 100:
    print "starting static 100% app"
    #load db
    while True:
        l = f.readline()
        if not l or l[0] == "#":
            break
        fields = l.split("\n")[0].split(" ")
        if key_type == 1:
            k = "%s"%(fields[1])
        elif key_type == 2:
            k = "%s"%(fields[2])
        elif key_type == 3:
            k = "%s %s"%(fields[1],fields[2])
        elif key_type == 4:
            k ="%s %s %s %s %s"%(fields[1], fields[2], fields[3], fields[4], fields[5])
        else:
            print("wrong key type")
            sys.exit(-1)

        num_of_packets += 1
        #faccio count cosi alla fine del while ho il DB pieno
        a=testA.count(k)
        d=testD.count(k)
    
        if (a!=d):
            print "ERROR in load"
            print a,d,k
            print "HT size is", testA.HT.get_size()
            print "items in HT:", testA.HT.get_nitem()
            sys.exit(-1)


    
    #make sure cahce and HT have the same values...
    testD.sync_cache_and_ht()

    print "processed %d packets. DB is now loaded with load factor %d"%(num_of_packets, 100*testA.HT.get_nitem()/testA.HT.get_size())
else:
    print "wrong app type"
    sys.exit(-1)



print "starting app"
f.seek(0)
num_of_packets=0
while True:

    if sample_period_pkts and num_of_packets % sample_period_pkts==0:
        testA.mem_report()
        testD.mem_report()
        retA = testA.sample()
        retD = testD.sample()

    l = f.readline()
    if not l:
        break
    fields = l.split("\n")[0].split(" ")
    if key_type == 1:
        k = "%s"%(fields[1])
    elif key_type == 2:
        k = "%s"%(fields[2])
    elif key_type == 3:
        k = "%s %s"%(fields[1],fields[2])
    elif key_type == 4:
        k ="%s %s %s %s %s"%(fields[1], fields[2], fields[3], fields[4], fields[5])
    else:
        print("wrong key type")
        sys.exit(-1)

    num_of_packets += 1

    if app_type == 0:
        a=testA.count(k)
        d=testD.count(k)
    elif app_type == 100:
        a=testA.query(k)
        d=testD.query(k)
    else:
        print "non so come e' possibile perche' ho controllato prima mah oiboh, non costava niente.."
        sys.exit(-1)

    if (a!=d):
        print "ERROR "
        print a,d,k
        print "HT size is", testA.HT.get_size()
        print "items in HT:", testA.HT.get_nitem()
        sys.exit(-1)


print "------------ FINE TRACCIA --------------"
print "Trace " + tname
print "Number of packets " + str(num_of_packets)
print "cache ratio is:", cache_size/(0.0+testA.HT.get_size()) 
print "TestA report:"
testA.mem_report() 
print "TestD report:"
testD.mem_report() 
print "-----------------------------------------"

sys.exit()

