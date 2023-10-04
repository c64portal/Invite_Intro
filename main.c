/* 
 * Invitation intro for Moonshine Dragons and Teddy Beer parties
 * (c) 2022 by Carrion / C64portal.pl
 */ 

#pragma link("main.ld")

#define _MAIN_

#include <c64.h>
#include <6502.h>
#include "nmi.c"

byte *DD00 = $dd00;

#define CHARSET_CONST  0xe000
#define CHARSET2_CONST  0xf000
byte* CHARSET 		= CHARSET_CONST;
byte* CHARSET2 		= CHARSET2_CONST;
byte* SCREEN0  		= $e800; 
byte* SCREEN1  		= $f800; 

#define MAP1_CONST $5000
byte* MAP1  		= MAP1_CONST;

#define SPRITESMEM_CONST 0xcd00
#define SPRITESMEM_BASE  0x0d00

#define MUSIC_CONST 0x1000
byte* MUSIC 		= MUSIC_CONST; 

#define PICTURE0_CONST 0x2000
byte* PICTURE0 		= PICTURE0_CONST;

#define PICTURE1_CONST 0x6000
byte* PICTURE1 		= PICTURE1_CONST;

byte* PICTURE0_SCREEN = 0x0400;
byte* PICTURE1_SCREEN = 0x4800;

#define OFFSET0 $1f40 
#define OFFSET1 $1f40+$3e8 

#define MAX_TIMELINE 8
#define FRAMEFADE_DELAY 8


// --------------------------------------------------------------------
// ------------------------ timing stages


enum timelineStages {
	STAGE_IDLE,
	STAGE_PICPREP0,
	STAGE_PIC0, 
	STAGE_PICPREP1,
	STAGE_PIC1,  
	STAGE_TEDYPREP0,
	STAGE_TEDDY0,
	STAGE_TEDDY1
};

// addresses of the following irq routines
export word timelineIRQ[] = kickasm {{
	.word irqIdle
	.word irqTeddyIdle
	.word irqPic0
	.word irqTeddyIdle
	.word irqPic1
	.word irqTeddyIdle
	.word irqTeddy
	.word irqTeddyIdle

}};

// timings (in frames, 50 frames=1 sek) 
// of each screen that shows during this little demo
export unsigned int timelineTimings[] = {
	260,1300, 800, 100, 600, 50, 2000,50,300,200,100,200,300
};

// --------------------------------------------------------------------

// some global variables
volatile byte introState ; 
volatile word timer ; // main timer
volatile word mapPosition ; // used for scrolled screen
volatile word mapSpeed ;
volatile byte currentTimelinePosition = 0;

// --------------------------------------------------------------------

void main () {

    kickasm (uses MUSIC){{

		!: lda $d011
			bpl !-
		!: lda $d011
			bmi !-

			lda #0
			sta $d020
			lda #$0b
			sta $d011

    	lda #$00
    	ldx #$00
    	ldy #$00
    	jsr MUSIC
    }}

    clearScreen (SCREEN0,0x00);
    clearScreen (0xd800,0x09);
    clearTextArea (0x05);
    clearScreen (PICTURE0_SCREEN,0x00);
    clearScreen (PICTURE1_SCREEN,0x00);

    SEI();
    CIA1->INTERRUPT = CIA_INTERRUPT_CLEAR;
    VICII->CONTROL1 &=$7f;
    VICII->RASTER = $22;
    VICII->IRQ_ENABLE = IRQ_RASTER;
    VICII->BORDER_COLOR = 0;
    VICII->BG_COLOR =0;
    *HARDWARE_IRQ = timelineIRQ[currentTimelinePosition];
    *PROCPORT_DDR = PROCPORT_DDR_MEMORY_MASK;
    *PROCPORT = PROCPORT_RAM_IO;
    NMIhandle ();

	timer=0;
	introState = currentTimelinePosition;
    CLI();


    // poor people state machine :)
    // this could also be a switch with cases
    while (true){
    	if (introState == STAGE_IDLE){
		}

    	if (introState == STAGE_PICPREP0){
    		clearScreen (0xd800,0x00);
    		mapPosition = 0;
    		introState = STAGE_IDLE;
		}
    	if (introState == STAGE_PICPREP1){
    		clearScreen (0xd800,0x00);
    		introState = STAGE_IDLE;
		}

    	if (introState == STAGE_TEDYPREP0){
    		clearScreen (0xd800,0x09);
    		mapPosition = 80*8;
    		introState = STAGE_IDLE;
		}

    	if (introState == STAGE_TEDDY0){
			fadeFrame =0;
			fadeLines =0;
		    // clearScreen (0xd800,0x00);
		}

		if (introState == STAGE_PIC0){
		}
		if (introState == STAGE_PIC1){
		}

		if (introState == STAGE_TEDDY1){
		    clearScreen (0xd800,0x00);
		    clearTextArea (0x05);
	    	clearScreen (PICTURE0_SCREEN,0x00);
    		clearScreen (PICTURE1_SCREEN,0x00);

			mapPosition =1600*8;
			introState = STAGE_IDLE;
		}

	}
}


