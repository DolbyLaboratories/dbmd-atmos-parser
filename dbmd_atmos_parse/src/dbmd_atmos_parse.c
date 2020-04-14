/****************************************************************************
* Copyright (c) 2020, Dolby Laboratories Inc.
* All rights reserved.
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

#include <string.h>
#include "dbmd_atmos_parse.h"

/* Global Defines */
#define DOLBYATMOS_METD_SEG        0x09
#define DOLBYATMOS_SUP_METD_SEG    0x0a
#define DASMS_SYNC                 0xf8726fbd
#define DBMD_PARSER_VERSION	0x01000007	/* Parser is consistent with spec version 1.0.0.7 */

/* DBMD segment payload sizes (not including seg ID, size and checksum) */
#define DOLBY_ATMOS_SEG_SZ      248

/* Local function prototypes */
int parse_dolbyatmos_metadata(int seg_size, unsigned char **p_buf, DBMetadata *output);
int parse_dolbyatmos_splml_metadata(int seg_size, unsigned char **p_buf, DBMetadata *output);
int check_version(int version);
int calc_checksum(int seg_size, char *buf);
int unpack(int nbytes, unsigned char **p_bufptr);

/*******************************************************************************************
int parse_dbmd_metadata(...)
-Purpose:
	Parses the Dolby Audio Metadata Chunk and sets the encoder parameters accordingly
-Inputs:
	char *dbmd_chunk	-	Pointer to dbmd chunk buffer
	int dbmd_size		-	Size of buffer
********************************************************************************************/
int parse_dbmd_metadata(char *dbmd_chunk, int dbmd_size, DBMetadata *output)
{	
	int version;			/* DBMD Version Number */
	int segment_id;			/* Metadata Segment ID */
	int segment_size;		/* Metadata Segment Size */
	int error;				/* Error Code */
	unsigned char *p_buf;	/* Metadata Buffer Pointer */
	
	/* Initialize pointer to start of metadata chunk */
	p_buf = (unsigned char *)dbmd_chunk;
	
	/* Unpack version number */
	version = unpack(4, &p_buf);

	/* Verify that we understand this version of the
		Dolby Audio Metadata Chunk */
	if( check_version(version) )
		return DB_ERR_NEWERVERSION;

    /* Clear output data structure before parsing */
	output->DolbyAtmosSeg.segment_exists = 0;
	output->DolbyAtmosSupSeg.segment_exists = 0;

	while(1)
	{
		/* Unpack next metadata segment id */
		segment_id = unpack(1, &p_buf);

		if(segment_id == 0)	/* Signals end of dbmd chunk */
			break;
		else
		{
			/* Unpack metadata segment size */
			segment_size = unpack(2, &p_buf);

			switch(segment_id)
			{
				case DOLBYATMOS_METD_SEG: /* Dolby Atmos Metadata */

					/* Unpack Dolby Atmos Supplemental metadata segment */
					output->DolbyAtmosSeg.segment_exists = 1;
					if ( (error = parse_dolbyatmos_metadata(segment_size, &p_buf, output)) )
						return error;

					break;

				case DOLBYATMOS_SUP_METD_SEG: /* Dolby Atmos Supplemental Metadata */

					/* Unpack Dolby Atmos Supplemental metadata segment */
					output->DolbyAtmosSupSeg.segment_exists = 1;
					if ( (error = parse_dolbyatmos_splml_metadata(segment_size, &p_buf, output)) )
						return error;

					break;

				default:	/* All other segment types */

					/* Advance beyond this segment and checksum*/
					unpack((segment_size + 1), &p_buf);
					break;
			}			

		}

	}	/* while(1) */

	return DB_ERR_OK;
}

/*******************************************************************************************
int parse_dolbyatmos_metadata(...)
-Purpose:
Parses the Dolby Atmos metadata segment in the DBMD metadata chunk
-Inputs:
int seg_size			-	Size of segment
unsigned char **p_buf	-	Address of metadata buffer pointer
********************************************************************************************/
int parse_dolbyatmos_metadata(int seg_size, unsigned char **p_buf, DBMetadata *output)
{
	/* Metadata segment words */
	int temp_int, i;
	int read_count = 0;
	DolbyAtmosSegment *dams;

	/* If unsupported segment size */
	if (seg_size != DOLBY_ATMOS_SEG_SZ)
		return DB_ERR_DASEGSZ;

	/* Verify segment checksum before continuing */
	if ((*p_buf)[seg_size] != calc_checksum(seg_size, (char *)*p_buf))
		return DB_ERR_DACHECKSUM;

	/* setup pointer */
	dams = &output->DolbyAtmosSeg;

	/* Skip past unneeded fields */
	unpack(32, p_buf);
	read_count = read_count + 32; /* update read_count */

	/* content_information() */
	/* content_creation_tool */
	for (i = 0; i < ATMOS_DBMD_CONTENT_CREATION_TOOL_LEN; i++)
	{
		dams->content_creation_tool[i] = (char)unpack(1, p_buf);
		read_count = read_count + 1; /* update read_count */
	}
	dams->content_creation_tool[ATMOS_DBMD_CONTENT_CREATION_TOOL_LEN] = 0; /* null terminate string */

	/* content_creation_tool_version */
	dams->content_creation_tool_version.major = unpack(1, p_buf); /* major */
	dams->content_creation_tool_version.minor = unpack(1, p_buf); /* minor */
	dams->content_creation_tool_version.micro = unpack(1, p_buf); /* micro */
	read_count = read_count + 1 + 1 + 1; /* update read_count */

	/* Skip past unneeded fields */
	unpack(53, p_buf);
	read_count = read_count + 53; /* update read_count */

	/* additional_rendering_metadata() */
	temp_int = unpack(1, p_buf); /* bed_distribution, reserved, warp_mode */
	unpack(15, p_buf); /* reserved */
	read_count = read_count + 1 + 15; /* update read_count */

	/* warp_mode */
	dams->warp_mode = temp_int & 0x7;

	/* Skip past unneeded fields */
	unpack(80, p_buf);
	read_count = read_count + 80; /* update read_count */

	/* Unpack any remaining segment bytes, including the checksum */
	unpack((seg_size - read_count + 1), p_buf);

	return 0;
}


