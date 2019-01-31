#ifndef ROSE_Partitioner2_BasicTypes_H
#define ROSE_Partitioner2_BasicTypes_H

#include <boost/filesystem.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <string>
#include <vector>

// Define this as one if you want extra invariant checks that are quite expensive, or define as zero. This only makes a
// difference if NDEBUG and SAWYER_NDEBUG are both undefined--if either one of them are defined then no expensive (or
// inexpensive) checks are performed.
#define ROSE_PARTITIONER_EXPENSIVE_CHECKS 0

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

/** Level of precision for analysis. */
namespace Precision {
enum Level {                                            /**< Enum type for precision. */
    LOW,                                                /**< Low precision, but fast. This usually works reasonably well for
                                                         *   code generated by mainstream compilers. */
    HIGH                                                /**< High precision, but slow. This usually works better for
                                                         *   hand-written or obfuscated code. */
};
} // namespace

/** Flag whether to allow parallel edges in a graph. */
namespace AllowParallelEdges {
enum Type {                                             /**< Enum type for allowing parallel edges. */
    NO,                                                 /**< Don't allow parallel edges; use counts instead. */
    YES                                                 /**< Allow parallel edges, so each edge has a unit count. */
};
} // namespace

/** Partitioner control flow vertex types. */
enum VertexType {
    V_BASIC_BLOCK,                                      /**< A basic block or placeholder for a basic block. */
    V_UNDISCOVERED,                                     /**< The special "undiscovered" vertex. */
    V_INDETERMINATE,                                    /**< Special vertex destination for indeterminate edges. */
    V_NONEXISTING,                                      /**< Special vertex destination for non-existing basic blocks. */
    V_USER_DEFINED,                                     /**< User defined vertex. These vertices don't normally appear in the
                                                         *   global control flow graph but might appear in other kinds of
                                                         *   graphs that are closely related to a CFG, such as a paths graph. */
};

/** Partitioner control flow edge types. */
enum EdgeType {
    E_NORMAL,                                           /**< Normal control flow edge, nothing special. */
    E_FUNCTION_CALL,                                    /**< Edge is a function call. */
    E_FUNCTION_RETURN,                                  /**< Edge is a function return. Such edges represent the actual
                                                         *   return-to-caller and usually originate from a return instruction
                                                         *   (e.g., x86 @c RET, m68k @c RTS, etc.). */
    E_CALL_RETURN,                                      /**< Edge is a function return from the call site. Such edges are from
                                                         *   a caller basic block to (probably) the fall-through address of the
                                                         *   call and don't actually exist directly in the specimen.  The
                                                         *   represent the fact that the called function eventually returns
                                                         *   even if the instructions for the called function are not available
                                                         *   to analyze. */
    E_FUNCTION_XFER,                                    /**< Edge is a function call transfer. A function call transfer is
                                                         *   similar to @ref E_FUNCTION_CALL except the entire call frame is
                                                         *   transferred to the target function and this function is no longer
                                                         *   considered part of the call stack; a return from the target
                                                         *   function will skip over this function. Function call transfers
                                                         *   most often occur as the edge leaving a thunk. */
    E_USER_DEFINED,                                     /**< User defined edge.  These edges don't normally appear in the
                                                         *   global control flow graph but might appear in other kinds of
                                                         *   graphs that are closely related to a CFG, such as a paths graph. */
};

/** How sure are we of something. */
enum Confidence {
    ASSUMED,                                            /**< The value is an assumption without any proof. */
    PROVED,                                             /**< The value was somehow proved. */
};

/** Organization of semantic memory. */
enum SemanticMemoryParadigm {
    LIST_BASED_MEMORY,                                  /**< Precise but slow. */
    MAP_BASED_MEMORY                                    /**< Fast but not precise. */
};

/** Settings that control building the AST.
 *
 *  The runtime descriptions and command-line parser for these switches can be obtained from @ref
 *  astConstructionSwitches. */