// memory menagement. DataHigh and CodeHigh are defined in main.ld file
#pragma data_seg(DataHigh)
#pragma code_seg(CodeHigh)

// --------------------------------------------------------------------
/*
	This is a function that handles the timeline and changes the introState
	and switches irq handling procedures.
	when new irq starts the timer is zeroed.
	The values for timer and irq handling procedure are stored in 
	timelineIRQ and timelineTimings arrays.
*/
void handleTimeline (){
	timer ++;
	word t=timelineTimings[currentTimelinePosition];
	if (t==timer){
    	currentTimelinePosition++; 
    	timer=0;
    	
    	introState    = currentTimelinePosition;
		*HARDWARE_IRQ = timelineIRQ[currentTimelinePosition];
    	if (currentTimelinePosition == MAX_TIMELINE) {
    		currentTimelinePosition =2;
    		timer=0;
    		textPos=0;
    		introState    = currentTimelinePosition;
    		*HARDWARE_IRQ = timelineIRQ[currentTimelinePosition];
    	}
	}
}

inline void playMusic (){
	kickasm {{
		jsr music+3
	}}
}

// --------------------------------------------------------------------
// just empty screen.

__interrupt(hardware_all) void irqIdle() {

	handleTimeline();
	playMusic();

	*D018 = toD018 (PICTURE0_SCREEN,  PICTURE0);
    *DD00 = toDd00 (PICTURE0);
    *D011 = $00;
    *D016 = $c8;
    VICII->SPRITES_ENABLE = 0x00;

    // waitForSpace ();
    *IRQ_STATUS = IRQ_RASTER;
}

// --------------------------------------------------------------------
// picture and scroll

__interrupt(hardware_all) void irqPic0() {

	// VICII->BORDER_COLOR = $0b;

	*D018 = toD018 (PICTURE0_SCREEN, PICTURE0);
    *DD00 = toDd00 (PICTURE0);
    *D011 = $3b;
    *D016 = $d8;
    VICII->SPRITES_ENABLE = 0xff;
	VICII->SPRITES_MC = 0xff;
	VICII->SPRITES_MCOLOR1 = 0x00;
	VICII->SPRITES_MCOLOR2 = 0x01;
	putSpriteTile1 (278,208,0,0xc);


	if (fadeLines<24) fadeInPicture0 ();
	else {
		fadeLines=0;
		fadeFrame=0;
	}

	if (fadeFrame <= FRAMEFADE_DELAY) fadeFrame++;
	else fadeFrame =0;

	playMusic();
	// VICII->BORDER_COLOR = 0;

	rasterLineTeddy ();

    *D018 = toD018 (SCREEN0,CHARSET);
    *DD00 = toDd00 (CHARSET);
    *D011 = %00011011;

    *D016 = ((scroll ) & 0x07 ) | 0xc0;

	kickasm {{
		lda #$fc 
		cmp $d012
		bne *-3
	}};

	// VICII->BORDER_COLOR = 1;
    textScroller (scrollText1);
	// VICII->BORDER_COLOR = 0;

	handleTimeline();
    *IRQ_STATUS = IRQ_RASTER;
}

// --------------------------------------------------------------------
// another picture and scroll
// this could have been done better and not just exact copy of previous
// irq handler but I was lazy and nobody probably cares ;)

