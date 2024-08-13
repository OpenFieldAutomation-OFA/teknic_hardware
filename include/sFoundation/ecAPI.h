//*****************************************************************************
// DESCRIPTION:
///		\file
///		This file contains the platform independent definitions and data for
///		talking to an EtherPath motor.
//
// CREATION DATE:
//		05/20/2022
//
// COPYRIGHT NOTICE:
//		(C)Copyright 2010-2018  Teknic, Inc.  All rights reserved.
//
//		This copyright notice must be reproduced in any copy, modification,
//		or portion thereof merged into another program. A copy of the
//		copyright notice must be included in the object library of a user
//		program.
//																			  *
//*****************************************************************************
/// \cond INTERNAL_DOC

#ifndef __ECAPI_H__
#define __ECAPI_H__

//*****************************************************************************
// !NAME!																      *
// 	ecAPI.h headers included
//
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <pubNetAPI.h>
#endif
#include <pubEtherCatApi.h>
#include "tekTypes.h"
//																			  *
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    MN_EXPORT cnErrCode MN_DECL infcGetP402ParamCount(multiaddr theMultiAddr, uint32_t *count);
    MN_EXPORT cnErrCode MN_DECL infcGetP402ParamInfo(multiaddr theMultiAddr, uint16_t index, P402ParamInfo_t* result);
    MN_EXPORT cnErrCode MN_DECL infcGetP402ParamValue(multiaddr theMultiAddr, uint16_t index, double* result);
    MN_EXPORT cnErrCode MN_DECL infcGetP402ParamRaw(multiaddr theMultiAddr, uint16_t index, char* pParamBuf, uint16_t maxBufSize);
    MN_EXPORT cnErrCode MN_DECL infcSetP402Param(multiaddr theMultiAddr, uint16_t index, uint8_t subindex,
        void* pParam, uint16_t paramSize);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif
/// \endcond
//=============================================================================
//	END OF FILE ecApi.h
// ============================================================================
