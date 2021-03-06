#pragma once

#ifndef __AY8910_H__
#define __AY8910_H__

#include "emu.h"

/*
AY-3-8910A: 2 I/O ports
AY-3-8912A: 1 I/O port
AY-3-8913A: 0 I/O port
AY-3-8914:  same as 8910 except for different register mapping and two bit envelope enable / volume field
AY8930: upper compatible with 8910.
In extended mode, it has higher resolution and duty ratio setting
YM2149: higher resolution, selectable clock divider
YM3439: same as 2149
YMZ284: 0 I/O port, different clock divider
YMZ294: 0 I/O port
*/

#define ALL_8910_CHANNELS -1

/* Internal resistance at Volume level 7. */

#define AY8910_INTERNAL_RESISTANCE  (356)
#define YM2149_INTERNAL_RESISTANCE  (353)

/*
 * Default values for resistor loads.
 * The macro should be used in AY8910interface if
 * the real values are unknown.
 */
#define AY8910_DEFAULT_LOADS        {1000, 1000, 1000}

/*
 * The following is used by all drivers not reviewed yet.
 * This will like the old behaviour, output between
 * 0 and 7FFF
 */
#define AY8910_LEGACY_OUTPUT        (0x01)

/*
 * Specifing the next define will simulate the special
 * cross channel mixing if outputs are tied together.
 * The driver will only provide one stream in this case.
 */
#define AY8910_SINGLE_OUTPUT        (0x02)

/*
 * The following define is the default behaviour.
 * Output level 0 is 0V and 7ffff corresponds to 5V.
 * Use this to specify that a discrete mixing stage
 * follows.
 */
#define AY8910_DISCRETE_OUTPUT      (0x04)

/*
 * The following define causes the driver to output
 * resistor values. Intended to be used for
 * netlist interfacing.
 */

#define AY8910_RESISTOR_OUTPUT      (0x08)

/*
 * This define specifies the initial state of YM2149
 * pin 26 (SEL pin). By default it is set to high,
 * compatible with AY8910.
 */
/* TODO: make it controllable while it's running (used by any hw???) */
#define YM2149_PIN26_HIGH           (0x00) /* or N/C */
#define YM2149_PIN26_LOW            (0x10)


struct ay8910_interface
{
	int                 flags;          /* Flags */
	int                 res_load[3];    /* Load on channel in ohms */
	//devcb_read8         portAread;
	//devcb_read8         portBread;
	//devcb_write8        portAwrite;
	//devcb_write8        portBwrite;
};

/*********** An interface for SSG of YM2203 ***********/

void *ay8910_start_ym(device_t *device, const ay8910_interface *intf);
void ay8910_stop_ym(void *chip);
void ay8910_reset_ym(void *chip);
void ay8910_set_clock_ym(void *chip, int clock);
void ay8910_set_volume(void *chip ,int channel, int volume);
void ay8910_write_ym(void *chip, int addr, int data);
int ay8910_read_ym(void *chip);
void ay8910_update //(void *param, stream_sample_t **outputs, int samples);
(device_t *device, sound_stream *stream, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples);


class ay8910_device : public device_t /*,
									public device_sound_interface */
{
public:
	ay8910_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ay8910_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_WRITE8_MEMBER( data_w );

	/* /RES */
	DECLARE_WRITE8_MEMBER( reset_w );

	/* use this when BC1 == A0; here, BC1=0 selects 'data' and BC1=1 selects 'latch address' */
	DECLARE_WRITE8_MEMBER( data_address_w );

	/* use this when BC1 == !A0; here, BC1=0 selects 'latch address' and BC1=1 selects 'data' */
	DECLARE_WRITE8_MEMBER( address_data_w );

	void set_volume(int channel,int volume);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	const ay8910_interface *m_ay8910_config;
	void *m_psg;
};

#endif /* __AY8910_H__ */
