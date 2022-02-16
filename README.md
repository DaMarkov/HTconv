# HTconv

Converter for .htc and .hts files.
Can read .htc files.
Read and write .hts files.
Replace textures in .hts files etc.

# Building

Open the solution file with Visual Studio.
Use Win32 as the configuration and build.

## Usage

Examples:

`htconv -open zelda.htc -save zelda.hts`

Opens the file `zelda.htc` and exports it to `zelda.hts`.

`htconv -open zelda.hts -export export_dir`

Opens the file `zelda.hts` and exports all textures in the directory `export_dir`.