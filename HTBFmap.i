/* HTBFmap.i */

%module HTBFmap
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "HTBFmap.hpp"
%}
%include <stdint.i>
%include <std_vector.i>
%include "std_string.i"

%include "HTBFmap.hpp"
%template(myhashstring) myhash<string>;
%template(HTBFint) HTBFmap<int,int>;
%template(HTBFstring) HTBFmap<string,int>;
%template(IntVector) vector<int>;
