/* listd64.c 																						*/
/* Written by Martin Sigley, GroundhoGrafix 3/28/2026. 	*/
/* groundhografix@mailbox.org 													*/

#include<stdio.h>
#include<conio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include "listd64.h"

int lastError;											/* If an error occurs check here. */
FILE *dirlistfp;										/* Directory list file pointer. 	*/
char *dirlistfilename = "dir.lst";	/* Directory list file name. 			*/

/* Set lastError to an error or no error. See listd64.h for #define'd errors. */
void setError( int error )
{
	lastError = error;
}

/* Prints an error message based off of the lastError variable to the console. */
void prError( void )
{
	switch(lastError)
	{
		case ER_NO_ERROR:
			printf("No error. %d\n", lastError);
			break;
		case ER_CANT_OPEN_FILE:
			printf("Can't open file. %d\n", lastError);
			break;
		case ER_UNSUPPORTED_FILE:
			printf("Unsupported file. %d\n", lastError);
			break;
		case ER_OUT_OF_MEMORY:
			printf("Out of memory. %d\n", lastError);
			break;
		case ER_FILE_READ_ERROR:
			printf("Error reading file. %d\n", lastError);
			break;
	}
	return;
}

/* Converts a track and block reference to a file position address. */
/* d64Init() must be called before using this function.							*/
unsigned d64tba( int track, int block )
{
	int trackCount;
	long address = 0;
	
	for(trackCount = 1; trackCount < track; trackCount++)
			address += d64track[trackCount] * 256;
		
	return address + block * 256;
}

/* Creates an array with the number of blocks per track on a 35 track .d64. */
void d64Init( void )
{
	int track = 0;
	
	d64track[0]=0;
	
	for(track=1; track < 18; track++)
		d64track[track]=21;
	for(track=18; track < 25; track++)
		d64track[track]=19;
	for(track=25; track < 31; track++)
		d64track[track]=18;
	for(track=31; track < 36; track++)
		d64track[track]=17;
}

/* Loads a d64 image file. */
/* Check lastError for problems if the d64 does not load. */
D64Image* d64LoadImage( char *fn )
{
		FILE *f = NULL;					/* .d64 file handle																	*/
		byte *d = NULL;					/* Contents of the .d64 file												*/
		long size;							/* Size of the .d64 file														*/
		D64Image* disk = NULL;	/* All of the above get assigned to this structure 	*/
		
		setError(ER_NO_ERROR);	/* Clear errors */
		
		/* Open .d64 file */
		if(f=fopen(fn, "rb"))
		{
			/* Get the size of the image file */
			fseek( f, 0L ,SEEK_END );
			size = ftell(f);
			rewind(f);
			
			/* Make sure the file size matches the size of supported image files.	*/
			switch(size)
			{
				/* Add more file sizes to support more formats here. 								*/
				/* No other changes in this function should be needed. 							*/
				/* Define the new sizes in the header file of course.								*/
				case D64_SIZE:			/* Shouldn't this be D41? Oh well.						 	*/
		/*	case D71_SIZE: 	*/	/* Uncomment to add more supported image files 	*/
		/*	case D81_SIZE:	*/
					
					/* Allocate memory for the file */
					if(d = (byte*) malloc(size * sizeof(byte)))
					{
						/* Read the file to memory */
						if( size == fread(d, sizeof(byte), size, f))
						{
							fclose(f);
							
							/* Allocate a D64Image to return */
							if(disk = (D64Image*) malloc(sizeof(D64Image)))
							{
								/* Assign everything to the D64Image */
								disk->size = size;	/* This identifies the image type. */
								disk->image = d;
								strncpy(disk->filename, fn, MAX_FILE_NAME_LENGTH);
								return disk;
							}
							else
								setError(ER_OUT_OF_MEMORY);
						}
						else
							setError(ER_FILE_READ_ERROR);

						free(d);
					}
					else
						setError(ER_OUT_OF_MEMORY);
					break;
				default:
					setError(ER_UNSUPPORTED_FILE);
					break;
			}
			fclose(f);
		}
		else
			setError(ER_CANT_OPEN_FILE);
		
	prError();		/* Prints errors out */	
	return NULL;
}

/* Deallocates memory used for a previously loaded d64 image. */
void d64UnloadImage( D64Image* d )
{
	if(d)
	{
		free(d->image);
		free(d);
	}
}

