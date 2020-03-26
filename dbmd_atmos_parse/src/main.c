/****************************************************************************
* Copyright (c) 2020, Dolby Laboratories Inc.
* All rights reserved.
*
*	dbmd_atmos_parse.exe
*		Dolby Audio Metadat Wave Chunk, Dolby Atmos Metadata Segment Parse
*
*		This program parses and displays the values of metadata parameters
*		embedded within the Dolby audio metadata (DBMD) wav chunk of the input
*		wav file. Only the Dolby Atmos metadata segment information is
*       displayed.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions
*    and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
*    and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
*    promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define _LARGEFILE_SOURCE

#include "dbmd_atmos_parse.h"
#include "dbmd_text.h"

/* Global Defines */
#define REV_STR "1.0"
#define SIZE_LIMIT 0x40000000u
#define RF64_INDICATION 0xFFFFFFFFu
#define MAX_DBMD_SIZE 6144

/* WAV File Chunk Status Bit Masks */
#define WAV_RIFF_HEADER_MASK 0x01
#define WAV_WAVE_HEADER_MASK 0x02
#define WAV_FMT_CHUNK_MASK 0x04
#define WAV_DATA_CHUNK_MASK 0x08
#define WAV_DBMD_CHUNK_MASK 0x10
#define WAV_AXML_CHUNK_MASK 0x20
#define WAV_DS64_CHUNK_MASK 0x40

/* Local function prototypes */
void show_usage(void);
int parse_wav_header(FILE *in_file);
void display_dbmd_metadata(void);
void display_dbmd_error(int error_code);

/* Global variables */
uint64_t dbmd_chunk_size = 0;
char dolby_metadata[MAX_DBMD_SIZE];
DBMetadata DolbyMetadata;
unsigned char status;

int main(int argc, char **argv)
{
	FILE *inFilePtr;
	char *infilename;
	int dbmd_error;



	/*	Print banner */
	printf("\nDolby Atmos DBMD Parser (Version %s)\n", REV_STR);
	puts("Copyright (C) 2020, Dolby Laboratories Inc.");

	/* Verify input arguments */
	if (argc == 1)
	{
		show_usage();
	}

	/* Retrieve file name from command line input */
	infilename = argv[1];

	/* Open input file */
	inFilePtr = fopen(infilename, "rb");

	if (!inFilePtr)
	{
		printf("\nError opening input file!\n");
		return 1;
	}

	/* Parse input file wave header */
	if (parse_wav_header(inFilePtr))
	{
		/* Test if DBMD chunk was found */
		if ( !(status & WAV_DBMD_CHUNK_MASK) || !dbmd_chunk_size )
		{
			printf("\nError, Dolby audio metadata chunk not found!\n");
		}

		/* Test if AXML chunk was found */
		if ( !(status & WAV_AXML_CHUNK_MASK) )
		{
			printf("\nError, ADM XML chunk not found!\n");
		}		
		
		printf("\nError, file not recognized as valid ADM WAV file!\n");
		return 1;
	}
	/* close file**/
	fclose(inFilePtr);

	/* If a DBMD chunk was found, parse it */
	if ( (dbmd_error = parse_dbmd_metadata(dolby_metadata, (unsigned int)dbmd_chunk_size, &DolbyMetadata)) )
	{
		/* parse dbmd error & display message */
		display_dbmd_error(dbmd_error);
		return 1;
	}

	/* Display dbmd values for all programs */
	display_dbmd_metadata();

	return 0;
}

