//
// Midnight Commander, hosturl's
// April 2025
//

static const char
#if defined(_WIN64) || defined(_M_AMD64) // x64; or as channel?
        *hosturl2 = "https://sourceforge.net/projects/mcwin32/files/mcwin32x64.manifest/download", // legacy.
        *hosturl1 = "https://api.github.com/repos/adamyg/mcwin32~mcwin32x64.manifest"; // primary
#else
        *hosturl2 = "https://sourceforge.net/projects/mcwin32/files/mcwin32.manifest/download", // legacy
        *hosturl1 = "https://api.github.com/repos/adamyg/mcwin32~mcwin32.manifest"; // primary
#endif

//end
