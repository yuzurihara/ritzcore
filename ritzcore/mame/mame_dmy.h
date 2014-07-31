
#ifndef __MAME_DMY_H_INCLUDED__
#define __MAME_DMY_H_INCLUDED__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif
#define RITZAPI  __stdcall
#define RITZ


#ifdef INLINE
# undef INLINE
#endif
#define INLINE

typedef signed char INT8;
typedef __int16 INT16;
typedef __int32 INT32;
typedef __int64 INT64;
typedef unsigned char UINT8;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;

//typedef signed __int16 stream_sample_t;

#define ATTR_CONST
#define ATTR_FORCE_INLINE
#define ATTR_NORETURN
#define ATTR_PRINTF(x,y)


/*
#define READ8_MEMBER(name)              UINT8  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 mem_mask)
#define WRITE8_MEMBER(name)             void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask)
#define DECLARE_READ8_MEMBER(name)      UINT8  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 mem_mask = 0xff)
#define DECLARE_WRITE8_MEMBER(name)     void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask = 0xff)
*/
#define READ8_MEMBER(name)              UINT8  name(offs_t offset, UINT8 mem_mask)
#define WRITE8_MEMBER(name)             void   name(offs_t offset, UINT8 data, UINT8 mem_mask)
#define DECLARE_READ8_MEMBER(name)      UINT8  name(offs_t offset, UINT8 mem_mask = 0xff)
#define DECLARE_WRITE8_MEMBER(name)     void   name(offs_t offset, UINT8 data, UINT8 mem_mask = 0xff)

class device_t;


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/
UINT64 mulu_32x32(UINT32 a, UINT32 b);
/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/
UINT32 divu_64x32_rem(UINT64 a, UINT32 b, UINT32 *rem);
/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/
INT64 mul_32x32(INT32 a, INT32 b);


#define M_PI 3.14159265358979323846

#define logerror(...)

/****************************************************************
 *
 * osdcomm.h
 *
 ****************************************************************/
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

/****************************************************************
 *
 * diexec.h
 *
 ****************************************************************/
// I/O line states
enum line_state
{
	CLEAR_LINE = 0,             // clear (a fired or held) line
	ASSERT_LINE,                // assert an interrupt immediately
	HOLD_LINE,                  // hold interrupt line until acknowledged
	PULSE_LINE                  // pulse interrupt line instantaneously (only for NMI, RESET)
};

#define MAX_INPUT_LINES 32+3



#endif /* __MAME_DMY_H_INCLUDED__ */

