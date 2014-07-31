// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emu.h

    Core header file to be included by most files.

    NOTE: The contents of this file are designed to meet the needs of
    drivers and devices. In addition to this file, you will also need
    to include the headers of any CPUs or other devices that are required.

    If you find yourself needing something outside of this file in a
    driver or device, think carefully about what you are doing.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#define __EMU_H__

#include <Windows.h>
#include "mame_dmy.h"
#include "emucore.h"
#include "../ritzcore.h"

#define SIN_BITS        10
#define SIN_LEN         (1<<SIN_BITS)
#define SIN_MASK        (SIN_LEN-1)

#ifndef M_PI
#define M_PI                            3.14159265358979323846
#endif

// #include "sound.h"


typedef void emu_timer;
typedef double attotime;
typedef unsigned int offs_t;

#define ATTR_UNUSED


/************************************************************************
 * 
 * import from emu/sound.h
 *
 ************************************************************************/
//#define STREAM_UPDATE(name) void name(device_t *device, sound_stream *stream, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
//#define STREAM_UPDATE(name) void name(device_t *device, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)


/************************************************************************
 * 
 * import from emu/devcb.h
 *
 ************************************************************************/

#include "cconv.h"
#include "emucore.h"
#include "memory.h"
#include "sound.h"
#include "device.h"

#endif  /* __EMU_H__ */
