//*****************************************************************************
// $Workfile: pubCpmAdvAPI.h $
//
// DESCRIPTION:
/**
    \file
    \brief Function prototypes and constants related to the ClearPath-SC
    motor API.

    \ingroup CPMgrp
    @{

**/
// CREATION DATE:
//        8/2/2011 refactored from Meridian pubIscAPI.h
//
// COPYRIGHT NOTICE:
//        (C)Copyright 1998-2019  Teknic, Inc.  All rights reserved.
//
//        This copyright notice must be reproduced in any copy, modification, 
//        or portion thereof merged into another program. A copy of the 
//        copyright notice must be included in the object library of a user
//        program.
//                                                                            *
//*****************************************************************************

#ifndef __PUB_CPM_ADV_API_H_
#define __PUB_CPM_ADV_API_H_

#include "mnParamDefs.h"
#include "pubCpmRegs.h"
#include "pubNetAPI.h"

//////////////////////
// PUBLIC FUNCTIONS //
//////////////////////

/// Helper function to convert volatile parameter access reference into
/// a non-volatile parameter access.
inline cpmParams cpmNonVol(cpmParams volParam) {
    return((cpmParams)(volParam | PARAM_OPT_MASK));
}

#ifdef __cplusplus
extern "C" {
#endif
    
// Restore the parameters to factory default state
MN_EXPORT cnErrCode MN_DECL cpmFactoryDefaults(
    multiaddr theMultiAddr);    // Destination node

// Restore the parameters to factory default state
MN_EXPORT cnErrCode MN_DECL cpmFactoryDefaultsEx(
    multiaddr theMultiAddr,     // Destination node
    bool skipShutdownRisks);    // Skip some params which may cause shutdown

MN_EXPORT cnErrCode MN_DECL cpmGetParameterEx(
    multiaddr theMultiAddr,     // Node address
    cpmParams theParam,         // Parameter index
    paramValue *pParamVal,      // Pointer to value area
    paramInfo *pParamInfo);     // Pointer to information area


// Update the local parameter table and update node using a buffer
MN_EXPORT cnErrCode MN_DECL cpmSetParameterEx(
    multiaddr theMultiAddr,     // Node address
    cpmParams theParam,         // Parameter index
    packetbuf *pNewValue);      // New value

// Adjust the position command
MN_EXPORT cnErrCode MN_DECL cpmAddToPosition(
    multiaddr theMultiAddr,     // Node address
    double theOffset);          // Offset from current location

// Register parameter change function
MN_EXPORT paramChangeFunc MN_DECL cpmParamChangeFunc(
    paramChangeFunc newFunc);   // Pointer to callback function


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                                 MOTION API
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

//-----------------
// Velocity Group
//-----------------
// Low level command primitive
MN_EXPORT cnErrCode MN_DECL cpmForkMoveVelEx(
    multiaddr theMultiAddr,          // Node address
    double velTargetStepPerSec,      // Velocity (steps/sec)
    nodelong positionTarget,         // Target position (optional)
    mgVelStyle moveType);            // Motion style


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                          MONITOR PORT/DATA ACQUISITION API
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Monitor port manipulations
MN_EXPORT cnErrCode MN_DECL cpmGetMonitor(
    multiaddr theMultiAddr,    // Target node addr
    nodeushort channel,        // Target monitor port if supported
    iscMonState *pState);

MN_EXPORT cnErrCode MN_DECL cpmSetMonitor(
    multiaddr theMultiAddr,    // Target node addr
    nodeushort channel,        // Target monitor port if supported
    iscMonState *pNewState);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                             TUNING STIMULUS API
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
MN_EXPORT cnErrCode MN_DECL cpmGetStimulus(
    multiaddr theMultiAddr,        // Target node addr
    iscStimState *pState);

MN_EXPORT cnErrCode MN_DECL cpmSetStimulus(
    multiaddr theMultiAddr,        // Target node addr
    iscStimState *pNewState);

// Reset the vector search flag
MN_EXPORT cnErrCode MN_DECL cpmReVector(
    multiaddr theMultiAddr);       // Target node addr


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                           NODE IDENTIFICATION API
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Set/Get the user ID
MN_EXPORT cnErrCode MN_DECL cpmSetUserID(
    multiaddr theMultiAddr,
    const char *pNewName);

MN_EXPORT cnErrCode MN_DECL cpmGetUserID(
    multiaddr theMultiAddr,
    char *pUserIdStr,
    Uint16 maxBufSize);

// Set/Get Motor File Name
MN_EXPORT cnErrCode MN_DECL cpmSetMotorFileName(
    multiaddr theMultiAddr,
    const char *pNewName);

MN_EXPORT cnErrCode MN_DECL cpmGetMotorFileName(
    multiaddr theMultiAddr,
    char *pMotorFileNameStr,
    Uint16 maxBufSize);

// Set/Get User Description
MN_EXPORT cnErrCode MN_DECL cpmSetUserDesc(
    multiaddr theMultiAddr,
    const char *pNewName);

MN_EXPORT cnErrCode MN_DECL cpmGetUserDesc(
    multiaddr theMultiAddr,
    char *pMotorFileNameStr,
    Uint16 maxBufSize);


#ifdef __cplusplus
}
#endif
/// @}

#endif // __PUB_CPM_ADV_API_H_



/*==========================================================================
    END OF FILE pubCpmAdvAPI.h
  ==========================================================================*/
