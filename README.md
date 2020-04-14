# Dolby Atmos DBMD Parser (dbmd_atmos_parse)
# version 1.1

dbmd_atmos_parse is a command line utility that parses the Dolby Audio Metadata (DBMD) chunk in a Dolby Atmos Audio Definition Model (ADM) WAV file. The tool displays the status of some Dolby Atmos metadata, including:

- Headphone rendering metadata

- Trim metadata per output speaker configuration

- Warp mode setting

The primary purpose of this code is to help users that need to assess the state of the metadata in an ADM WAV file. Note that this tool only parses the Dolby Atmos metadata segments in the DBMD chunk, it does not parse the ADM XML.

For more information see the release notes.

## Getting Started

These instructions will help you get a copy of the project up and
running on your local machine for development and testing purposes.

### Folder Structure

- **README.md** This file.

- **ReleaseNotes.md** Release notes.

- **LICENSE** Terms of use.

- **dbmd_atmos_parse/** Main application folder.

    - **src/** Source code.

    - **make/** Build files.

- **sample_files/** Sample Dolby Atmos ADM WAV files.

### Prerequisites

For Linux and OSX, the library and tool can be built using GNU makefiles. For windows, Visual Studio 2017 projects and solutions are provided. For all platforms, 64-bit targets are supported. 

### Build instructions

#### Using the GNU makefiles

Use the makefiles located in dbmd_atmos_parse/make/. Go to the appropriate directory and run GNU make. Executables are created in the bin/ directory within the same directory as the makefile.

#### Using Microsoft Visual Studio (on Windows)

Go to the Windows MSVS directory under dbmd_atmos_parse/make/. In Visual Studio 2017, open the solution file (.sln). Select build solution in Visual Studio. The executable is created in the bin/ directory within the same directory as the solution file.

## Running the tool

dbmd_atmos_parse is a command line utility and the usage is provided by running the tool with no options.

```
Dolby Atmos DBMD Parser (Version 1.1)
Copyright (C) 2020, Dolby Laboratories Inc.

Usage: DBMD_ATMOS_PARSE <input ADM WAV file name> 

```

## Sample Files and Output

To test the basic functionality of the tool, sample ADM WAV files with varying metadata have been provided. These files can be found in the sample_files/ directory. For each sample WAV file, there is a corresponding text file with output from the tool. These files can be used for debugging purposes or verify any modifications.

## Release Notes

See the [Release Notes](ReleaseNotes.md) file for additional details.

## License

This project is licensed under the BSD-3 License - see the [LICENSE](LICENSE) file for details


