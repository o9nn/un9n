// Copyright Epic Games, Inc. All Rights Reserved.
// SWIG interface file for DNA Python bindings

%module(directors="1") dna

%{
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Include all required headers
#include <pma/MemoryResource.h>
#include <pma/ScopedPtr.h>
#include <pma/PolyAllocator.h>
#include <status/Status.h>
#include <status/StatusCode.h>
#include <trio/Stream.h>
#include <trio/streams/FileStream.h>
#include <trio/streams/MemoryMappedFileStream.h>
#include <trio/streams/MemoryStream.h>
#include <trust/ArrayView.h>

// DNA library headers
#include "dna/DataLayer.h"
#include "dna/types/Aliases.h"
#include "dna/types/ArrayView.h"
#include "dna/types/StringView.h"
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

// Ignore the problematic using declarations and namespaces
%ignore pma;
%ignore trio;
%ignore sc;
%ignore trust;

// Ignore specific types that cause issues
%ignore dna::BoundedIOStream;
%ignore dna::FileStream;
%ignore dna::MemoryMappedFileStream;
%ignore dna::MemoryStream;
%ignore dna::Status;
%ignore dna::ArrayView;
%ignore dna::ConstArrayView;

// Exception handling
%exception {
    try {
        $action
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        SWIG_fail;
    }
}

// Only include the DataLayer enum which is safe to wrap
%include "dna/DataLayer.h"
