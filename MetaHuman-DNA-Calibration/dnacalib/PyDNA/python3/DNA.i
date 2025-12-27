// Copyright Epic Games, Inc. All Rights Reserved.
// SWIG interface file for DNA Python bindings

%module(directors="1") dna

%{
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// DNA library headers
#include "dna/DNA.h"
#include "dna/DataLayer.h"
#include "dna/BinaryStreamReader.h"
#include "dna/BinaryStreamWriter.h"
#include "dna/JSONStreamReader.h"
#include "dna/JSONStreamWriter.h"
#include "dna/StreamReader.h"
#include "dna/StreamWriter.h"
#include "dna/types/Aliases.h"
%}

// Include standard SWIG typemaps
%include <std_string.i>
%include <std_vector.i>
%include <stdint.i>
%include <std_shared_ptr.i>

// Define vector templates
%template(StringVector) std::vector<std::string>;
%template(FloatVector) std::vector<float>;
%template(IntVector) std::vector<int>;
%template(UInt16Vector) std::vector<uint16_t>;
%template(UInt32Vector) std::vector<uint32_t>;

// Exception handling
%exception {
    try {
        $action
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        SWIG_fail;
    }
}

// Forward declarations
namespace dna {
    class Reader;
    class Writer;
    class StreamReader;
    class StreamWriter;
    class BinaryStreamReader;
    class BinaryStreamWriter;
    class JSONStreamReader;
    class JSONStreamWriter;
}

// Include the main DNA headers for SWIG processing
%include "dna/types/Aliases.h"
%include "dna/DataLayer.h"