struct AstConstructionSettings {
    /** Whether to allow an empty global block.
     *
     *  If the partitioner contains no functions then either create an empty global block (top-level @ref SgAsmBlock) when
     *  this setting is true, or return a null global block pointer when this setting is false. */
    bool allowEmptyGlobalBlock;

    /** Whether to allow functions with no basic blocks.
     *
     *  If the the partitioner knows about a function but was unable to produce any basic blocks then we have two choices
     *  for constructing the @ref SgAsmFunction node in the AST: if this setting is true, then create a function node with
     *  no @ref SgAsmBlock children; otherwise return a null pointer and do not add ths function to the AST. */
    bool allowFunctionWithNoBasicBlocks;

    /** Whether to allow a basic block to be empty.
     *
     *  If the partitioner contains a basic block with no instructions, such as a block whose starting address is not
     *  mapped, then we have two choices when creating the corresponding @ref SgAsmBlock node in the AST: if this setting
     *  is true, then create a basic block with no @ref SgAsmInstruction children; otherwise return a null pointer and do
     *  not add the basic block to the AST. */
    bool allowEmptyBasicBlocks;

    /** Whether to allow shared instructions in the AST.
     *
     *  This setting controls how an instruction that is shared between two or more functions by virtue of its basic block
     *  being part of both functions is represented in the AST.  If this setting is true, instruction ASTs (rooted at @ref
     *  SgAsmInstruction) are deep-copied into the AST at each place they occur.
     *
     *  The partitioner allows an instruction to be shared by two or functions by virtue of the instruction's basic block
     *  being shared by those functions.  If the copying is not performed then the AST will no longer be a tree (it will be
     *  a lattice) but each instruction can point to only one parent basic block (chosen arbitrarily). Thus, a depth-first
     *  traversal of the AST will find the same @ref SgAsmInstruction node more than once, yet following the instruction's
     *  parent pointer will always return the same basic block. */
    bool copyAllInstructions;

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, unsigned version) {
        s & BOOST_SERIALIZATION_NVP(allowEmptyGlobalBlock);
        s & BOOST_SERIALIZATION_NVP(allowFunctionWithNoBasicBlocks);
        s & BOOST_SERIALIZATION_NVP(allowEmptyBasicBlocks);
        s & BOOST_SERIALIZATION_NVP(copyAllInstructions);
    }

