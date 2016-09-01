#!/usr/bin/python 

import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import json
import operator


if len(sys.argv) != 3:
    print "usage: test.py <out_file> <key_type> \nwhere:\nkey_type: 1[ip src]; 2[ip src | ip dst]; 3 [socket 5.tuple]"
    sys.exit()

dic = {}
points = {}

#path of the trace file
out_file = sys.argv[1]

#1 ip src, 2 ip src - ip dst, 3 5tuple
key_type = sys.argv[2]

#flow timeout in seconds 
#timeout =  sys.argv[3]


input_traces = {   
       "campus": "../campus.5.txt",
#       "wand" : "../wand.5.txt",
#       "caida": "../caida.5.2.txt",
}

graphs_data = {}

out_file = open(out_file, "w")

packets = {}

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

        if key_type == "1":
            k ="%s"%(fields[1])
            graph_title = "flow key ip.src"  
        elif key_type == "2":
            k ="%s %s"%(fields[1], fields[2])
            graph_title = "flow key ip.src,ip.dst"
        elif key_type == "3":
            k ="%s %s %s %s %s"%(fields[1], fields[2], fields[3], fields[4], fields[5])
            graph_title = "flow key socket 5-tuple"
        else:
            print "unknown key type: " + key_type
            sys.exit()

        packets[int(float(fields[0]) * float(1000000))] = k

	if count.has_key(k):
	    count[k] = count[k] +1
        else: 
            count[k] = 1

    count_list_k = []
    count_list_v = []
    count_list_accv = []

    i =1
    tot = 0
    for k in sorted(count, key=count.get, reverse=True):
	#print k, count[k]
        i = i+1
        tot = tot + count[k]
        if (i % 100) == 0:
            count_list_k.append(i)
            count_list_v.append([count[k]])
        #if i == 100:
        #    break

    i =1
    acc = 0 
    for k in sorted(count, key=count.get, reverse=True):
        i = i+1
        #acc = float(acc + count[k])/float(tot)
        acc = float(acc + count[k])
        if (i % 100) == 0:
            count_list_accv.append([100*acc/tot])
        #if i == 100:
        #    break
         

print "peparing graph img file for the following measures"

graphs_traces = {}

markers = {
    1:"D",
    2:"s",
    3:"x",
}

colors = {
    1:"r",
    2:"g",
    3:"b",
}

fig, ax1 = plt.subplots()

x = np.array(count_list_k)
#y = np.array(count_list_v)
y = np.array(count_list_accv)
#plt.semilogy(x, y, linestyle='-', linewidth=1, marker=markers[i], color=colors[i], markersize=7) 
#y = np.array(count_list_accv)
#plt.semilogy(x, y, linestyle='-', linewidth=1, marker=markers[1], color=colors[1], markersize=7) 
ax1.plot(x, y, linestyle='-', linewidth=1, marker=markers[1], color=colors[1], markersize=7) 
#ax2 = ax1.twinx()
#y2 = np.array(count_list_accv)
#ax2.plot(x, y2, linestyle='-', linewidth=1, marker=markers[1], color=colors[1], markersize=7) 
#plt.xlabel('Time (s)')
#plt.ylabel('Number of active flows')
plt.title(graph_title)
plt.grid(True)
plt.savefig(out_file, format='pdf')
