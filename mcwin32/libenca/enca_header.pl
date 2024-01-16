#!/usr/bin/perl
# $Id: enca_header.pl,v 1.1 2023/09/30 05:39:55 cvsuser Exp $
# libenca header import tool
# enca.h importer
#
#
#

BEGIN {
    my $var = $ENV{"PERLINC"};

    if (defined($var) && -d $var) {             # import PERLINC
        my ($quoted_var) = quotemeta($var);
        push (@INC, $var)
            if (! grep /^$quoted_var$/, @INC);

    } elsif ($^O eq "MSWin32") {                # ActivePerl (defaults)
        if (! grep /\/perl\/lib/, @INC) {
            push (@INC, "c:/perl/lib")  if (-d "c:/perl/lib");
            push (@INC, "/perl/lib") if (-d "/perl/lib");
        }
    }
}

use strict;
use Cwd 'realpath', 'getcwd';
use Getopt::Long;
use File::Copy;                                 # copy()
use File::Basename;
use POSIX 'asctime';

my $o_version   = '1.19';
my $o_type      = undef;
my $o_src       = "./enca-1.19/lib";
my $o_output    = undef;

sub ProcessHeader($$);

exit &main();

sub
main()
{
    my $o_help  = 0;

    my $ret
        = GetOptions(
                't|type=s'      => \$o_type,
                's|src=s'       => \$o_src,
                'o|output=s'    => \$o_output,
                'v|version=s'   => \$o_version,
                'h|help'        => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV);

    Usage("expected --type") if (!$o_type);
    Usage("invalid --type, expect 'config', 'enca' or 'internal'")
        if ($o_type ne 'config' && $o_type ne 'enca' && $o_type ne 'internal');

    Usage("expected --src") if (!$o_src);
    Usage("expected --output") if (!$o_output);

    ProcessHeader($o_src, $o_output);
    return 0;
}


sub
Usage                   # (message)
{
    print "\nenca_header @_\n\n" if (@_);
    print <<EOU;

Usage: perl enca_header.pl [options]

Options:
    --help                  Help.
    --src=<path>            libenca file
    --output=<path>         destination directory
    --version=<version>     Magic lib version prefix (eg. 119).

EOU
    exit(42);
}


sub
ProcessHeader($$)       # (src, output)
{
    my ($src, $output) = @_;

    my $in      = "${src}/${o_type}.h.in";
    my $out     = "${output}/${o_type}.h";

    open(FILE, "<${in}") or
        die "cannot open <${in}> : $!\n";
    print "importing: ${in}\n";

    my $cplusplus = 0;
    my $text = '';
    while (<FILE>) {
        if ($_ =~ /define (VERSION|PACKAGE_VERSION|PACKAGE_STRING)/) {
                    #
                    # define VERSION            X.YY
                    # define PACKAGE_VERSION    "X.YY"
                    # define PACKAGE_STRING     "xxx X.YY"
                    #
            if (/\"/) {
                s/X\.YY/${o_version}/;
            } else {
                my $num = $o_version;
                my $num =~ s/\.//;              # x.yy -> xyy
                s/X\.YY/${num}/;
            }

        } elsif ($o_type eq 'enca' || $o_type eq 'internal') {
            if (/^#ifdef __cplusplus/) {
                if (1 == ++$cplusplus) {
                    my $prototype = <<EOT;
/* Windows bindings */
#if !defined(__ENCA_DECL)
# if ((defined __WIN32__) || (defined _WIN32) || defined(__CYGWIN__)) && (!defined LIBENCA_STATIC)
#  ifdef __LIBENCA_BUILD
#   ifdef __GNUC__
#    define __ENCA_DECL __attribute__((dllexport)) extern
#   else
#    define __ENCA_DECL __declspec(dllexport)
#   endif
#  else
#   ifdef __GNUC__
#    define __ENCA_DECL
#   else
#    define __ENCA_DECL __declspec(dllimport)
#   endif
#  endif
# else /*__ENCA_DECL*/
#  define __ENCA_DECL
# endif
#endif /*__ENCA_DECL*/

EOT
                    $text .= $prototype;
                }

            } else {
                # __ENCA_DECL xxx function(xxx)
                # unless within a comment (/* .. or * ..) or #define
                $text .= '__ENCA_DECL '
                    if (/ \*?enca_.*\(/ && !/^[ \t\/]+\*/ && !/\#define/);
            }

        } elsif ($o_type eq 'internal') {
        }
        $text .= $_;
    }
    close FILE;

    print "exporting: ${out}\n";

    open(FILE, ">${out}") or
        die "cannot open <${out}> : $!\n";
    print FILE $text;
    close FILE;
}

#end

