#!/usr/bin/perl

use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/tool";
use MakePl;
use autodie;
use File::Path qw<remove_tree>;
use File::Spec::Functions qw(:ALL);

my $here = cwd;

##### COMMAND LINES
my %flags = (
    clang => {
        compile => [qw(-std=c++11 -c), "-I$here/lib/Box2D"],
        link => [qw(-std=c++11 -lstdc++ -lm)],
        release => ['-O3 -Wno-null-conversion -Wno-format-security'],
        debug => [qw(-ggdb -Wall -Wno-null-conversion -Wno-unused-function -Wno-format-security)],
        profile => ['-pg'],
    },
    'g++' => {
        compile => [qw(-std=c++11 -c), "-I$here/lib/Box2D"],
        link => [qw(-std=c++11)],
        release => ['-O3'],
        debug => [qw(-ggdb -Wall -Wno-null-conversion -Wno-unused-function -Wno-format-security)],
        profile => ['-pg'],
    },
);

##### CONFIGURATION
my %options = (
    compiler => undef,
    'with-clang' => 'clang',
    'with-g++' => 'g++',
    build => 'release',
    profile => 0,
    'compiler-opts' => [],
    'linker-opts' => []
);
config('build-config', \%options, sub {
    my $have_clang = (`$options{'with-clang'} -v 2>&1` // '' =~ /clang version (\d+)\.(\d+)/);
    my $have_gpp = (`$options{'with-g++'} -v 2>&1` // '' =~ /gcc version (\d+)\.(\d+)/);
    if (!$have_clang and !$have_gpp) {
        die "Neither clang nor g++ were detected.  Please install one or, if one is already installed, point to its location with --with-clang=<command> or --with-g++=<command>\n";
    }
    unless (defined $options{compiler}) {
        if ($have_clang) {
            $options{compiler} = 'clang';
        }
        elsif ($have_gpp) {
            $options{compiler} = 'g++';
        }
    }
});

option 'compiler', sub {
    grep $_ eq $_[0], keys %flags
        or die "Unrecognized compiler: $_[0]; recognized are: " . keys(%flags) . "\n";
    $options{compiler} = $_[0];
}, "--compiler=[clang|g++] - Use this to compile and link (current: " . ($options{compiler} // 'clang') . ')';
option 'with-clang', \$options{'with-clang'},
    "--with-clang=<command> - Use this command to invoke clang (current: " . $options{'with-clang'} . ')';
option 'with-g++', \$options{'with-g++'},
    "--with-g++=<command> - Use this command to invoke g++ (current: " . $options{'with-g++'} . ')';
option 'build', sub {
    $_[0] eq 'release' or $_[0] eq 'debug'
        or die "Invalid build type: $_[0]; recognized are: release debug\n";
    $options{build} = $_[0];
}, "--build=[release|debug] - Select build type (current: " . $options{build} . ')';
option 'profile', \$options{profile},
    "--profile - Add profiling code to the binary (once only)";
option 'compiler-opts', sub {
    $options{'compiler-opts'} = split /\s+/, $_[0];
}, "--compiler-opts=<options> - Give extra options to the compiler (once only)";
option 'linker-opts', sub {
    $options{'linker-opts'} = split /\s+/, $_[0];
}, "--linker-opts=<options> - Give extra options to the linker (once only)";

 # Arch specific stuff.  Currently only g++-4.7 is nominally supported, because
 # we require advanced C++11 features.  All modules' make.pls should include
 # this file.
sub cppc {
    my $fs = $flags{$options{compiler}};
    run $options{'with-' . $options{compiler}}, @{$fs->{compile}}, @{$fs->{$options{build}}},
        $options{profile} ? @{$fs->{profile}} : (), @_;
}
sub ld {
    my $fs = $flags{$options{compiler}};
    run $options{'with-' . $options{compiler}}, @{$fs->{link}}, @{$fs->{$options{build}}},
        $options{profile} ? @{$fs->{profile}} : (), @_;
}
sub output { '-o', $_[0]; }

 # Convenient rule-making functions for modules that follow a convention.
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
    $file =~ /\.(?:c(?:pp)?|h)$/ or return ();
    my @vdf = splitpath($file);
    my $base = catpath($vdf[0], $vdf[1], '');
    open my $F, '<', $file or (warn "Could not open $file: $!\n" and return);
    read $F, my $head, 1000;
    close $F;
    my @r;
    for ($head =~ /^\s*#include\s*"([^"]*)"/gmi) {
        push @r, rel2abs($_, $base);
    }
    return (@r, 'build-config');  # And depend on the build configuration file
};

include glob 'modules/*';

phony 'test', sub { targetmatch qr/^modules\/.*\/test/ }, sub { };

phony 'clean', sub { targetmatch qr/^modules\/.*\/clean/ }, sub { };

unless (-e 'lib/libBox2D.a') {
    die <<END
This program requires the physics engine Box2D.  Please get libBox2D via:
    cd lib
    svn checkout http://box2d.googlecode.com/svn/trunk/ box2d-read-only -r 252
and compile it somehow.  Then copy libBox2D.a into lib/ and make sure the Box2D
headers are in lib/ (such that the file lib/Box2D/Box2D/Box2D.h exists).
Sorry for the inconvenience.
END
}

defaults 'rata';

make;