public:
    /** Default constructor. */
    AstConstructionSettings()
        : allowEmptyGlobalBlock(false), allowFunctionWithNoBasicBlocks(false), allowEmptyBasicBlocks(false),
          copyAllInstructions(true) {}

    /** Default strict settings.
     *
     *  These settings try to construct an AST that will work with all old AST-based analyses. Some information represented
     *  in the partitioner might not be copied into the AST. */
    static AstConstructionSettings strict() {
        AstConstructionSettings s;
        s.allowEmptyGlobalBlock = false;
        s.allowFunctionWithNoBasicBlocks = false;
        s.allowEmptyBasicBlocks = false;
        s.copyAllInstructions = true;
        return s;
    }

    /** Default permissive settings.
     *
     *  These settings allow as much of the partitioner's information as possible to be copied into the AST even if it means
     *  that AST might violate some invariants that are expected by old analyses.  For instance, it will allow creation of a
     *  basic block with no instructions if the block exists at a virtual addresses that could not be disassembled. */
    static AstConstructionSettings permissive() {
        AstConstructionSettings s;
        s.allowEmptyGlobalBlock = true;
        s.allowFunctionWithNoBasicBlocks = true;
        s.allowEmptyBasicBlocks = true;
        s.copyAllInstructions = true;               // true keeps the AST a tree instead of a lattice
        return s;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Settings.  All settings must act like properties, which means the following:
//   1. Each setting must have a name that does not begin with a verb.
//   2. Each setting must have a command-line switch to manipulate it.
//   3. Each setting must have a method that queries the property (same name as the property and taking no arguments).
//   4. Each setting must have a modifier method (same name as property but takes a value and returns void)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** How the partitioner should globally treat memory. */
enum MemoryDataAdjustment {
    DATA_IS_CONSTANT,                               /**< Treat all memory as if it were constant. This is accomplished by
                                                     *   removing @ref MemoryMap::READABLE from all segments. */
    DATA_IS_INITIALIZED,                            /**< Treat all memory as if it were initialized. This is a little
                                                     *   weaker than @ref MEMORY_IS_CONSTANT in that it allows the
                                                     *   partitioner to read the value from memory as if it were constant,
                                                     *   but also marks the value as being indeterminate. This is
                                                     *   accomplished by adding @ref MemoryMap::INITIALIZED to all
                                                     *   segments. */
    DATA_NO_CHANGE,                                 /**< Do not make any global changes to the memory map. */
};

/** Settings for loading specimens.
 *
 *  The runtime descriptions and command-line parser for these switches can be obtained from @ref loaderSwitches. */
struct LoaderSettings {
    size_t deExecuteZerosThreshold;                 /**< Size threshold for removing execute permission from zero data. If
                                                     *   this data member is non-zero, then the memory map will be adjusted
                                                     *   by removing execute permission from any region of memory that has
                                                     *   at least this many consecutive zero bytes. The affected regions
                                                     *   are adjusted by the @ref deExecuteZerosLeaveAtFront and @ref
                                                     *   deExecuteZerosLeaveAtBack data members. This happens after the
                                                     *   @ref memoryIsExecutable property is processed. */
    size_t deExecuteZerosLeaveAtFront;              /**< Number of bytes at the beginning of each zero area to leave
                                                     *   unaffected. */
    size_t deExecuteZerosLeaveAtBack;               /**< Number of bytes at the end of each zero area to leave
                                                     *   unaffected. */
    MemoryDataAdjustment memoryDataAdjustment;      /**< How to globally adjust memory segment access bits for data
                                                     *   areas. See the enum for details. The default is @ref
                                                     *   DATA_NO_CHANGE, which causes the partitioner to use the
                                                     *   user-supplied memory map without changing anything. */
    bool memoryIsExecutable;                        /**< Determines whether all of memory should be made executable. The
                                                     *   executability bit controls whether the partitioner is able to make
                                                     *   instructions at that address.  The default, false, means that the
                                                     *   engine will not modify executable bits in memory, but rather use
                                                     *   the bits already set in the memory map. This happens before the
                                                     *   @ref deExecuteZeros property is processed. */
    bool linkObjectFiles;                           /**< Link object files before parsing. */
    bool linkStaticArchives;                        /**< Link static libraries before parsing. */
    std::string linker;                             /**< Command to run to link object and archives.  ELF object files
                                                     *   typically don't contain information about how the object is mapped
                                                     *   into memory. If this setting is a non-empty string then a shell
                                                     *   command is constructed and run on all the supplied object and library
                                                     *   files and the resulting file is used instead.  The string should
                                                     *   contain two variables of the form "%o" and "%f" which are the single
                                                     *   output file name and the space separated list of input names. The
                                                     *   names are escaped when the command is generated and therefore the "%o"
                                                     *   and "%f" should not be quoted. */

    LoaderSettings()
        : deExecuteZerosThreshold(0), deExecuteZerosLeaveAtFront(16), deExecuteZerosLeaveAtBack(1),
          memoryDataAdjustment(DATA_IS_INITIALIZED), memoryIsExecutable(false), linkObjectFiles(true),
          linkStaticArchives(true), linker("ld -o %o --unresolved-symbols=ignore-all --whole-archive %f") {}

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, unsigned version) {
        s & BOOST_SERIALIZATION_NVP(deExecuteZerosThreshold);
        s & BOOST_SERIALIZATION_NVP(deExecuteZerosLeaveAtFront);
        s & BOOST_SERIALIZATION_NVP(deExecuteZerosLeaveAtBack);
        s & BOOST_SERIALIZATION_NVP(memoryDataAdjustment);
        s & BOOST_SERIALIZATION_NVP(memoryIsExecutable);
    }
};