/*******************************************************************************************
int parse_dolbyatmos_splml_metadata(...)
-Purpose:
Parses the Dolby Atmos Supplemental metadata segment in the DBMD metadata chunk
-Inputs:
int seg_size			-	Size of segment
unsigned char **p_buf	-	Address of metadata buffer pointer
********************************************************************************************/
int parse_dolbyatmos_splml_metadata(int seg_size, unsigned char **p_buf, DBMetadata *output)
{
	/* Metadata segment words */
	unsigned int read_count = 0;
	int sync, object_count;
	int auto_trim;
	int cfg, obj, temp_int;
	DolbyAtmosSupplementalSegment *dasms;

	/* Verify segment checksum before continuing */
	if ((*p_buf)[seg_size] != calc_checksum(seg_size, (char *)*p_buf))
		return DB_ERR_DASCHECKSUM;

	/* setup pointer */
	dasms = &output->DolbyAtmosSupSeg;

	/* check sync */
	sync = unpack(4, p_buf);
	read_count = read_count + 4; /* update read_count */
	if (sync != DASMS_SYNC)
	{
		return DB_ERR_BADDASMSSYNC;
	}

	/* parse object_count */
	object_count = unpack(2, p_buf);
	read_count = read_count + 2; /* update read_count */
	if (object_count > MAX_OBJECT_COUNT)
	{
		return DB_ERR_TOOMANYOBJS;
	}
	dasms->object_count = object_count;
	
	/* parse trim metadata */
	unpack(1, p_buf); /* reserved */
	read_count = read_count + 1; /* update read_count */

	for (cfg = 0; cfg < NUM_TRIM_CONFIGS; cfg++)
	{
		/* auto_trim */
		auto_trim = unpack(1, p_buf) & 0x01; /* reserved + auto_trim */
		read_count = read_count + 1; /* update read_count */
		dasms->trims[cfg].auto_trim = auto_trim;

		/* Skip past unneeded fields */
		unpack(14, p_buf);
		read_count = read_count + 14; /* update read_count */
	}

	/* Skip past unneeded fields */
	for (obj = 0; obj < object_count; obj++)
	{
		unpack(1, p_buf);
		read_count = read_count + 1; /* update read_count */
	}

	/* headphone metadata */
	for (obj = 0; obj < object_count; obj++)
	{
		temp_int = unpack(1, p_buf);
		read_count = read_count + 1; /* update read_count */
		dasms->binaural_render_mode[obj] = temp_int & 0x7;
	}

	/* Unpack any remaining segment bytes, including the checksum */
	unpack((seg_size - read_count + 1), p_buf);

	return 0;
}


/*******************************************************************************************
int check_version(...)
-Purpose:
	Verifies that the Dolby Audio Metadata Chunk version present in the input file
	is not a newer version than the parser is current to.
-Inputs:	
	int version		-	DBMD chunk version of input file
********************************************************************************************/
int check_version(int version)
{
	int i_parser_version = DBMD_PARSER_VERSION;
	unsigned char *file_version = (unsigned char *)&version;
	unsigned char *parser_version = (unsigned char *)&i_parser_version;
    int i;

	for(i = 4; i > 0; i--)
	{
		if(file_version[i - 1] > parser_version[i - 1])
			return -1;
		else if(file_version[i - 1] == parser_version[i - 1])
			continue;
		else
			return 0;
	}

	return 0;
}

/*******************************************************************************************
int calc_checksum(...)
-Purpose:
	Calculates the checksum of the metadata segment
-Inputs:	
	int seg_size	-	Size of segment
	char *buf		-	Pointer to segment buffer
********************************************************************************************/
int calc_checksum(int seg_size, char *buf)
{
	char data;
    int j;

	/* initialize the checksum to the metadata segment size */
	unsigned char sum = seg_size;

	/* perform checksum on the metadata segment payload including unused bits */
	for (j = 0; j < seg_size; j++)
	{
		data = *buf++;
		sum = (sum + data) & 0xFF;
	}

	/* take the 2's complement of the running checksum */
	sum = ((~sum) + 1) & 0xFF;

	return sum;
}

/*******************************************************************************************
int unpack(...)
-Purpose:
	Unpacks the requested number of bytes from the input buffer and advances the
	buffer pointer.  The unpacked bytes are formed into a data word nbytes wide.
-Inputs:	
	int nbytes					-	Number of bytes to unpack
	unsigned char **p_bufptr	-	Address of metadata buffer pointer
********************************************************************************************/
int unpack(int nbytes, unsigned char **p_bufptr)
{
	unsigned int data = 0;
    int i;

	/* Unpack data values, LSB->MSB ordering */
	for(i = 0; i < nbytes; i++)
		data += (*(*p_bufptr)++) << (i * 8);

	return(data);
}
