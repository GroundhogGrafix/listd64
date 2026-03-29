/* listd64.h */
/* Written by Martin Sigley, GroundhoGrafix 3/28/2026. 	*/
/* groundhografix@mailbox.org 													*/

#define MAX_FILE_NAME_LENGTH 260

#define D64_BLKS		683
#define D64_SIZE 	(256 * D64_BLKS)
/* #define D71_SIZE 349696	*/
/* #define D81_SIZE 819200	*/

#define BLOCK_SIZE 256
#define DIR_ENTRIES 8
#define FILE_NAME_WIDTH 16

#define ER_NO_ERROR					0
#define ER_CANT_OPEN_FILE 	-100
#define ER_UNSUPPORTED_FILE -101
#define ER_OUT_OF_MEMORY		-102
#define ER_FILE_READ_ERROR	-103

typedef unsigned char   byte;

int d64track[35+1]; // Plus 1 because track 0 does not exist.

/* Conversion table from Petscii to Ascii, altered a bit to match Power C 		*/
/* Characters 0 and 160 were set to 32 (space) specifically for this program. */
char pc2asc[] =
{      /*   0     1     2     3      4 */
      /*   ----------------------------*/
/*   0 */   32,    0,    0,    0,    0,
/*   5 */   0,     0,    0,    0,    0,
/*  10 */   0,     0,    0,   10,    0,
/*  15 */   0,     0,    0,    0,    0,
/*  20 */   0,     0,    0,    0,    0,
/*  25 */   0,     0,    0,    0,    0,
/*  30 */   0,     0,    32, '!',   34,
/*  35 */ '#',   '$',   '%', '&', '\'',
/*  40 */  40,    41,   '*', '+',  ',',
/*  45 */  '-',  '.',   '/', '0',  '1',
/*  50 */  '2',  '3',   '4', '5',  '6',
/*  55 */  '7',  '8',   '9', ':',  ';',
/*  60 */  '<',  '=',   '>', '?',  '@',
/*  65 */  'a',  'b',   'c', 'd',  'e',
/*  70 */  'f',  'g',   'h', 'i',  'j',
/*  75 */  'k',  'l',   'm', 'n',  'o',
/*  80 */  'p',  'q',   'r', 's',  't',
/*  85 */  'u',  'v',   'w', 'x',  'y',
/*  90 */  'z',   91,  '\\',  93,  '^',
/*  95 */   0,     0,    0,    0,    0,
/* 100 */   0,     0,    0,    0,    0,
/* 105 */   0,     0,    0,    0,    0,
/* 110 */   0,     0,    0,    0,    0,
/* 115 */   0,     0,    0,    0,    0,
/* 120 */   0,     0,    0,    0,    0,
/* 125 */   0,     0,    0,    0,    0,
/* 130 */   0,     0,    0,    0,    0,
/* 135 */   0,     0,    0,    0,    0,
/* 140 */   0,     0,    0,    0,    0,
/* 145 */   0,     0,    0,    0,    0,
/* 150 */   0,     0,    0,    0,    0,
/* 155 */   0,     0,    0,    0,    0,
/* 160 */   32,    0,    0,    0,   95,
/* 165 */   0,     0,    0,    0,    0,
/* 170 */   0,     0,    0,    0,    0,
/* 175 */   0,     0,    0,    0,    0,
/* 180 */   0,     0,    0,    0,    0,
/* 185 */   0,     0,    0,    0,    0,
/* 190 */   0,     0,    0,  'A',  'B',
/* 195 */  'C',  'D',  'E',  'F',  'G',
/* 200 */  'H',  'I',  'J',  'K',  'L',
/* 205 */  'M',  'N',  'O',  'P',  'Q',
/* 210 */  'R',  'S',  'T',  'U',  'V',
/* 215 */  'W',  'X',  'Y',  'Z',  123,
/* 220 */   0,   125,    0,  '|',    0,
/* 225 */   0,     0,    0,    0,    0,
/* 230 */   0,     0,    0,    0,    0,
/* 235 */   0,     0,    0,    0,    0,
/* 240 */   0,     0,    0,    0,    0,
/* 245 */   0,     0,    0,    0,    0,
/* 250 */   0,     0,    0,    0,    0,
/* 255 */   0
};

typedef struct
{
	byte blksFree;
	byte blks0to7;
	byte blks8to16;
	byte blks17to23;
} D64BAM;

typedef struct D64Header
{
        byte            track;
				byte						block;
        byte            dosVersion;
        byte            unused1;
        D64BAM					bam[35];												// byte            bam[BAM_SIZE];
        byte            diskName[16];
				byte						shiftedSpace[2];
        byte            id[2];
        byte            unused2;                    // Usually $A0
        byte            dosType[2];
        byte            unused3[4];                 // Usually $A0
        byte            track40Extended[55];        // Usually $00, except for 40 track format
} D64Header;

typedef struct
{
        byte            track;		               		// Track and block of next directory
        byte            block;                 			// block. When the first byte is 00
                                                    // that is the last block.             

        byte            fileType;
                                                    // 0x80 = DELeted                       
                                                    // 0x81 = SEQuential                    
                                                    // 0x82 = PROGram
                                                    // 0x83 = USER
                                                    // 0x84 = RELative

        byte            dataBlock[2];               // Track and block of first data block
        byte            fileName[16];               // Filename padded with spaces
        byte            sideSector[2];              // Relative only track and block first side
                                                    // sector.                             

        byte            recordSize;                 // Relative file only. Record Size.                     
        byte            unused[6];                  // Unused bytes                                        

        byte            fileSize[2];                // Number of blocks in file. Low Byte, High Byte.             
} D64FileEntry;

typedef struct
{
	char filename[MAX_FILE_NAME_LENGTH];
	unsigned size;
	byte *image;
} D64Image;

unsigned 	d64tba					( int track, int block );		// Track & Block to Address
void 			d64Init					( void );										// Setup d64track data
D64Image* d64LoadImage		( char *fn );								// Load a D64 image
void 			d64UnloadImage	( D64Image* d );						// Deallocate a D64 image
int 			d64ListDirectory( D64Image* disk );					// Print a directory listing