__interrupt(hardware_all) void irqPic1() {

	*D018 = toD018 (PICTURE1_SCREEN, PICTURE1);
    *DD00 = toDd00 (PICTURE1);
    *D011 = $3b;
    *D016 = $d8;
    VICII->SPRITES_ENABLE = 0xff;
	VICII->SPRITES_MC = 0xff;
	VICII->SPRITES_MCOLOR1 = 0x00;
	VICII->SPRITES_MCOLOR2 = 0x01;
	putSpriteTile1 (278,208,0,0xc);

	if (fadeLines<24) fadeInPicture1 ();
	else {
		fadeLines=0;
		fadeFrame=0;
	}

	if (fadeFrame <= FRAMEFADE_DELAY) fadeFrame++;
	else fadeFrame =0;

	playMusic();
	// VICII->BORDER_COLOR = 0;

	rasterLineTeddy ();

    *D018 = toD018 (SCREEN0,CHARSET);
    *DD00 = toDd00 (CHARSET);
    *D011 = %00011011;
    VICII->BG_COLOR1 = 0;
    VICII->BG_COLOR2 = $e;

    *D016 = ((scroll ) & 0x07 ) | 0xc0;

	kickasm {{
		lda #$fc 
		cmp $d012
		bne *-3
	}};

	// VICII->BORDER_COLOR = 1;
    textScroller (scrollText1);
	// VICII->BORDER_COLOR = 0;

	handleTimeline();
    *IRQ_STATUS = IRQ_RASTER;
}

// --------------------------------------------------------------------
// this is only teddy (sprites) and scroll

volatile byte sinPos = 0;

__interrupt(hardware_all) void irqTeddy() {

    *D018 = toD018 (SCREEN0,CHARSET2);
    *DD00 = toDd00 (CHARSET2);
    VICII->BG_COLOR1 = 0x0f;
    VICII->BG_COLOR2 = 0x0b;

	// VICII->BORDER_COLOR = 2;
    byte m = (byte) mapPosition;
   	*D016 = ((m ^ 0xff) & 0x07 ) | 0xd0;

   	if (introState == STAGE_TEDDY0)	{
   		if (timer < 310 && timer > 0) mapPosition --;
   		if (timer > 600 && timer <800) mapPosition =1600;
   		if (timer > 800){
   			mapPosition = 1000+sinus1[sinPos++];

   		}
   	}


    textScroller (scrollText1);
	// VICII->BORDER_COLOR = 0;

	putSpriteTile1 (278,208,0,0xc);

	playMusic();

	rasterLineTeddy ();

    *D018 = toD018 (SCREEN0,CHARSET);
    *DD00 = toDd00 (CHARSET);
    *D011 = %00011011;
    VICII->BG_COLOR1 = 0;
    VICII->SPRITES_ENABLE = 0xff;
	VICII->SPRITES_MC = 0xff;
	VICII->SPRITES_MCOLOR1 = 0x00;
	VICII->SPRITES_MCOLOR2 = 0x01;

    *D016 = ((scroll ) & 0x07 ) | 0xc0;

	// VICII->BORDER_COLOR = 1;
	moveMap ();
	// VICII->BORDER_COLOR = 0;

	handleTimeline();
    *IRQ_STATUS = IRQ_RASTER;
}

// --------------------------------------------------------------------
__interrupt(hardware_all) void irqTeddyIdle() {

    *D018 = toD018 (SCREEN1,CHARSET2);
    *DD00 = toDd00 (CHARSET2);
    VICII->BG_COLOR1 = 0x0f;
    VICII->BG_COLOR2 = 0x0b;

	// VICII->BORDER_COLOR = 2;
    textScroller (scrollText1);
	// VICII->BORDER_COLOR = 0;
	playMusic();

	putSpriteTile1 (278,208,0,0xc);

	rasterLineTeddy ();

    *D018 = toD018 (SCREEN0,CHARSET);
    *DD00 = toDd00 (CHARSET);
    *D011 = %00011011;
    VICII->BG_COLOR1 = 0;
    VICII->SPRITES_ENABLE = 0xff;
	VICII->SPRITES_MC = 0xff;
	VICII->SPRITES_MCOLOR1 = 0x00;
	VICII->SPRITES_MCOLOR2 = 0x01;

    *D016 = ((scroll ) & 0x07 ) | 0xc0;

	kickasm {{
		lda #$fc 
		cmp $d012
		bne *-3
	}};

	handleTimeline();
    *IRQ_STATUS = IRQ_RASTER;
}

//----------------------------------------------------------
// busy wait for a line 

inline void rasterLineTeddy (){

	kickasm {{
		lda #$e6 
		cmp $d012
		bne *-3
		nop 
		nop 
		nop 
	}};

}

// --------------------------------------------------------------------
// scroll routine

volatile byte scroll =7;
volatile word textPos=0;

void textScroller (byte *ptr){

	byte* scrollLine = SCREEN0+(22*40)+40;
	scroll --;
	if (scroll==$ff){
		scroll=7;
		byte c = ptr[textPos++];
		if (c!=0) {
			scrollLine[35]=c;
			scrollLine[35+40]=c+0x40;
		}
		else textPos=0;

	    for(byte i=0;i!=39;i++) scrollLine[i]=scrollLine[i+1];
	    for(byte i=0;i!=39;i++) scrollLine[40+i]=scrollLine[40+i+1];
	}

}