void display_dbmd_metadata(void)
{
	unsigned int i;
	int is_same_brm = 0;
	int same_brm_count = 0;
	atmos_dbmd_binaural_render_mode brm = ATMOS_DBMD_BINAURAL_RENDER_MODE_NOT_INDICATED;

	printf("\nDolby Audio Metadata Wave Chunk Found\n");

	if (DolbyMetadata.DolbyAtmosSeg.segment_exists)
	{
		printf("\nDolby Atmos Metadata\n");
		printf("   Created by: %s (%d.%d.%d)\n",
			DolbyMetadata.DolbyAtmosSeg.content_creation_tool,
			DolbyMetadata.DolbyAtmosSeg.content_creation_tool_version.major,
			DolbyMetadata.DolbyAtmosSeg.content_creation_tool_version.minor,
			DolbyMetadata.DolbyAtmosSeg.content_creation_tool_version.micro);
		printf("   warp_mode: %s\n", warpmodetext[DolbyMetadata.DolbyAtmosSeg.warp_mode]);
	}
	else
	{
		printf("\nDolby Atmos Metdata\n");
		printf("   Not present. This may indicate that this is not a valid Dolby Atmos ADM file.\n");
	}

	if (DolbyMetadata.DolbyAtmosSupSeg.segment_exists)
	{
		printf("\nDolby Atmos Supplemental Metadata\n");

		/* Determine if same brm is used for all objects */
		for (i = 1; i < DolbyMetadata.DolbyAtmosSupSeg.object_count; i++)
		{
			if (DolbyMetadata.DolbyAtmosSupSeg.binaural_render_mode[i] == DolbyMetadata.DolbyAtmosSupSeg.binaural_render_mode[i-1])
			{
				same_brm_count = same_brm_count + 1;
			}
		}
		if ((same_brm_count + 1) == DolbyMetadata.DolbyAtmosSupSeg.object_count)
		{
			is_same_brm = 1;
			brm = DolbyMetadata.DolbyAtmosSupSeg.binaural_render_mode[0];
		}
		else
		{
			is_same_brm = 0;
		}

		if (is_same_brm == 1)
		{
			if (brm == ATMOS_DBMD_BINAURAL_RENDER_MODE_BYPASS)
			{
				printf("   Headphone metadata present: \n   \tbinaural render mode: %s (all objects have identical value)\n   \tNo binauralization will be applied. \n   \tRecommend editing binaural render mode parameters.\n", binauralrendermodetext[brm]);
			}
			if (brm == ATMOS_DBMD_BINAURAL_RENDER_MODE_NEAR)
			{
				printf("   Headphone metadata present: \n   \tbinaural render mode: %s (all objects have identical value)\n   \tNear binaural render mode will be applied. \n   \tRecommend editing binaural render mode parameters.\n", binauralrendermodetext[brm]);
			}
			if (brm == ATMOS_DBMD_BINAURAL_RENDER_MODE_FAR)
			{
				printf("   Headphone metadata present: \n   \tbinaural render mode: %s (all objects have identical value)\n   \tFar binaural render mode will be applied. \n   \tRecommend editing binaural render mode parameters.\n", binauralrendermodetext[brm]);
			}
			if (brm == ATMOS_DBMD_BINAURAL_RENDER_MODE_MID)
			{
				printf("   Headphone metadata present: \n   \tbinaural render mode: %s (all objects have identical value)\n   \tDefault binaural render mode will be applied.\n", binauralrendermodetext[brm]);
			}
			if (brm == ATMOS_DBMD_BINAURAL_RENDER_MODE_NOT_INDICATED)
			{
				printf("   Headphone metadata not present: \n   \tbinaural render mode: %s (all objects have identical value)\n   \tNo binaural render mode metadata present. Default binaural render mode metadata may be applied. \n   \tRecommend editing binaural render mode parameters.\n", binauralrendermodetext[brm]);
			}
		}
		else
		{
			printf("   Headphone metadata present: \n   \tbinaural render mode: varied (objects have different values)\n");
		}

		/* check trim metadata */
		printf("   Trim Metadata:\n");
		for (i = 0; i < NUM_TRIM_CONFIGS; i++)
		{
			printf("   \tTrim mode: %s, %s trims\n",
				trimmodecfgtext[i],
				trimtypetext[DolbyMetadata.DolbyAtmosSupSeg.trims[i].auto_trim]);
		}
	}
	else
	{
		printf("\nDolby Atmos Supplemental Metadata\n"); 
		printf("   Headphone metadata not present. Default metadata will apply.\n");
		printf("   Trim metadata not present. Default metadata will apply.\n");
	}

	printf("\n");
}

