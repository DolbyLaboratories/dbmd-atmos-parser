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

/* This defines the Metadata as parsed from the wave 
 *  metadata chunk
 */
#define MAX_OBJECT_COUNT 128
#define NUM_TRIM_CONFIGS 9

enum {
    DB_ERR_OK = 0,
    DB_ERR_NEWERVERSION = -1, /* Can't understand Metadata because its a newer version */
	DB_ERR_BADDASMSSYNC = -9, /* Bad DASMS Sync value */
	DB_ERR_TOOMANYOBJS = -10, /* Too many objects */
	DB_ERR_DASEGSZ = -11,     /* Unsupored segment size for Dolby Atmos Segment */
	DB_ERR_DACHECKSUM = -12,  /* Bad checksum for Dolby Atmos Segment */
	DB_ERR_DASCHECKSUM = -13  /* Bad checksum for Dolby Atmos Supplemental Segment */
};

typedef enum
{
	ATMOS_DBMD_WARP_MODE_NORMAL = 0x0,
	ATMOS_DBMD_WARP_MODE_WARPING = 0x1,
	ATMOS_DBMD_WARP_MODE_DOWNMIX_PLIIX = 0x2,
	ATMOS_DBMD_WARP_MODE_DOWNMIX_LORO = 0x3,
	ATMOS_DBMD_WARP_MODE_NOT_INDICATED = 0x4,
} atmos_dbmd_warp_mode;

typedef struct
{
	int major;
	int minor;
	int micro;
} atmos_dbmd_version;

#define ATMOS_DBMD_CONTENT_CREATION_TOOL_LEN 64

typedef struct
{
	unsigned int segment_exists; 

	/* content_information */
	char content_creation_tool[ATMOS_DBMD_CONTENT_CREATION_TOOL_LEN + 1]; 
	atmos_dbmd_version content_creation_tool_version;

	/* additional_rendering_metadata */
	atmos_dbmd_warp_mode warp_mode;
} DolbyAtmosSegment;

struct trim_mode
{
	unsigned int auto_trim;
};

typedef enum
{
	ATMOS_DBMD_BINAURAL_RENDER_MODE_BYPASS = 0x00,
	ATMOS_DBMD_BINAURAL_RENDER_MODE_NEAR = 0x01,
	ATMOS_DBMD_BINAURAL_RENDER_MODE_FAR = 0x02,
	ATMOS_DBMD_BINAURAL_RENDER_MODE_MID = 0x03,
	ATMOS_DBMD_BINAURAL_RENDER_MODE_NOT_INDICATED = 0x04
} atmos_dbmd_binaural_render_mode;

typedef struct
{
	unsigned int segment_exists; 
	unsigned int object_count;
	atmos_dbmd_binaural_render_mode binaural_render_mode[MAX_OBJECT_COUNT];
	struct trim_mode trims[NUM_TRIM_CONFIGS];
} DolbyAtmosSupplementalSegment;

typedef struct 
{
	DolbyAtmosSegment DolbyAtmosSeg;
	DolbyAtmosSupplementalSegment DolbyAtmosSupSeg;
} DBMetadata;

int parse_dbmd_metadata(char *dbmd_chunk, int dbmd_size, DBMetadata *output);
