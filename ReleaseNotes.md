Dolby Atmos DBMD Parser 1.1 release notes
=======================

Dolby Atmos provides greater flexibility to create, transmit, and present content. Through the use of objects, an immersive experience can be provided. Each object is an audio signal plus its associated object audio metadata that contains individually-assigned object properties. The objects properties more explicitly specify how the content creator intends the audio content to be rendered to loudspeakers or headphones. 

ITU-R BS.2076 Audio Definition Model (ADM) specifies a metadata model to ensure compatibility for content production and program exchange systems that support object-based audio and its metadata.

Dolby Atmos can be carried within an ADM-compliant WAV file. In addition to carrying object audio metadata via the ADM chunk, Dolby Atmos ADM files also carry additional metadata (e.g. headphone rendering metadata). This additional metadata is contained in the Dolby Audio metadata (DBMD) chunk in the encapsulating WAV file. The DBMD chunk adheres to the specification defined in EBU Tech 3285 Supplement 6: BWF - Dolby Metadata.

The Dolby Atmos DBMD Parser tool parses the DBMD chunk and displays some of its metadata.

Supported/Tested Platforms:
---------------------------
- Windows, Microsoft Visual Studio 2017 compiler
- OSX, GNU compiler
- Linux, GNU compiler

Known issues:
-------------
- The tool does not validate the elements of the ADM XML chunk, only the existance of the ADM XML chunk is determined.

Version 1.0:
--------------------
- Initial release.

Changes since 1.0:
--------------------
- Removed parsing of unsused metadata fields. 
