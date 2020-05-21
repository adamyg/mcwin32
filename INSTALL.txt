
   Midnight Commander Win32 Native

      Requirements:

           o gnu-win32.

           o Open Watcom 1.9
                or Microsoft Visual Studio 2013 (or greater).

           o perl.

           o gtk-win32 (optional).

           o Inno Setup.

                Required for the creation of installation packages.

      Components:

           o mcsrc      Midnight Commander source (currently 4.8.21),
                        mc-win32 changes enclosed within "if defined(WIN32)"
                        and/or //WIN32 comment blocks.

           o mcwin32    Windows specific code and build env.

      Build steps:

        a. Install the gnuwin32 tool set or similar. (optional) set the
            PATH environment variable to include the win32 sub-directory which
            contains console versions of several GNU tools, including gmake.

              Source: http://unxutils.sourceforge.net/

        b. Compiler installation

              Either Open-Watcom, install the current Open Watcom 1.9 installation
              or MSVC 2013 or greater is required.  The installation should visible
              within the current PATH.

        c. Perl installation

              A perl installation must be available, examples include ActiveState Perl,
              Strawberry Perl. The installation should visible within the current PATH.

        d. For older builds (pre 4.8.19), install the 'all-in-one-bundle' within the
            root under the directory 'gtk'.

              Source: http://www.gtk.org/download/win32.php

        e. Prime the tree; generate makefiles

             cd mcwin32
             vc2015config.bat or owcconfig.bat

        f. Build,

             [g]make

        g. To create the installer and/or copy the bin tree your install location.

             start mc-inno-setup.iss

=end=
