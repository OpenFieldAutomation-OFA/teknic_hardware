/*
 * pubEtherCatApi.h
 *
 *  Created on: May 23, 2022
 *      Author: scott_mayne
 */

#ifndef INC_INC_PUB_PUBETHERCATAPI_H_
#define INC_INC_PUB_PUBETHERCATAPI_H_

#include <stdint.h>


typedef union _P402ParamInfo_t {
    uint32_t  bits[2];
    struct _fld {
        unsigned index          : 16;
        unsigned subindex       :  8;
        unsigned octetSizeLess1 :  4;
        unsigned signedVal      :  1;
        unsigned hexDisplay     :  1;
        unsigned misalignedByte :  1;
        unsigned stringValue    :  1;
        unsigned isWriteable    :  1;
    } fld;
#ifdef __cplusplus
    _P402ParamInfo_t() {
        bits[0] = 0;
        bits[1] = 0;
    }
    _P402ParamInfo_t(const _P402ParamInfo_t &ref) {
        bits[0] = ref.bits[0];
        bits[1] = ref.bits[1];
    }
    _P402ParamInfo_t(const uint32_t &info1, const uint32_t& info2) {
        bits[0] = info1;
        bits[1] = info2;
    }
#endif
} P402ParamInfo_t;



#endif /* INC_INC_PUB_PUBETHERCATAPI_H_ */
