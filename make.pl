#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use if !$^S, lib => "$FindBin::Bin/tool";
use Make_pl;
use autodie;
use File::Path qw<remove_tree>;
use File::Spec::Functions qw(:ALL);

my $here;
my @compilers = qw(clang g++);
my $compiler = 'clang';

workflow {
    $here = Cwd::cwd;

    option 'compiler', sub {
        grep $_ eq $_[0], @compilers
            or die "Unrecognized compiler: $_[0]; recognized are: @compilers\n";
        $compiler = $_[0];
    }, "--compiler=[clang|g++] - Use this to compile and link.  Default: clang";
    
     # Arch specific stuff.  Currently only g++-4.7 is nominally supported, because
     # we require advanced C++11 features.  All modules' make.pls should include
     # this file.
    sub cppc {
        if ($compiler eq 'clang') {
            run qw<clang -std=c++11 -c -Wall -Wno-null-conversion -Wno-unused-function -Wno-format-security -ggdb>, "-I$here/lib/Box2D", @_;
        }
        elsif ($compiler eq 'g++') {
            run qw<g++ -std=c++11 -c -Wall -Wno-unused-function -Wno-format-security -ggdb>, "-I$here/lib/Box2D", @_;
        }
    }
    sub ld {
        if ($compiler eq 'clang') {
            run qw<clang>, @_;
        }
        elsif ($compiler eq 'g++') {
            run qw<g++>, @_;
        }
        else {
            die "Unrecognized compiler name: $compiler; recognized are: clang g++\n";
        }
    }
    sub output { '-o', $_[0]; }

     # Convenience rule making functions for modules that follow a convention.
    sub cppc_rule {
        rule $_[0], $_[1], sub { cppc((grep /.cpp$/, @{$_[1]}), output($_[0][0])); }
    }
    sub ld_rule {
        my @libs = defined $_[2] ? ref($_[2]) eq 'ARRAY' ? @{$_[2]} : $_[2] : ();
        rule $_[0], $_[1], sub { ld @{$_[1]}, @libs, output($_[0][0]); }
    }
    sub test_rule {
        phony 'test', $_[0], sub { run "./$_[1][0] --test | prove -e '' -"; };
    }
    sub clean_rule {
        my @tmps = @_;
        phony 'clean', [], sub { no autodie; remove_tree @tmps; }
    }
    sub objects {
        my @objs = @_;
        for (@objs) {
            cppc_rule("tmp/$_.o", "src/$_.cpp");
        }
    }

     # Automatically gleam subdeps from #includes
    subdep sub {
        my ($file) = @_;
        $file =~ /\.(?:cpp|h)$/ or return ();
        my @vdf = splitpath($file);
        my $base = catpath($vdf[0], $vdf[1], '');
        open my $F, '<', $file or (warn "Could not open $file: $!\n" and return);
        read $F, my $head, 1000;
        close $F;
        my @r;
        for ($head =~ /^\s*#include\s*"([^"]*)"/gmi) {
            push @r, rel2abs($_, $base);
        }
        return @r;
    };

    include glob 'modules/*';

    phony 'test', sub { targetmatch qr/^modules\/.*\/test/ }, sub { };

    phony 'clean', sub { targetmatch qr/^modules\/.*\/clean/ }, sub { };

    rule 'lib/libBox2D.a', [], sub {
        die <<END
This program requires the physics engine Box2D.  Please get libBox2D via:
    cd lib
    svn checkout http://box2d.googlecode.com/svn/trunk/ box2d-read-only -r 252
and compile it somehow.  Then copy libBox2D.a into lib/ and make sure the Box2D
headers are in lib/ (such that the file lib/Box2D/Box2D/Box2D.h exists).
Sorry for the inconvenience.
END
    };

    defaults 'test';

}


