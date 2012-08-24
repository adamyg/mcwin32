#!/usr/bin/perl
# $Id:$
# Rename global libmagic symbols
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
use Data::Dumper;;

my %x_files = (
        'apprentice',
	'apptype',
	'ascmagic',
	'cdf',
	'cdf_time',
	'compress',
	'encoding',
	'fsmagic',
	'funcs',
	'getline',
	'is_tar',
	'magic',
	'print', 
	'readcdf',
	'readelf',
        'softmagic'
        );

my $o_keep                  = 0;
my $o_version               = '511';
my $o_original              = 0;

sub ProcessDir($);
sub ProcessFile($$);

exit &main();

sub
main()
{
    my $o_clean             = 0;
    my $o_help              = 0;

    my $ret
        = GetOptions(
                'keep'      => \$o_keep,
                'version'   => \$o_version,
                'original'   => \$o_original,
                'help'      => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV);

    ProcessDir('./file');

    return 0;
}


sub
Usage                   # (message)
{
    print "\nmakelib @_\n\n" if (@_);
    print <<EOU;

Usage: perl magic_rename.pl [options] <command>

Options:
    --help                  Help.

    --version=<version>     Magic lib version prefix.

    --original              Reprocess original file images, if they exist.

    --keep                  Keep temporary file images.
EOU
    exit(42);
}



sub
ProcessDir($)           # (dir)
{
    my ($dir) = @_;

    return if (! -d $dir);

    opendir(DIR, $dir) or
        die "error opening dir <$dir> : $!\n";
    my @FILES = readdir(DIR);
    close DIR;

    if (scalar @FILES) {
        mkdir("${dir}/original") if (-d "${dir}/original");

        foreach my $file (@FILES) {
            if ($file =~ /^(.*)\.([^.]+)$/) {
                my ($name, $ext) = ($1, $2);

                if (('c' eq $ext || 'h' eq $ext) || exists $x_files{$name}) {
                    ProcessFile($dir, $file);
                }
            }
        }
    }
}


sub
ProcessFile($$)         # (dir, file)
{
    my ($dir, $file) = @_;
    my $filenameorg = "${dir}/original/${file}";
    my $filename = "${dir}/${file}";
    my $text = '';

    if (!$o_original || !open(FILE, "<${filenameorg}")) {
        open(FILE, "<${filename}") or
            die "cannot open <${filename}> : $!\n";
        print "exporting: ${filename}\n";
    } else {
        print "importing: ${filenameorg}\n";
    }
    while (<FILE>) {
        $text .= $_;
    }
    close FILE;

    $text =~ s/file_/file${o_version}_/g;
    $text =~ s/cdf_/file${o_version}_cdf_/g;
    $text =~ s/file${o_version}_opts\.h/file_opts.h/g;

    $text =~ s/sread/file${o_version}_sread/g;
    $text =~ s/getdelim/file${o_version}_getdelim/g;
    $text =~ s/getline/file${o_version}_getline/g;

    rename($filename, $filenameorg)             # save original
        if (! -f $filenameorg);

    print "exporting: ${filename}\n";

    open(FILE, ">${filename}") or
        die "cannot open <${filename}> : $!\n";
    print FILE $text;
    close FILE;
}

#end










