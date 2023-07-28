/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __CC_H__
#define __CC_H__

#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

typedef int sys_prot_t;

#define LWIP_PROVIDE_ERRNO

#if defined (__GNUC__) & !defined (__CC_ARM)

#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>

#endif
#if 0 //def RSI_WITH_OS
//!Data types
#define u8_t uint8_t
#define u16_t uint16_t
#define u32_t uint32_t
#define s8_t int8_t
#define s16_t int16_t
#define s32_t int32_t

//!printf formatters for data types
#define U16_F %hu
#define S16_F %d
#define X16_F %hx
#define U32_F %u
#define S32_F %d
#define X32_F %x
#define SZT_F %uz

//! Byte ordering 
//#define BYTE_ORDER BIG_ENDIAN

//! byte Swapping
#define LWIP_PLATFORM_BYTESWAP 1
#define LWIP_PLATFORM_HTONS(x) ( (((u16_t)(x))>>8) | (((x)&0xFF)<<8) )
#define LWIP_PLATFORM_HTONL(x) ( (((u32_t)(x))>>24) | (((x)&0xFF0000)>>8) | (((x)&0xFF00)<<8) | (((x)&0xFF)<<24) ) 
							   
//! IP protocols use checksums (see RFC 1071). LwIP gives you a choice of 3 algorithms
//! load byte by byte, construct 16 bits word and add: not efficient for most platforms
//! load first byte if odd address, loop processing 16 bits words, add last byte.
//! load first byte and word if not 4 byte aligned, loop processing 32 bits words, add last word/byte.
#define LWIP_CHKSUM_ALGORITHM 2		

//! Used structure packing			
#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END	

//! Platform specific diagnostic output
//#define LWIP_PLATFORM_DIAG(x) printf("\n non-fatal :0x%x\n", x)	
//#define LWIP_PLATFORM_ASSERT(x) printf("\n fatal :%s\n", x) \
//                                while(1)	
//int32_t *mem_ptr_t;
#endif
/* define compiler specific symbols */
#if defined (__ICCARM__)

#define PACK_STRUCT_BEGIN
//#define PACK_STRUCT_STRUCT 
#define PACK_STRUCT_END
//#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_USE_INCLUDES

#elif defined (__GNUC__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined (__CC_ARM)

#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined (__TASKING__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#endif

//#define LWIP_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\n", \
//                                     x, __LINE__, __FILE__); } while(0)

/* Define random number generator function */
#define LWIP_RAND() ((u32_t)rand())

#endif /* __CC_H__ */
