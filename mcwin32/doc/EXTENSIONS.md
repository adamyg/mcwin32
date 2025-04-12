# Extension

## Wrapper scripts

On exit, the shell shall return Midnight Commander to the same directory it was started from, instead of the last active directory.
Using the ```--printwd``` command line option wrapper scripts may inherit the current directory on exit.

```
-P file, --printwd=file
```

Print the last working directory to the specified file.  This option is not meant to be used directly.  Instead, it's used from a special shell script that automatically changes the current directory of the shell to the last directory Midnight Commander was in. 

Several bundled implementations are available within the installation under the sub-directory _libexec_.

For example, for use within a command shell setup an aliases:

```
doskey mc=call "%ProgramFiles(x86)%\Midnight Commander\libexec\mc-wrapper.bat" $*
```


## Opening files

Midnight Commander reads the MC_XDG_OPEN environment variable to open files, which defaults to _mcstart_ when unset; mcstart is the equivalent of the window _start_ command. 


## Application mappings

Word, Excel and PowerPoint, mapping:

```
# Microsoft Word Document
regex/i/\.(do[ct]|wri|docx)$
	Open=cmd /c start winword %p
	View=cmd /c start winword %p
type/^Microsoft\ Word
	Open=cmd /c start winword %p
	View=cmd /c start winword %p

# Microsoft Excel Worksheet
regex/i/\.(xl[sw]|xlsx)$
	Open=cmd /c start excel %p
	View=cmd /c start excel %p
type/^Microsoft\ Excel
	Open=cmd /c start excel %p
        View=cmd /c start excel %p

# Microsoft PowerPoint Presentation
regex/i/\.(pp[ts]|pptx)$
	Open=cmd /c start powerpnt %p
```
