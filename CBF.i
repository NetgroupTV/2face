/* CBF.i */
%module cbf
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "CBF.cpp"
%}
%include <stdint.i>
%include "CBF.hpp"
%include "std_string.i"

%template(CBFint) CBF<int>;
%template(CBFstring) CBF<string>;
