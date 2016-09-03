
py_cbf: 
	        swig -c++  -python CBF.i
		g++ -std=c++11 -fPIC -c -I/usr/include/python2.7 CBF.cpp CBF_wrap.cxx city.cpp
		g++ -shared -lpython2.7 *.o -o _cbf.so

py_cuckoo: 
	        swig -c++  -python HTmap.i
		g++  -std=c++11 -fPIC -c -I/usr/include/python2.7 HTmap_wrap.cxx city.cpp
		g++ -shared -lpython2.7 *.o -o _HTmap.so

#http://www.swig.org/Doc1.3/SWIGPlus.html#SWIGPlus_nn30

