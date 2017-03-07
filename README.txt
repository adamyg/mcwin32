
   Midnight Commander Win32 Native

      Requirements:

           o Open Watcom 1.9
              or Microsoft Visual Studio 2013 (or greater)

           o gnu-win32

           o gtk-win32

           o Inno Setup

                Required for the creation of installation packages.


      Build steps:

        a. Install the gnuwin32 tool set or similar. (optional) set the
            PATH environment variable to include the win32
            sub-directory which contains console versions of several
            GNU tools, including gmake.

              Source: http://unxutils.sourceforge.net/

        b. Compiler installation

              either Open-Watcom, install the current Open Watcom 1.9 installation.
              or MSVC 2013 or greater is required.

        c. Install the 'all-in-one-bundle' within the root under
            the directory 'gtk'.

              Source: http://www.gtk.org/download/win32.php

        d. Prime the tree; generate makefiles

             cd mcwin32
             vc2015config.bat or owcconfig.bat

        e. Build the entire tree,

             [g]make

        f. To create the installer,

             start mc-inno-setup.iss

=end=

