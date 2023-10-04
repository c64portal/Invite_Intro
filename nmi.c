/* 
 * Invitation intro for Moonshine Dragons and Teddy Beer parties
 * (c) 2022 by Carrion / C64portal.
 *
 * nmi.c
 * RUNStop + Restore handler. Included by all screens. 
 */

#ifndef _NMI
#define _NMI

void NMIhandle (){
 
kickasm	{{
   	lda #<NMI
   	sta $fffa
   	lda #>NMI
   	sta $fffb
   	rts
 NMI:
	rti
	}}
}

#endif