/* Print's a directory listing to the screen and to a file. */
int d64ListDirectory( D64Image* disk )
{
	int t = 18, b = 0;
	int cnt, cpos;
	unsigned blocksused = 0;
	D64Header *head;
	D64FileEntry *file;
	
	/* File name extensions, note these are in order from 0x80 which is the value		*/
	/* for DEL. To use these you must subtract 0x80 from the fileType member in 		*/
	/* D64FileEntry structure.																											*/
	char *ext[]={"del", "seq", "prg", "usr", "rel"};
	
	/* Disk Header which has disk name, disk id and dos type. Refer to listd64.h		*/
	/* for all members of this structure. 																					*/
	head = (D64Header*) (disk->image + d64tba(t,b));
	
	/* Typically this will be track 18, block 1 for the first directory block. 			*/
	/* However, if this is not standard for some reason, we are still covered by    */
	/* following the block chain.																										*/
	t = head->track;
	b = head->block;
	
	/* Print disk name, pc2asc[x] converts petscii to ascii.												*/
	/* Otherwise an unwanted symbol will be printed.																*/
	printf("0 \"");
	fprintf(dirlistfp, "0 \"");
	
	for(cpos=0; cpos < 17; cpos++)
	{
		putchar(pc2asc[head->diskName[cpos]]);
		fputc(pc2asc[head->diskName[cpos]], dirlistfp);
	}
	
	/* Print a close quote after the disk name. 																		*/
	printf("\" ");
	fprintf(dirlistfp, "\" ");
	
	/* Print disk ID.																																*/
	for(cpos=0; cpos < 2; cpos++)
	{
		putchar(pc2asc[head->id[cpos]]);
		fputc(pc2asc[head->id[cpos]], dirlistfp);
	}
	
	putchar(' ');
	fputc(' ', dirlistfp);
	
	/* Print disk dos type, typically 2A. 																					*/
	for(cpos=0; cpos < 2; cpos++)
	{
		putchar(pc2asc[head->dosType[cpos]]);
		fputc(pc2asc[head->dosType[cpos]], dirlistfp);
	}
	
	putchar('\n');
	fputc('\n', dirlistfp);
	
	/* Follow and print the directory block chain, each block has 8 entries. 				*/
	while(t)
	{
		/* Getting the offset into the file for the directory block. 									*/
		file = (D64FileEntry*) (disk->image + d64tba(t,b));
		
		/* Get the track and block link to the next directory block. 									*/
		/* Note: Only the first directory entry in a directory block contains the			*/
		/* track and block link to the next directory block. The final directory 			*/
		/* block will have a track entry of 0.																				*/
		t = file->track;
		b = file->block;
		
		/* Loop through all directory entries in the directory block.									*/
		for(cnt=0; cnt < 8; cnt++)
		{
			/* Print files except files marked as DELeted 0x80.													*/
			if(file[cnt].fileType-0x80 > 0 && file[cnt].fileType-0x80 < 5)
			{
				/* Print the filesize.																										*/
				printf("%-5d\"",  *((uint16_t*) file[cnt].fileSize));
				fprintf(dirlistfp, "%-5d\"",  *((uint16_t*) file[cnt].fileSize));
				blocksused += *((uint16_t*) file[cnt].fileSize);
				
				/* Print the file name. 																									*/
				for(cpos=0; cpos < 17 && 160!=file[cnt].fileName[cpos]; cpos++)
				{
					putchar(pc2asc[file[cnt].fileName[cpos]]);
					fputc(pc2asc[file[cnt].fileName[cpos]], dirlistfp);
				}
				
				/* Print a closing parenthesis after the file name.												*/
				putchar('\"');
				fputc('\"', dirlistfp);
				
				for(; cpos < 18; cpos++)
				{
					putchar(' ');
					fputc(' ', dirlistfp);
				}
				
				/* Print the file type extension, PRG, SEQ, etc and a new line						*/
				printf("%s\n", ext[file[cnt].fileType-0x80]);
				fprintf(dirlistfp, "%s\n", ext[file[cnt].fileType-0x80]);
			}
		}
	}
	/* Print blocks free after the directory listing.																*/
	printf("%-d blocks Free.\n", 664-blocksused);
	fprintf(dirlistfp, "%-d blocks Free.\n", 664-blocksused);		
	
	return 0;
}

int main( int argc, char **argv )
{
	int carg = 1;		/* Current argument 		*/
	D64Image* d;		/* Disk image 					*/
	
	/* This sets up the number of blocks per track for d64tba()											*/
	d64Init();
	
	/* Open a file to output the directory to. 																			*/
	if(!(dirlistfp=fopen(dirlistfilename, "w")))
	{
		printf("Error: Can't open directory listing file %s.\n\n", dirlistfilename);
		if(!(dirlistfp=fopen("nul", "w")))
		{
			printf("Can't open nul file. Program aborted.\n");
			exit(-2);
		}
	}

	/* Command line arguments must be 2 or more, loop through them.									*/
	while( carg < argc )
	{
		if('-'!=argv[carg][0])
		{
			/* Load the .d64 image file name from the arguments.												*/
			if(d = d64LoadImage(argv[carg]))
			{
				/* Print the directory listing.																						*/
				d64ListDirectory(d);
				
				/* Deallocate the disk image.																							*/
				d64UnloadImage(d);
			}
			/* Print 2 spacing lines between listings */
			printf("\n\n");
			fprintf(dirlistfp, "\n\n");
			
			
			/* On to the next argument.																									*/
			carg++;
		}
	}
	
	/* Close the output directory listing file */
	fclose(dirlistfp);
	
	/* Did not recieve enough arguments. Print usage.																*/
	if(1 == carg)
	{
		printf("Usage: %s filename.d64 [more file names...]", argv[0]);
	}
	else
	{
		printf("Press key when done.\n");
		getch();
	}
	
	return 0;
}