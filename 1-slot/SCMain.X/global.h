/* 
 * File:   global.h
 * Author: KLam
 *
 * Created on November 23, 2016, 7:08 PM
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "mcc_generated_files/mcc.h"
#include "comm_handle.h"
#include "misc.h"
#include "atecc_handle.h"
#include "charge_handle.h"

#ifdef	__cplusplus
extern "C" {
#endif

//#define DEBUG_MSG
    
#define FW_VERSION 0x01
#define FW_BUILD   0x11
    
#define MAX_CURRENT_MA  2000


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

