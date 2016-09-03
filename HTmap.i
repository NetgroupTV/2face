/* HTmap.i */

%module HTmap
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "HTmap.hpp"
%}
%include <stdint.i>
%include <std_vector.i>
%include "std_string.i"

int myhash(int64_t key, int i, int s);
%include "HTmap.hpp"
%template(HTcuckoo) HTmap<int,int>;
%template(HTstring) HTmap<string,int>;
%template(IntVector) vector<int>;
