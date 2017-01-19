#FLAGS=-D_DEBUG

all: py_cbf py_cuckoo py_cuckooBF


py_cbf: 
	        swig -c++  -python CBF.i
		g++ -std=c++11 -fPIC -c -I/usr/include/python2.7/ $(FLAGS) CBF.cpp CBF_wrap.cxx city.cpp
		g++ -shared -lpython2.7 city.o CBF_wrap.o -o _cbf.so

py_cuckoo: 
	        swig -c++  -python HTmap.i
		g++  -std=c++11 -fPIC -c -I/usr/include/python2.7 $(FLAGS) HTmap_wrap.cxx city.cpp
		g++ -shared -lpython2.7 city.o HTmap_wrap.o -o _HTmap.so

py_cuckooBF: 
	        swig -c++  -python HTBFmap.i
		g++  -std=c++11 -fPIC -c -I/usr/include/python2.7 $(FLAGS) HTBFmap_wrap.cxx city.cpp CBF.cpp
		g++ -shared -lpython2.7 city.o CBF.o CBF_wrap.o HTBFmap_wrap.o -o _HTBFmap.so

#http://www.swig.org/Doc1.3/SWIGPlus.html#SWIGPlus_nn30

