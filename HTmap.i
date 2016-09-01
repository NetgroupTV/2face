/* HTmap.i */

%module HTmap
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "HTmap.hpp"
%}
%include <stdint.i>
%include <std_vector.i>

%include "HTmap.hpp"
%template(HTcuckoo) HTmap<int,int>;
%template(IntVector) vector<int>;
