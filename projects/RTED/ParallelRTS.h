

//
// creates the interfaces to support RTED on UPC
//
// On on UPC code, the interfaces decay to static inline functions

#ifndef _PARALLEL_RTS_H
#define _PARALLEL_RTS_H

#include "CppRuntimeSystem/rted_iface_structs.h"
#include "CppRuntimeSystem/ptrops.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef WITH_UPC

/// \brief polls incoming message buffer
void rted_ProcessMsg(void);

/// \brief sends a free message to all other processes
void snd_FreeMemory(rted_Address addr, rted_AllocKind freeKind, rted_SourceInfo si);

/// \brief shares information about non-local heap allocations
void snd_CreateHeapPtr(rted_TypeDesc, rted_Address, rted_Address, rted_AddressDesc, size_t, size_t, rted_AllocKind, const char*, rted_SourceInfo);

/// \brief shares information about variable initializations
void snd_InitVariable(rted_TypeDesc, rted_Address, rted_Address, rted_AddressDesc, size_t, int, const char*, rted_SourceInfo);

/// \brief shares information about pointer movements
void snd_MovePointer(rted_TypeDesc, rted_Address, rted_Address, rted_AddressDesc, const char*, rted_SourceInfo);

/// \brief initializes the runtime system
void rted_UpcAllInitialize(void);

#else /* WITH_UPC */

#define UNUSEDARG(X) ((void) &(X)) /* we write for C and C++ compilers ;) */

// when we do not use UPC, we compile the runtime system with empty
//   implementations.
static inline
void rted_ProcessMsg(void)
{}

static inline
void snd_FreeMemory(rted_Address r, rted_AllocKind a, rted_SourceInfo s)
{
  UNUSEDARG(r), UNUSEDARG(a), UNUSEDARG(s);
}

static inline
void snd_CreateHeapPtr(rted_TypeDesc td, rted_Address addr, rted_Address haddr, rted_AddressDesc hdesc, size_t sz, size_t mallocsz, rted_AllocKind ak, const char* cn, rted_SourceInfo si)
{
  UNUSEDARG(td), UNUSEDARG(addr), UNUSEDARG(haddr), UNUSEDARG(hdesc), UNUSEDARG(sz), UNUSEDARG(mallocsz), UNUSEDARG(ak), UNUSEDARG(cn), UNUSEDARG(si);
}

static inline
void snd_InitVariable(rted_TypeDesc td, rted_Address addr, rted_Address haddr, rted_AddressDesc hdesc, size_t sz, int pm, const char* cn, rted_SourceInfo si)
{
  UNUSEDARG(td), UNUSEDARG(addr), UNUSEDARG(haddr), UNUSEDARG(hdesc), UNUSEDARG(sz), UNUSEDARG(pm), UNUSEDARG(cn), UNUSEDARG(si);
}

static inline
void snd_MovePointer(rted_TypeDesc td, rted_Address addr, rted_Address haddr, rted_AddressDesc hdesc, const char* cn, rted_SourceInfo si)
{
  UNUSEDARG(td), UNUSEDARG(addr), UNUSEDARG(haddr), UNUSEDARG(hdesc), UNUSEDARG(cn), UNUSEDARG(si);
}

/// \note even w/o UPC this function is needed b/c it will be the first function executed in main
///       it becomes the reference point for further RTED initializations at startup
/// \todo remove
static inline
void rted_UpcAllInitialize(void)
{}

#endif /* WITH_UPC */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _PARALLEL_RTS_H */
