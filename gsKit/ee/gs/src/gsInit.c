//  ____     ___ |    / _____ _____
// |  __    |    |___/    |     |
// |___| ___|    |    \ __|__   |     gsKit Open Source Project.
// ----------------------------------------------------------------------
// Copyright 2004 - Chris "Neovanglist" Gilbert <Neovanglist@LainOS.org>
// Licenced under Academic Free License version 2.0
// Review gsKit README & LICENSE files for further details.
//
// gsInit.c - GS initialization and configuration routines.
//
// Parts taken from ooPo's ee-syscalls.txt
// http://www.oopo.net/consoledev/files/ee-syscalls.txt
//
// Parts taken from emoon's BreakPoint Demo Library
//

#include "gsKit.h"

int gsKit_init(unsigned int interlace, unsigned int mode, unsigned int field)
{
	__asm__ __volatile__("
		# SetGsCrt
		li  $3, 0x02;
		syscall;
		nop;");

	return 0;
}

GSGLOBAL gsKit_init_screen(GSGLOBAL gsGlobal)
{
	u64	*p_data;
	u64	*p_store;

	GS_SET_PMODE(0,		// Read Circuit 1
		     1,		// Read Circuit 2
		     0,		// Use ALP Register for Alpha Blending
		     1,		// Alpha Value of Read Circuit 2 for Output Selection
		     0,		// Blend Alpha with output of Read Circuit 2
		     0xFF);	// Alpha Value = 1.0

	GS_SET_DISPFB2(0,			// Frame Buffer Base Pointer (Address/2048)
		       gsGlobal.Width / 64,	// Buffer Width (Address/64)
		       gsGlobal.PSM,		// Pixel Storage Format
		       0,			// Upper Left X in Buffer
		       0);			// Upper Left Y in Buffer

	GS_SET_BGCOLOR(gsGlobal.BGColor.Red,	// Red
		       gsGlobal.BGColor.Green,	// Green
		       gsGlobal.BGColor.Blue);	// Blue

	gsGlobal.ScreenBuffer[0] = gsKit_vram_alloc( gsGlobal, 256*640*4 ); // Context 1
	gsGlobal.ScreenBuffer[1] = gsKit_vram_alloc( gsGlobal, 256*640*4 ); // Context 2
	gsGlobal.ScreenBuffer[2] = gsKit_vram_alloc( gsGlobal, 256*640*4 ); // Z Buffer
	p_data = p_store  = dmaKit_spr_alloc( (8+5)*16 );

	*p_data++ = GIF_TAG( 7+5, 1, 0, 0, 0, 1 );
	*p_data++ = GIF_AD;

	*p_data++ = 1;
	*p_data++ = GS_PRMODECONT,	
	
	*p_data++ = GS_SETREG_FRAME_1( 0, gsGlobal.Width / 64, gsGlobal.PSM, 0 );
	*p_data++ = GS_FRAME_1,

	*p_data++ = GS_SETREG_XYOFFSET_1( gsGlobal.OffsetX << 4, gsGlobal.OffsetY << 4 );
	*p_data++ = GS_XYOFFSET_1,

	*p_data++ = GS_SETREG_SCISSOR_1( 0, gsGlobal.Width-1, 0, gsGlobal.Height-1 );
	*p_data++ = GS_SCISSOR_1,

	*p_data++ = GS_SETREG_ZBUF_1( gsGlobal.ScreenBuffer[2] / 8192, 0, 0 );
	*p_data++ = GS_ZBUF_1,

	*p_data++ = GS_SETREG_TEST( 1, 7, 0xFF, 0, 0, 0, 1, 1 );
	*p_data++ = GS_TEST_1,

	*p_data++ = GS_SETREG_COLCLAMP( 255 );
	*p_data++ = GS_COLCLAMP,

	*p_data++ = GS_SETREG_FRAME_1( 0, gsGlobal.Width / 64, gsGlobal.PSM, 0 );
	*p_data++ = GS_FRAME_2,

	*p_data++ = GS_SETREG_XYOFFSET_1( gsGlobal.OffsetX << 4, gsGlobal.OffsetY << 4 );
	*p_data++ = GS_XYOFFSET_2,

	*p_data++ = GS_SETREG_SCISSOR_1( 0, gsGlobal.Width-1, 0, gsGlobal.Height-1 );
	*p_data++ = GS_SCISSOR_2,

	*p_data++ = GS_SETREG_ZBUF_1( gsGlobal.ScreenBuffer[2] / 8192, 0, 0 );
	*p_data++ = GS_ZBUF_2,

	*p_data++ = GS_SETREG_TEST( 1, 7, 0xFF, 0, 0, 0, 1, 1 );
	*p_data++ = GS_TEST_2,

	dmaKit_send_chain( DMA_CHANNEL_GIF, 0, p_store, sizeof(p_store) );

	return gsGlobal;
}

