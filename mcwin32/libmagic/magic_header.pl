#!/usr/bin/perl
# $Id: magic_header.pl,v 1.1 2023/09/20 16:02:47 cvsuser Exp $
# libmagic header import tool
# magic.h.in importer
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

my $o_version   = '545';
my $o_type      = undef;
my $o_src       = "./file-5.45/src";
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
    Usage("invalid --type, expect 'config' or 'magic'")
        if ($o_type ne 'config' && $o_type ne 'magic');

    Usage("expected --src") if (!$o_src);
    Usage("expected --output") if (!$o_output);

    $o_version =~ s/\.//;                       # x.yy -> x.yy

    ProcessHeader($o_src, $o_output);
    return 0;
}


sub
Usage                   # (message)
{
    print "\nmagic_header @_\n\n" if (@_);
    print <<EOU;

Usage: perl magic_header.pl [options]

Options:
    --help                  Help.
    --src=<path>            libmagic source directory (./file-5.45/src).
    --output=<path>         destination directory
    --version=<version>     Magic lib version prefix (eg. 545).

EOU
    exit(42);
}


sub
ProcessHeader($$)       # (src, output)
{
    my ($src, $output) = @_;

    my $in = "${src}/${o_type}.h.in";
    my $out = "${output}/${o_type}.h";

    open(FILE, "<${in}") or
        die "cannot open <${in}> : $!\n";
    print "importing: ${in}\n";

    my $cplusplus = 0;
    my $text = '';
    while (<FILE>) {
        if ($_ =~ /define (MAGIC_VERSION|PACKAGE_VERSION|PACKAGE_STRING)/) {
                    #
                    # define MAGIC_VERSION	X.YY
                    # define PACKAGE_VERSION    "X.YY"
                    # define PACKAGE_STRING     "file X.YY"
                    #
            s/X\.YY/${o_version}/;

        } elsif ($o_type eq 'magic') {
            if (/^#ifdef __cplusplus/) {
                if (1 == ++$cplusplus) {
                    my $prototype = <<EOT;
/* Windows bindings */
#if ((defined __WIN32__) || (defined _WIN32) || defined(__CYGWIN__)) && (!defined LIBMAGIC_STATIC)
# ifdef __LIBMAGIC_BUILD
#  ifdef __GNUC__
#   define __MAGIC_DECL __attribute__((dllexport)) extern
#  else
#   define __MAGIC_DECL __declspec(dllexport)
#  endif
# else
#  ifdef __GNUC__
#   define __MAGIC_DECL
#  else
#   define __MAGIC_DECL __declspec(dllimport)
#  endif
# endif
#else /*__MAGIC_DECL*/
# define __MAGIC_DECL
#endif

EOT
                    $text .= $prototype;
                }

            } else {
                # __MAGIC_DECL xxx function(xxx)
                $text .= '__MAGIC_DECL '
                    if (/ \*?magic_.*\(/);
            }
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

