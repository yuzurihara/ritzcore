// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    memory.c

    Functions which handle device memory access.

****************************************************************************/

#include "emu.h"

address_space::address_space(offs_t offset, offs_t size)
:	m_offset(offset),
	m_bytes(size)
{
	m_memory = (UINT8*)RitzAlloc(size);
}

address_space::~address_space(){
	RitzFree(m_memory);
}


UINT8 address_space::read_byte(offs_t adrs){
	UINT8 byte;
	if(adrs<m_offset){
		byte = 0xff;
	}else{
		adrs -= m_offset;
		if(adrs<m_bytes)
			byte = m_memory[adrs];
		else
			byte = 0xff;
	}
	return byte;
}

void address_space::write_byte(offs_t adrs, UINT8 data){
	if(m_offset<=adrs){
		adrs -= m_offset;
		if(adrs<m_bytes)
			m_memory[adrs] = data;
	}
}