// --------------------------------------------------------------------
// these are functions to show bitmap pic line by line
// the lines order are stored in a fadeRandoms[] array 
// which... is not random ;)

volatile byte fadeLines = 0;
volatile byte fadeFrame = 0;

export word muls40[] = kickasm {{
	.for (var i=0; i<24; i++){
		.word i*40
	}
}};

export byte fadeRandoms[24] = {0,5,20,15,1,21,6,14,3,17,22,9,12,2,22,4,7,13,19,8,10,18,16,11};

void fadeInPicture0 (){
	word ff=muls40[ fadeRandoms [fadeLines] ];
	for (byte i=0; i<40; i++){
		PICTURE0_SCREEN[ff+i] = PICTURE0[OFFSET0+ff+i];
		COLS[ff+i] = PICTURE0[OFFSET1+ff+i];
	}
	if (fadeFrame==FRAMEFADE_DELAY) fadeLines ++;
}

void fadeInPicture1 (){
	word ff=muls40[ fadeRandoms [fadeLines] ];
	for (byte i=0; i<40; i++){
		PICTURE1_SCREEN[ff+i] = PICTURE1[OFFSET0+ff+i];
		COLS[ff+i] = PICTURE1[OFFSET1+ff+i];
	}
	if (fadeFrame==FRAMEFADE_DELAY) fadeLines ++;
}

// --------------------------------------------------------------------
// some helpfull functions

void clearScreen (char *scrn, char v) {
		
	byte i=0;
	do {
		i++;
		scrn[i+$000]=v;
		scrn[i+$100]=v;
		scrn[i+$200]=v;
		scrn[i+$300-(3*40)]=v;
	}
	while (i!=0);

}

void clearTextArea (byte col) {
	for (byte i=0; i<40*2; i++){
		SCREEN0 [(23*40)+i]=0x20;
	}
	for (byte i=0; i<40; i++){
		COLORRAM[(22*40)+i]=0;
		COLORRAM[(23*40)+i]=col;
		COLORRAM[(24*40)+i]=col;
	}
}

// copied from some of my older projects.
// probably from Robot Jet Action game.
void putSpriteTile1 (word sprPosX, byte sprPosY, byte spr, byte col){
	byte s=spr<<2;
	moveSprite (sprPosX, sprPosY,       spritesTiles[s+0], col, 0);
	moveSprite (sprPosX+24, sprPosY,    spritesTiles[s+1], col, 1);
	moveSprite (sprPosX+48, sprPosY,    spritesTiles[s+2], col, 2);

	moveSprite (sprPosX, sprPosY+21,       spritesTiles[s+3], col, 3);
	moveSprite (sprPosX+24, sprPosY+21,    spritesTiles[s+4], col, 4);
	moveSprite (sprPosX+48, sprPosY+21,    spritesTiles[s+5], col, 5);
}


// this function is not optimal but easy to use to place and move sprite on screen
void moveSprite (word sprPosX, byte sprPosY, byte spr, byte col, byte slot){

    export byte sprites_MSB_1[] = kickasm {{
        .byte %00000001,%00000010,%00000100,%00001000,%00010000,%00100000,%01000000,%10000000
    }};

    export byte sprites_MSB_2[] = kickasm {{
        .byte %11111110,%11111101,%11111011,%11110111,%11101111,%11011111,%10111111,%01111111
    }};

    byte sprxmsb = *SPRITES_XMSB;
    if (sprPosX>255){   
        byte msb = sprites_MSB_1[slot];
        *SPRITES_XMSB = sprxmsb | msb;
    }
    else {
        byte msb = sprites_MSB_2[slot];
        *SPRITES_XMSB = sprxmsb & msb;
    }

    byte s = slot <<1;
    SPRITES_XPOS[s] = (byte)sprPosX;
    SPRITES_YPOS[s] = sprPosY;
    SCREEN0[$03f8+slot] = SPRITESMEM_BASE/64 + spr; 
    SCREEN1[$03f8+slot] = SPRITESMEM_BASE/64 + spr; 
	PICTURE0_SCREEN[$03f8+slot] = SPRITESMEM_BASE/64 + spr;
	PICTURE1_SCREEN[$03f8+slot] = SPRITESMEM_BASE/64 + spr;
    SPRITES_COLOR[slot] = col;
}


