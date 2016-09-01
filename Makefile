

py_cbf: 
	        swig -c++  -python CBF.i
		g++ -fno-rtti -I/usr/include/python2.7 -fPIC -c CBF.cpp CBF_wrap.cxx city.cpp
		g++ -shared -lpython2.7 *.o -o _cbf.so

#http://www.swig.org/Doc1.3/SWIGPlus.html#SWIGPlus_nn30