/** Settings that control the disassembler.
 *
 *  The runtime descriptions and command-line parser for these switches can be obtained from @ref disassemblerSwitches. */
struct DisassemblerSettings {
    std::string isaName;                            /**< Name of the instruction set architecture. Specifying a non-empty
                                                     *   ISA name will override the architecture that's chosen from the
                                                     *   binary container(s) such as ELF or PE. */

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, unsigned version) {
        s & BOOST_SERIALIZATION_NVP(isaName);
    }
};

/** Controls whether the function may-return analysis runs. */
enum FunctionReturnAnalysis {
    MAYRETURN_DEFAULT_YES,                          /**< Assume a function returns if the may-return analysis cannot
                                                     *   decide whether it may return. */
    MAYRETURN_DEFAULT_NO,                           /**< Assume a function cannot return if the may-return analysis cannot
                                                     *   decide whether it may return. */
    MAYRETURN_ALWAYS_YES,                           /**< Assume that all functions return without ever running the
                                                     *   may-return analysis. */
    MAYRETURN_ALWAYS_NO,                            /**< Assume that a function cannot return without ever running the
                                                     *   may-return analysis. */
};

/** Settings that directly control a partitioner.
 *
 *  These settings are specific to a @ref Partitioner object. */
struct BasePartitionerSettings {
    bool usingSemantics;                            /**< Whether instruction semantics are used. If semantics are used,
                                                     *   then the partitioner will have more accurate reasoning about the
                                                     *   control flow graph.  For instance, semantics enable the detection
                                                     *   of certain kinds of opaque predicates. */
    bool checkingCallBranch;                        /**< Check for situations where CALL is used as a branch. */
    bool basicBlockSemanticsAutoDrop;               /**< Conserve memory by dropping semantics for attached basic blocks. */

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_NVP(usingSemantics);
        s & BOOST_SERIALIZATION_NVP(checkingCallBranch);
        s & BOOST_SERIALIZATION_NVP(basicBlockSemanticsAutoDrop);
    }

public:
    BasePartitionerSettings()
        : usingSemantics(false), checkingCallBranch(false), basicBlockSemanticsAutoDrop(true) {}
};

/** Settings that control the engine partitioning.
 *
 *  These switches are used by the engine to control how it partitions addresses into instructions and static data,
 *  instructions into basic blocks, and basic blocks and static data into functions.  Some of these settings are copied into a
 *  @ref Partitioner object while others affect the @ref Engine directly.
 *
 *  The runtime descriptions and command-line parser for these switches can be obtained from @ref partitionerSwitches. */