void display_dbmd_error(int error_code)
{
	switch(error_code)
	{
		case DB_ERR_OK:
			break;
		case DB_ERR_NEWERVERSION:
			printf("DBMD Error, unsupported DBMD version!\n");
			break;
		case DB_ERR_BADDASMSSYNC:
			printf("DBMD Error, invalid Dolby Atmos Supplemental Metadata sync!\n");
			break;
		case DB_ERR_TOOMANYOBJS:
			printf("DBMD Error, too many objects!\n");
			break;
		case DB_ERR_DASEGSZ:
			printf("DBMD Error, unsupported segment size for Dolby Atmos!\n");
			break;
		case DB_ERR_DACHECKSUM:
			printf("DBMD Error, checksum failure for Dolby Atmos segment!\n");
			break;
		case DB_ERR_DASCHECKSUM: 
			printf("DBMD Error, checksum failure for Dolby Atmos Supplemental segment!\n");
			break;
		case DB_ERR_DAISATMOS:
			printf("DBMD Error, invalid Dolby Atmos identifier for Dolby Atmos segment!\n");
			break;
	}
}

/*******************************************************************************************
int parse_wav_header(...)
-Purpose:
	Parses the input file wave header, if it exists
-Inputs:
	FILE *in_file	-	input file pointer
-Returns:
	int				-	error code
********************************************************************************************/
int parse_wav_header(FILE *in_file)
{
	char byte_buf[5] = "";
	int b_is_RF64_BW64 = 0;
	int b_ds64_present = 0;
	uint64_t subchunk_size=0; 
	uint64_t num_mini_chunks, mini_chunk_size, leftover_size, mchnk;
	uint64_t data64_chunk_size = 0;
	unsigned int riff_size_low = 0, riff_size_high = 0, data_size_low = 0, data_size_high = 0;
	
	if (in_file != NULL)
	{
		status = 0;	         /* Initialize status variable */
		dbmd_chunk_size = 0; /* Initialize dbmd chunk size */

		/* Read in the first 4 header bytes of the file */
		fread(byte_buf, 1, 4, in_file);

		if ( !strcmp(byte_buf, "RIFF") || !strcmp(byte_buf, "RF64") || !strcmp(byte_buf, "BW64") )	/* if RIFF/RF64/BW64 bytes found */
		{ 
			if ( !strcmp(byte_buf, "RF64") || !strcmp(byte_buf, "BW64") )
			{
				/* Flag that file adheres to RF64/BW64 specification */
				b_is_RF64_BW64 = 1; 
			}
		
			status = status | WAV_RIFF_HEADER_MASK; /* update status */

			fread(&subchunk_size, 1, 4, in_file);	    /* read in size of RIFF/RG64/BW64 chunk */
			fread(byte_buf, 1, 4, in_file);		        /* read in next 4 bytes */

			if (!strcmp(byte_buf,"WAVE"))		        /* if WAVE, continue */
				status = status | WAV_WAVE_HEADER_MASK; /* update status */		
			else								        /* else, error, exit */
				return 1;

			while(1)
			{
				if (fread(byte_buf, 1, 4, in_file) != 4)	/* read next subchunk ID */	
					break;

				subchunk_size = 0; /* reset subchunk_size value */
				if (fread(&subchunk_size, 1, 4, in_file) != 4)	/* read subchunk size */
					break;

				/* sanity check size */
				if ((subchunk_size % 2) && (subchunk_size != RF64_INDICATION))
				{
					subchunk_size++;
				}
				if (subchunk_size == 0)
					return 1;


				/* Read in subchunk based on ID */
				if (!strcmp(byte_buf, "ds64"))	/* DS64 Chunk for RF64/BW64 */
				{
					b_ds64_present = 1;                    /* flag presence of ds64 chunk */
					status = status | WAV_DS64_CHUNK_MASK; /* update status */

					if (subchunk_size < 16)
						return 1;

					/* read in riffSizeLow */
					fread(&riff_size_low, 1, 4, in_file);

					/* read in riffSizeHigh */
					fread(&riff_size_high, 1, 4, in_file);

					/* read in dataSizeLow */
					fread(&data_size_low, 1, 4, in_file);

					/* read in dataSizeHigh */
					fread(&data_size_high, 1, 4, in_file);

					/* combine riffSIzeLow and riffSizeHigh to form actual size */
					data64_chunk_size = ((uint64_t)data_size_high << 32) | (uint64_t)data_size_low;

					/* advance beyond remaining subchunk bytes */
#ifdef WIN32
					_fseeki64(in_file, subchunk_size - 16, SEEK_CUR);
#else
					fseek(in_file, subchunk_size - 16, SEEK_CUR);
#endif 
				}				
				else if (!strcmp(byte_buf,"fmt "))	/* Format Chunk */
				{
					status = status | WAV_FMT_CHUNK_MASK; /* update status */
					
					/* advance beyond remaining subchunk bytes */
#ifdef WIN32
					_fseeki64(in_file, subchunk_size, SEEK_CUR);	
#else
					fseek(in_file, subchunk_size, SEEK_CUR);	
#endif 
				}
				else if (!strcmp(byte_buf,"data")) 
				{
					status = status | WAV_DATA_CHUNK_MASK; /* update status */
					
					if ( (b_is_RF64_BW64 == 1) && (subchunk_size == RF64_INDICATION) )
					{
						subchunk_size = data64_chunk_size; /* rewrite size value using ds64 data size */
					}

					if (subchunk_size > SIZE_LIMIT)
					{
						/* divide large file into mini-chunks */
						num_mini_chunks = subchunk_size / SIZE_LIMIT;
						mini_chunk_size = subchunk_size / num_mini_chunks;
						leftover_size = subchunk_size - (num_mini_chunks * mini_chunk_size);
						for (mchnk = 0; mchnk < num_mini_chunks; mchnk++)
						{
#ifdef WIN32
							_fseeki64(in_file, mini_chunk_size, SEEK_CUR);
#else
							fseek(in_file, mini_chunk_size, SEEK_CUR);
#endif 
						}
					}
					else
					{
						leftover_size = subchunk_size;
					}

					/* advance to end of data chunk */
					if (leftover_size > 0)
					{
#ifdef WIN32
						_fseeki64(in_file, leftover_size, SEEK_CUR);
#else
						fseek(in_file, leftover_size, SEEK_CUR);
#endif 
					}
				}
				else if (!strcmp(byte_buf,"dbmd"))	/* Dolby Audio Metadata Chunk */
				{
					status = status | WAV_DBMD_CHUNK_MASK; /* update status */
					
					/* Check if DBMD is too big */
					if (subchunk_size > MAX_DBMD_SIZE)
					{
						return 1;
					}

					/* Read in the metadata chunk */
					fread(dolby_metadata, 1, subchunk_size, in_file);

					/* Save the chunk size */
					dbmd_chunk_size = subchunk_size;
				}
				else if (!strcmp(byte_buf,"axml"))	/* ADM XML Chunk */
				{
					status = status | WAV_AXML_CHUNK_MASK; /* update status */

					if (subchunk_size > SIZE_LIMIT)
					{
						/* divide large file into mini-chunks */
						num_mini_chunks = subchunk_size / SIZE_LIMIT;
						mini_chunk_size = subchunk_size / num_mini_chunks;
						leftover_size = subchunk_size - (num_mini_chunks * mini_chunk_size);
						for (mchnk = 0; mchnk < num_mini_chunks; mchnk++)
						{
#ifdef WIN32
							_fseeki64(in_file, mini_chunk_size, SEEK_CUR);
#else
							fseek(in_file, mini_chunk_size, SEEK_CUR);
#endif 
						}
					}
					else
					{
						leftover_size = subchunk_size;
					}

					/* advance to end of axml chunk */
					if (leftover_size > 0)
					{
#ifdef WIN32
						_fseeki64(in_file, leftover_size, SEEK_CUR);
#else
						fseek(in_file, leftover_size, SEEK_CUR);
#endif 
					}
				}				
				else
				{
					/* advance beyond unsupported subchunks */
#ifdef WIN32
					_fseeki64(in_file, subchunk_size, SEEK_CUR);
#else
					fseek(in_file, subchunk_size, SEEK_CUR); 
#endif 
				}
			}
			
			if ( (b_is_RF64_BW64 == 1) && (b_ds64_present == 1) )
			{
				/* if we received all necessary subchunks for RF64/BW64 large files */
				if (status == 0x7F) 
					return 0; 
				else
					return 1;
			}
			else 
			{
				/* if we received all necessary subchunks for RIFF files */
				if (status == 0x3F)	/* if we received all necessary subchunks */
					return 0;
				else
					return 1;
			}
		}
		else /* if file does not begin with RIFF/RF64/BW64 bytes */
		{
			return 1;
		}
	}

	return 0;
}

void show_usage(void)
{
	puts("\nUsage: DBMD_ATMOS_PARSE <input ADM WAV file name> \n");

	exit(0);
}