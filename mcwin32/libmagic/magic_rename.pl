#!/usr/bin/perl
# $Id: magic_rename.pl,v 1.4 2021/12/01 12:58:46 cvsuser Exp $
# libmagic import tool
# Rename global libmagic symbols, prefixing with library version number.
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
        'apprentice'        => 1,
        'apptype'           => 1,
        'ascmagic'          => 1,
        'cdf'               => 1,
        'cdf_time'          => 1,
        'compress'          => 1,
        'encoding'          => 1,
        'fsmagic'           => 1,
        'funcs'             => 1,
        'getline'           => 1,
        'is_tar'            => 1,
        'is_csv'            => 1,
        'is_json'           => 1,
        'magic'             => 1,
        'print'             => 1,
        'readcdf'           => 1,
        'readelf'           => 1,
        'softmagic'         => 1,
        'fmtcheck'          => 1,
        'file'              => 1
        );

my $o_keep                  = 0;
my $o_version               = '541';
my $o_src                   = "./file-5.41/src";
my $o_original              = 0;

sub ProcessDir($);
sub ProcessFile($$;$);

exit &main();

sub
main()
{
    my $o_clean = 0;
    my $o_help  = 0;

    my $ret
        = GetOptions(
                'keep'      => \$o_keep,
                'version'   => \$o_version,
                'original'  => \$o_original,
                'src'       => \$o_src,
                'help'      => \$o_help
                );

    Usage() if (!$ret || $o_help);
    Usage("unexpected arguments $ARGV[1] ...") if (scalar @ARGV);
    ProcessDir($o_src);
    return 0;
}


sub
Usage                   # (message)
{
    print "\nmakelib @_\n\n" if (@_);
    print <<EOU;

Usage: perl magic_rename.pl [options]

Options:
    --help                  Help.
    --src <path>            libmagic source directory.
    --version=<version>     Magic lib version prefix (eg. 541).
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

                print "file: ${name}.${ext}\n";

                if (('c' eq $ext || 'h' eq $ext) && exists $x_files{${name}}) {
                    ProcessFile($dir, $file);

                } elsif (('magic.h' eq $name) && ('in' eq $ext)) {
                    ProcessFile($dir, $file, "magic.h");
                }
            }
        }
    }
}


sub
ProcessFile($$;$)       # (dir, file, outfile)
{
    my ($dir, $file, $outfile) = @_;
    $outfile = $file if (!$outfile);

    my $filenameorg = "${dir}/original/${file}";
    my $filename = "${dir}/${file}";
    my $outfilename = "${dir}/${outfile}";
    my $text = '';

    if (!$o_original || !open(FILE, "<${filenameorg}")) {
        open(FILE, "<${filename}") or
            die "cannot open <${filename}> : $!\n";
        print "importing: ${filename}\n";
    } else {
        print "importing: ${filenameorg}\n";
    }
    while (<FILE>) {
        $text .= $_;
    }
    close FILE;

    $text =~ s/file_fmtcheck/softmagic${o_version}_fmtcheck/g;
    $text =~ s/fmtcheck/file${o_version}_fmtcheck/g;

    $text =~ s/ file_/ file${o_version}_/g;
    $text =~ s/ cdf_/ file${o_version}_cdf_/g;

    if ($o_version eq '511') { #5.29 via define's
        $text =~ s/sread/file${o_version}_sread/g;
        $text =~ s/getdelim/file${o_version}_getdelim/g;
        $text =~ s/getline/file${o_version}_getline/g;
        $text =~ s/strlcpy/file${o_version}_strlcpy/g;
        $text =~ s/strlcat/file{o_version}_strlcat/g;
    }

    $text =~ s/file${o_version}_opts\.h/file_opts.h/g;
    $text =~ s/file{o_version}_file{o_version}/file{o_version}/g;

    $text =~ s/X\.YY/${o_version}/
        if ($outfile eq 'magic.h');

    rename($filename, $filenameorg)             # save original
        if (($filename eq $outfilename) && ! -f $filenameorg);

    print "exporting: ${outfilename}\n";

    open(FILE, ">${outfilename}") or
        die "cannot open <${outfilename}> : $!\n";
    print FILE $text;
    close FILE;
}

#end