struct PartitionerSettings {
    BasePartitionerSettings base;
    std::vector<rose_addr_t> startingVas;           /**< Addresses at which to start recursive disassembly. These
                                                     *   addresses are in addition to entry addresses, addresses from
                                                     *   symbols, addresses from configuration files, etc. */
    bool followingGhostEdges;                       /**< Should ghost edges be followed during disassembly?  A ghost edge
                                                     *   is a CFG edge that is apparent from the instruction but which is
                                                     *   not taken according to semantics. For instance, a branch
                                                     *   instruction might have two outgoing CFG edges apparent by looking
                                                     *   at the instruction syntax, but a semantic analysis might determine
                                                     *   that only one of those edges can ever be taken. Thus, the branch
                                                     *   has an opaque predicate with one actual edge and one ghost edge. */
    bool discontiguousBlocks;                       /**< Should basic blocks be allowed to be discontiguous. If set, then
                                                     *   the instructions of a basic block do not need to follow one after
                                                     *   the other in memory--the block can have internal unconditional
                                                     *   branches. */
    size_t maxBasicBlockSize;                       /**< Maximum basic block size. Number of instructions. 0 => no limit. */
    bool findingFunctionPadding;                    /**< Look for padding before each function entry point? */
    bool findingDeadCode;                           /**< Look for unreachable basic blocks? */
    rose_addr_t peScramblerDispatcherVa;            /**< Run the PeDescrambler module if non-zero. */
    size_t findingIntraFunctionCode;                /**< Suck up unused addresses as intra-function code (number of passes).. */
    bool findingIntraFunctionData;                  /**< Suck up unused addresses as intra-function data. */
    bool findingInterFunctionCalls;                 /**< Look for function calls between functions. */
    bool findingFunctionCallFunctions;              /**< Create functions from function calls. */
    AddressInterval interruptVector;                /**< Table of interrupt handling functions. */
    bool doingPostAnalysis;                         /**< Perform enabled post-partitioning analyses? */
    bool doingPostFunctionMayReturn;                /**< Run function-may-return analysis if doingPostAnalysis is set? */
    bool doingPostFunctionStackDelta;               /**< Run function-stack-delta analysis if doingPostAnalysis is set? */
    bool doingPostCallingConvention;                /**< Run calling-convention analysis if doingPostAnalysis is set? */
    bool doingPostFunctionNoop;                     /**< Find and name functions that are effectively no-ops. */
    FunctionReturnAnalysis functionReturnAnalysis;  /**< How to run the function may-return analysis. */
    size_t functionReturnAnalysisMaxSorts;          /**< Number of times functions are sorted before using unsorted lists. */
    bool findingDataFunctionPointers;               /**< Look for function pointers in static data. */
    bool findingCodeFunctionPointers;               /**< Look for function pointers in instructions. */
    bool findingThunks;                             /**< Look for common thunk patterns in undiscovered areas. */
    bool splittingThunks;                           /**< Split thunks into their own separate functions. */
    SemanticMemoryParadigm semanticMemoryParadigm;  /**< Container used for semantic memory states. */
    bool namingConstants;                           /**< Give names to constants by calling @ref Modules::nameConstants. */
    bool namingStrings;                             /**< Give labels to constants that are string literal addresses. */
    bool namingSyscalls;                            /**< Give names (comments) to system calls if possible. */
    boost::filesystem::path syscallHeader;          /**< Name of header file containing system call numbers. */
    bool demangleNames;                             /**< Run all names through a demangling step. */

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, unsigned version) {
        s & BOOST_SERIALIZATION_NVP(base);
        s & BOOST_SERIALIZATION_NVP(startingVas);
        s & BOOST_SERIALIZATION_NVP(followingGhostEdges);
        s & BOOST_SERIALIZATION_NVP(discontiguousBlocks);
        s & BOOST_SERIALIZATION_NVP(maxBasicBlockSize);
        s & BOOST_SERIALIZATION_NVP(findingFunctionPadding);
        s & BOOST_SERIALIZATION_NVP(findingDeadCode);
        s & BOOST_SERIALIZATION_NVP(peScramblerDispatcherVa);
        if (version >= 2) {
            s & BOOST_SERIALIZATION_NVP(findingIntraFunctionCode);
        } else {
            bool temp = false;
            if (S::is_saving::value)
                temp = findingIntraFunctionCode > 0;
            s & boost::serialization::make_nvp("findingIntraFunctionCode", temp);
            if (S::is_loading::value)
                findingIntraFunctionCode = temp ? 10 : 0; // arbitrary number of passes
        }
        s & BOOST_SERIALIZATION_NVP(findingIntraFunctionData);
        s & BOOST_SERIALIZATION_NVP(findingInterFunctionCalls);
        if (version >= 4)
            s & BOOST_SERIALIZATION_NVP(findingFunctionCallFunctions);
        s & BOOST_SERIALIZATION_NVP(interruptVector);
        s & BOOST_SERIALIZATION_NVP(doingPostAnalysis);
        s & BOOST_SERIALIZATION_NVP(doingPostFunctionMayReturn);
        s & BOOST_SERIALIZATION_NVP(doingPostFunctionStackDelta);
        s & BOOST_SERIALIZATION_NVP(doingPostCallingConvention);
        s & BOOST_SERIALIZATION_NVP(doingPostFunctionNoop);
        s & BOOST_SERIALIZATION_NVP(functionReturnAnalysis);
        if (version >= 3)
            s & BOOST_SERIALIZATION_NVP(functionReturnAnalysisMaxSorts);
        s & BOOST_SERIALIZATION_NVP(findingDataFunctionPointers);
        s & BOOST_SERIALIZATION_NVP(findingCodeFunctionPointers);
        s & BOOST_SERIALIZATION_NVP(findingThunks);
        s & BOOST_SERIALIZATION_NVP(splittingThunks);
        s & BOOST_SERIALIZATION_NVP(semanticMemoryParadigm);
        s & BOOST_SERIALIZATION_NVP(namingConstants);
        s & BOOST_SERIALIZATION_NVP(namingStrings);
        s & BOOST_SERIALIZATION_NVP(demangleNames);
        if (version >= 1) {
            s & BOOST_SERIALIZATION_NVP(namingSyscalls);

            // There is no support for boost::filesystem serialization due to arguments by the maintainers over who has
            // responsibility, so we do it the hard way.
            std::string temp;
            if (S::is_saving::value)
                temp = syscallHeader.string();
            s & boost::serialization::make_nvp("syscallHeader", temp);
            if (S::is_loading::value)
                syscallHeader = temp;
        }
    }

