#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: config_windows.pl,v 1.4 2025/04/08 18:31:31 cvsuser Exp $
# Configure front-end for native windows targets.
#

use strict;
use warnings 'all';

use Cwd 'realpath', 'getcwd';
use File::Which qw(which where);
use File::Basename;

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
my $trace = 0;

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
Trace
{
	print "config_windows: (V) " . sprintf(shift, @_) . "\n"
		if ($trace);
}

sub
Resolve			# (default, apps ...)
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
		"",					# PATH
		"c:/msys64/usr",			# MSYS installation(s)
		"d:/msys64/usr",
		"${PROGRAMFILES}/Git/usr",		# Git for Windows
		"c:/GnuWin32",				# https://sourceforge.net/projects/getgnuwin32/files (legacy)
		"c:/Program Files (x86)/GnuWin32",	# choco install gnuwin32-coreutils.install (legacy)
		);
	my @cmds = ("mkdir", "rmdir", "cp", "mv", "rm", "grep", "gzip", "tar", "unzip", "zip");

	foreach my $path (@paths) {
		if (! $path) {				# PATH
			my $success = 1;
			Trace("checking CoreUtils against <PATH>");
			foreach my $app (@cmds) {
				my $resolved = which($app);
				Trace("  $app=%s", $resolved ? lc $resolved : "(unresolved)");
				if (! $resolved) {
					$success = 0;
					last;
				}
				++$success;
			}
			if ($success) {
				print "config_windows: CoreUtils=PATH\n";
				return "";
			}

		} else {				# explicit; test possible solutions
			my $bin = "${path}/bin";
			my $success = (-d $bin);
			if ($success) {
				Trace("checking CoreUtils against <${bin}>");
				foreach my $app (@cmds) {
					my $resolved = (-f "${bin}/${app}" || "${bin}/${app}.exe");
					Trace("  $app=%s", $resolved ? "${bin}/${app}" : "(unresolved)");
					if (! $resolved) {
						$success = 0;
						last;
					}
					++$success;
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

my $perlpath	= undef;
my $busybox	= Resolve('./support/busybox', 'busybox');
my $wget	= Resolve('./support/wget', 'wget');
my $bison	= Resolve('$(D_BIN)/byacc', 'bison', 'yacc');
my $flex	= Resolve('$(D_BIN)/flex', 'flex');
my $coreutils	= undef;
my $inno	= undef;

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
	if (/^--busybox=(.*)$/) {			# busybox path, otherwise located.
		$busybox = $1;
	} elsif (/^--perlpath=(.*)$/) {			# Perl binary path, otherwise resolved.
		$perlpath = $1;
	} elsif (/^--binpath=(.*)$/) {			# Path to coreutils, otherwise these are assumed to be in the path.
		$coreutils = $1;
	} elsif (/^--wget=(.*)$/) {			# wget installation path.
		$wget = $1;
	} elsif (/^--bison=(.*)$/) {			# yacc/bison installation path.
		$bison = $1;
	} elsif (/^--flex=(.*)$/) {			# flex installation path.
		$flex = $1;
	} elsif (/^--inno=(.*)$/) {			# inno-setup installation path.
		$inno = $1;
	} elsif (/^--cfg-symlink$/) {			# undocumented
		# --cfg-symlink, Symlink detected coreutils to ./CoreUtils.
		$core_symlink = 1;
	} elsif (/^--cfg-localutils$/) {
		# consume
	} elsif (/^--cfg-msys=/) {
		# consume
	} elsif (/^--help$/) {
		$ohelp = 1;
	} elsif (/^--trace$/) {
		$trace = 1;
	} else {
		if (/^--/) {				# others, pass-thru
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

if (! defined $perlpath) {
	my $running = lc realpath($^X);
        my $resolved = dirname(${running});

	my $perl = which("perl");
	$perl = lc realpath($perl)
		if (defined $perl);

	if (! $perl || $perl eq 'perl' || $perl ne $running) {
							# non-found, generic or alternative
		print "config_windows: Perl=${resolved} (resolved, ${perl})\n";
		push @options, "--perlpath=\"${resolved}\"";
	} else {
		print "config_windows: Perl=PATH (${resolved})\n";
	}
} else {
	print "config_windows: Perl=${perlpath}\n";
	push @options, "--perlpath=\"${perlpath}\"";
}

$coreutils = ResolveCoreUtils()
	if (! $coreutils);
if ($coreutils) {
	if ($core_symlink) {
		if ($coreutils =~ / /) {		# spaces, symlink
			print "config_windows: coreutils: ./CoreUtils => ${coreutils} (symlink)\n";
			system "mklink /J CoreUtils \"${coreutils}\""
				if (! -d "CoreUtils/bin");
			push @options, "--binpath=\"./CoreUtils/bin\"";

		} else {
			push @options, "--binpath=\"${coreutils}/bin\"";
		}
	} else {
		die "config_windows: coreutils detected yet not in PATH, add <$coreutils/bin> before proceeding.\n";
		return;
	}
}

die "config_windows: target missing\n"
	if (! $otarget);

push @options, "--busybox=\"${busybox}\"";
push @options, "--wget=\"${wget}\"";
push @options, "--flex=\"${flex}\"";
push @options, "--bison=\"${bison}\"";
push @options, "--inno=\"${inno}\""
        if ($inno);

print "\n$^X ${script}\n => @options ${otarget}\n\n";
system "$^X ${script} @options ${otarget}";

#end
