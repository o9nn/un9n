// Copyright Epic Games, Inc. All Rights Reserved.
// SWIG interface file for DNACalib Python bindings

%module(directors="1") dnacalib

%{
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// DNACalib library headers
#include "dnacalib/DNACalib.h"
#include "dnacalib/Command.h"
#include "dnacalib/Defs.h"
#include "dnacalib/commands/CalculateMeshLowerLODsCommand.h"
#include "dnacalib/commands/ClearBlendShapesCommand.h"
#include "dnacalib/commands/CommandSequence.h"
#include "dnacalib/commands/ConditionalCommand.h"
#include "dnacalib/commands/PruneBlendShapeTargetsCommand.h"
#include "dnacalib/commands/RemoveAnimatedMapCommand.h"
#include "dnacalib/commands/RemoveBlendShapeCommand.h"
#include "dnacalib/commands/RemoveJointAnimationCommand.h"
#include "dnacalib/commands/RemoveJointCommand.h"
#include "dnacalib/commands/RemoveMeshCommand.h"
#include "dnacalib/commands/RenameAnimatedMapCommand.h"
#include "dnacalib/commands/RenameBlendShapeCommand.h"
#include "dnacalib/commands/RenameJointCommand.h"
#include "dnacalib/commands/RenameMeshCommand.h"
#include "dnacalib/commands/RotateCommand.h"
#include "dnacalib/commands/ScaleCommand.h"
#include "dnacalib/commands/SetBlendShapeTargetDeltasCommand.h"
#include "dnacalib/commands/SetLODsCommand.h"
#include "dnacalib/commands/SetNeutralJointTranslationsCommand.h"
#include "dnacalib/commands/SetNeutralJointRotationsCommand.h"
#include "dnacalib/commands/SetSkinWeightsCommand.h"
#include "dnacalib/commands/SetVertexPositionsCommand.h"
#include "dnacalib/commands/TranslateCommand.h"
#include "dnacalib/dna/DNACalibDNAReader.h"
#include "dnacalib/types/Aliases.h"
%}

// Include standard SWIG typemaps
%include <std_string.i>
%include <std_vector.i>
%include <stdint.i>
%include <std_shared_ptr.i>

// Import DNA module
%import "DNA.i"

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

// Forward declarations for dnacalib namespace
namespace dnacalib {
    class Command;
    class CommandSequence;
}

// Include the main DNACalib headers for SWIG processing
%include "dnacalib/Defs.h"
%include "dnacalib/Command.h"
%include "dnacalib/types/Aliases.h"
