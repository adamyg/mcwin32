#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: config_windows.pl,v 1.1 2025/02/13 17:56:42 cvsuser Exp $
# Configure front-end for native windows targets.
#

use strict;
use warnings 'all';

use Cwd;
use File::Which qw(which where);

sub
ProgramFiles
{
	my $path = $ENV{ProgramFiles};
	$path =~ s/\\/\//g
		if ($path);
	return $path if ($path);
	return "C:/Program Files";
}

##  resolve binutils

my $PROGRAMFILES = ProgramFiles();
my $olocalutils = 0;
my $omingw = undef;

foreach (@ARGV) {
	if (/^--cfg-localutils$/) {
		print "config_windows: utilising local utils\n";
		$olocalutils = 1;

	} elsif (/^--cfg-msys=(.*)/) {
		$omingw = "$1/usr/bin";
		die "config_windows: --cfg-msys=$1, invalid directory.\n"
			if (! -d $omingw);
		print "config_windows: using mingw=${omingw}\n";
	}
}


sub
Resolve 		# (default, apps ...)
{
	my $default = shift;
	return $default
		if ($olocalutils);

	if (! defined $omingw) {
		my $gcc = which("gcc");
		if ($gcc) {
			$gcc =~ s/\.exe$//i;
			$gcc =~ s/\\/\//g;

			$omingw = "";

			if ($gcc =~ /^(.*)\/mingw64\/bin\/gcc$/i) {
				$omingw = "$1/usr/bin"
					if (-d "$1/usr/bin");

			} elsif ($gcc =~ /^(.*)\/mingw64\/bin\/gcc$/i) {
				$omingw = "$1/usr/bin"
					if (-d "$1/usr/bin");
			}

			print "config_windows: detected mingw=${omingw}\n"
				if ($omingw);
		}
	}

	foreach my $app (@_) {

		my $mingw_resolved = undef;
		$mingw_resolved = "${omingw}/${app}"
			if ($omingw && -f "${omingw}/${app}.exe");

		# PATH
		my $resolved = which $app;
		if ($resolved) {
			$resolved =~ s/\.exe$//i;
			$resolved =~ s/\\/\//g;

			if ($mingw_resolved && ($resolved ne $mingw_resolved)) {
				print "config_windows: <${resolved}> and <${mingw_resolved}> found; using Mingw64 instance.\n";
				$resolved = $mingw_resolved;
			}
			return $resolved;

		# Mingw64 package
		} elsif ($mingw_resolved) {
			return $mingw_resolved;

		# chocolatey import
		} elsif (-d "C:/ProgramData/chocolatey/bin") {
			$resolved = "C:/ProgramData/chocolatey/bin/${app}";
			return $resolved
				if (-f "${resolved}.exe");
		}
	}
	return $default;
}

sub
ResolveCoreUtils	# ()
{
	my @paths = (
		"",                                 # PATH
		"c:/msys64/usr",                    # MSYS installation
		"${PROGRAMFILES}/Git/usr",          # Git for Windows
		"c:/GnuWin32",                      # https://sourceforge.net/projects/getgnuwin32/files (legacy)
		"C:/Program Files (x86)/GnuWin32",  # choco install gnuwin32-coreutils.install (legacy)
		);
	my @cmds = ("mkdir", "rmdir", "cp", "mv", "rm", "egrep", "gzip", "tar", "unzip", "zip");

	foreach my $path (@paths) {
		my $success = 1;
		if (! $path) {
			foreach my $app (@cmds) {
				if (! which($app) ) {
					$success = 0;
					goto LAST;
				}
			}
			if ($success) {
				print "config_windows: CoreUtils=PATH\n";
				return "";
			}
		} else {
			my $bin = "${path}/bin";
			if (-d $bin) {
				foreach my $app (@cmds) {
					if (! -f "${bin}/${app}") {
						$success = 0;
						goto LAST;
					}
				}
			}
			if ($success) {
				print "config_windows: CoreUtils=${path}\n";
				return $path;
			}
		}
	}
	die "config_windows: unable to determine coreutils\n";
}

my $busybox	= Resolve('./support/busybox', 'busybox');
my $wget	= Resolve('./support/wget', 'wget');
my $bison	= Resolve('$(D_BIN)/byacc', 'bison', 'yacc');
my $flex	= Resolve('$(D_BIN)/flex', 'flex');
my $coreutils	= undef;

##  build command line

my $cwd = getcwd;
die "config_windows: spaces within work directory <$cwd>; rename before proceeding.\n"
	if ($cwd =~ / /);

my @options;
my $core_symlink = 0;
my $otarget = undef;
my $ohelp = 0;

my $script  = shift @ARGV;
foreach (@ARGV) {
	if (/^--busybox=(.*)$/) {
		$busybox = $1;
	} elsif (/^--binpath=(.*)$/) {
		$coreutils = $1;
	} elsif (/^--wget=(.*)$/) {
		$wget = $1;
	} elsif (/^--bison=(.*)$/) {
		$bison = $1;
	} elsif (/^--flex=(.*)$/) {
		$flex = $1;
	} elsif (/^--cfg-symlink$/) { # undocumented
		# --cfg-symlink, Symlink detected coreutils to ./CoreUtils.
		$core_symlink = 1;
	} elsif (/^--cfg-localutils$/) {
		# consume
	} elsif (/^--cfg-msys=/) {
		# consume
	} elsif (/^--help$/) {
		$ohelp = 1;
	} else {
		if (/^--/) {
			if (/^--(.*)=(.*)$/) {
				push @options, "--$1=\"$2\"";
			} else {
				push @options, $_;
			}
		} else {
			die "config_windows: multiple targets, <$_> unexpected\n"
				if ($otarget);
			$otarget = $_;
		}
	}
}

if ($ohelp) {
    print <<EOU;

Usage: perl config_windows [options] <command>

Options:

    --cfg-localutils        Override selection of bison, flex, wget and busybox to bundled versions.

    --cfg-msys=<path>       Path to msys64 installation.
EOU
	system "$^X ${script} --help-options";
	exit 3;
}

$coreutils = ResolveCoreUtils()
	if (! $coreutils);
if ($coreutils) {
	if ($core_symlink) {
		if ($coreutils =~ / /) { # spaces, symlink
			print "config_windows: CoreUtils: ./CoreUtils => ${coreutils} (symlink)\n";
			system "mklink /J CoreUtils \"${coreutils}\""
				if (! -d "CoreUtils/bin");
			push @options, "--binpath=./CoreUtils/bin";

		} else {
			push @options, "--binpath=${coreutils}/bin";
		}
	} else {
		die "config_windows: coreutils detected yet not in PATH, add <$coreutils/bin> before proceeding.\n";
		return;
	}
}

die "config_windows: target missing\n"
	if (! $otarget);

print "\n$^X ${script}\n => --busybox=\"${busybox}\" --wget=\"${wget}\" --flex=\"${flex}\" --bison=\"${bison}\" @options ${otarget}\n\n";

system "$^X ${script} --busybox=\"${busybox}\" --wget=\"${wget}\" --flex=\"${flex}\" --bison=\"${bison}\" @options ${otarget}";

#end