public:
    PartitionerSettings()
        : followingGhostEdges(false), discontiguousBlocks(true), maxBasicBlockSize(0), findingFunctionPadding(true),
          findingDeadCode(true), peScramblerDispatcherVa(0), findingIntraFunctionCode(10), findingIntraFunctionData(true),
          findingInterFunctionCalls(true), findingFunctionCallFunctions(true), doingPostAnalysis(true),
          doingPostFunctionMayReturn(true), doingPostFunctionStackDelta(true), doingPostCallingConvention(false),
          doingPostFunctionNoop(false), functionReturnAnalysis(MAYRETURN_DEFAULT_YES), functionReturnAnalysisMaxSorts(50),
          findingDataFunctionPointers(false), findingCodeFunctionPointers(false), findingThunks(true), splittingThunks(false),
          semanticMemoryParadigm(LIST_BASED_MEMORY), namingConstants(true), namingStrings(true), namingSyscalls(true),
          demangleNames(true) {}
};

// BOOST_CLASS_VERSION(PartitionerSettings, 1); -- see end of file (cannot be in a namespace)

/** Settings for controling the engine behavior.
 *
 *  These settings control the behavior of the engine itself irrespective of how the partitioner is configured. The runtime
 *  descriptions and command-line parser for these switches can be obtained from @ref engineBehaviorSwitches. */
struct EngineSettings {
    std::vector<std::string> configurationNames;    /**< List of configuration files and/or directories. */
    bool exitOnError;                               /**< If true, emit error message and exit non-zero, else throw. */

    EngineSettings()
        : exitOnError(true) {}

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, unsigned version) {
        s & BOOST_SERIALIZATION_NVP(configurationNames);
        s & BOOST_SERIALIZATION_NVP(exitOnError);
    }
};

// Additional declarations w/out definitions yet.
class Partitioner;
class Function;
typedef Sawyer::SharedPointer<Function> FunctionPtr;
class BasicBlock;
typedef Sawyer::SharedPointer<BasicBlock> BasicBlockPtr;
class DataBlock;
typedef Sawyer::SharedPointer<DataBlock> DataBlockPtr;

} // namespace
} // namespace
} // namespace

// Class versions must be at global scope
BOOST_CLASS_VERSION(Rose::BinaryAnalysis::Partitioner2::PartitionerSettings, 4);

#endif