// scrolling of the char map. 
// map has to be 256 chars wide to work.
void moveMap (){
	byte j=0;
	byte mp = (byte) (mapPosition >> 3);

	for (char i=mp; i<mp+40; i++){
		SCREEN0[ 40*4  +j ] = MAP1[$000+i];
		SCREEN0[ 40*5  +j ] = MAP1[$100+i];
		SCREEN0[ 40*6  +j ] = MAP1[$200+i];
		SCREEN0[ 40*7  +j ] = MAP1[$300+i];
		SCREEN0[ 40*8  +j ] = MAP1[$400+i];
		SCREEN0[ 40*9  +j ] = MAP1[$500+i];
		SCREEN0[ 40*10 +j ] = MAP1[$600+i];
		SCREEN0[ 40*11 +j ] = MAP1[$700+i];
		SCREEN0[ 40*12 +j ] = MAP1[$800+i];
		SCREEN0[ 40*13 +j ] = MAP1[$900+i];
		SCREEN0[ 40*14 +j ] = MAP1[$a00+i];
		SCREEN0[ 40*15 +j ] = MAP1[$b00+i];
		SCREEN0[ 40*16 +j ] = MAP1[$c00+i];
		SCREEN0[ 40*17 +j ] = MAP1[$d00+i];
		j++;
	}
}

//-----------------------

volatile byte c1=0;
volatile byte c2=0;
volatile byte c3=0;
volatile byte c4=0;

#pragma data_seg(DataHigh)

// --------------------------------------------------------------------
//
// all the resources (gfx, fonts, music, etc used in this little demo
// the scrolled map and the sprites were edited in Charpad and Sprite Pad. 
// both files are included in data dir.

// sprite tiles exported from Sprite Pad - great C64 sprite editor for Windows
export char spritesTiles[] =  kickasm {{
	.var spritestiles = LoadBinary("data/teddy - Tiles.bin")
	.fill spritestiles.getSize(), spritestiles.get(i)
}};


export byte[256] sinus1 = kickasm {{
	.fill 256,round(120+120*sin(toRadians(i*360/256)))
}};


export __address(CHARSET_CONST) char charset[] =  kickasm {{
	.var chars = LoadBinary("data/fnt.bin",BF_C64FILE)
	.fill chars.getSize(), chars.get(i) ^ $ff
}};


export __address (MAP1_CONST) char mapa[] =  kickasm {{
	.var map1 = LoadBinary("data/mdteddy - Map (256x14).bin")
	.fill map1.getSize(), map1.get(i)
}};


export __address (CHARSET2_CONST) char mapchars[] =  kickasm {{
	.var mapch = LoadBinary("data/mdteddy - Chars.bin")
	.fill mapch.getSize(), mapch.get(i)
}};


export __address(PICTURE0_CONST) char picture[] =  kickasm {{
	.var pic = LoadBinary("data/md2.prg",BF_C64FILE)
	.fill pic.getSize(), pic.get(i)
}};


export __address(PICTURE1_CONST) char picture2[] =  kickasm {{
	.var pic2 = LoadBinary("data/katon1.prg",BF_C64FILE)
	.fill pic2.getSize(), pic2.get(i)
}};


export __address(SPRITESMEM_CONST) char spritesmem[] = kickasm {{
	.var sprites = LoadBinary("data/teddy - Sprites.bin")
	.fill sprites.getSize(), sprites.get(i)
}};
export __address(0x0d00) char spritesmem1[] = kickasm {{
	.fill sprites.getSize(), sprites.get(i)
}};
export __address(0x4d00) char spritesmem2[] = kickasm {{
	.fill sprites.getSize(), sprites.get(i)
}};


export __address(0xf800) char screen1[] = kickasm {{
	.fill 40*25, $0
}};


export __address(MUSIC_CONST) char music[] = kickasm {{
	.var mus = LoadSid("data/music.sid")
	.fill mus.size, mus.getData(i)
}};


//=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=

export char scrollText1[] = kickasm {{
	.text "                                     "
	.text "hi c64 demoscene!     time to visit real party in 2022.   there will be not one but two parties in poland this year    ";
	.text "first... moonshine dragons 27-29 may 2022 opole/poland              "
	.text "the biggest c64 only party in poland is confirmed now.   "
	.text "register today!            "
	.text "                                "
	.text "then....               teddy beer party: 29-31 july 2022 tuchola/poland      "
	.text "so remember...    moonshine dragons and teddy beer party     2022 in poland      "
	.text "more info and news on csdb.dk and parties websites.    see you at the party!         "
	.byte 0
}};





