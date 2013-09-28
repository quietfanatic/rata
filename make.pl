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
        release => [qw(-O3 -Wno-null-conversion -Wno-format-security)],
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
my %config = (
    compiler => undef,
    'with-clang' => 'clang',
    'with-g++' => 'g++',
    build => 'release',
    profile => 0,
    'compiler-opts' => [],
    'linker-opts' => []
);
config('build-config', \%config, sub {
    my $have_clang = (`$config{'with-clang'} -v 2>&1` // '' =~ /clang version (\d+)\.(\d+)/);
    my $have_gpp = (`$config{'with-g++'} -v 2>&1` // '' =~ /gcc version (\d+)\.(\d+)/);
    if (!$have_clang and !$have_gpp) {
        die "Neither clang nor g++ were detected.  Please install one or, if one is already installed, point to its location with --with-clang=<command> or --with-g++=<command>\n";
    }
    unless (defined $config{compiler}) {
        if ($have_clang) {
            $config{compiler} = 'clang';
        }
        elsif ($have_gpp) {
            $config{compiler} = 'g++';
        }
    }
});

option 'compiler', sub {
    grep $_ eq $_[0], keys %flags
        or die "Unrecognized compiler: $_[0]; recognized are: " . keys(%flags) . "\n";
    $config{compiler} = $_[0];
}, "--compiler=[clang|g++] - Use this to compile and link (current: " . ($config{compiler} // 'clang') . ')';
option 'with-clang', \$config{'with-clang'},
    "--with-clang=<command> - Use this command to invoke clang (current: " . $config{'with-clang'} . ')';
option 'with-g++', \$config{'with-g++'},
    "--with-g++=<command> - Use this command to invoke g++ (current: " . $config{'with-g++'} . ')';
option 'build', sub {
    $_[0] eq 'release' or $_[0] eq 'debug'
        or die "Invalid build type: $_[0]; recognized are: release debug\n";
    $config{build} = $_[0];
}, "--build=[release|debug] - Select build type (current: " . $config{build} . ')';
option 'profile', \$config{profile},
    "--profile - Add profiling code to the binary (once only)";
option 'compiler-opts', sub {
    $config{'compiler-opts'} = split /\s+/, $_[0];
}, "--compiler-opts=<options> - Give extra options to the compiler (once only)";
option 'linker-opts', sub {
    $config{'linker-opts'} = split /\s+/, $_[0];
}, "--linker-opts=<options> - Give extra options to the linker (once only)";

##### STANDARD COMMANDS
sub cppc {
    my $fs = $flags{$config{compiler}};
    run $config{'with-' . $config{compiler}}, @{$fs->{compile}}, @{$fs->{$config{build}}},
        $config{profile} ? @{$fs->{profile}} : (), @_;
}
sub ld {
    my $fs = $flags{$config{compiler}};
    run $config{'with-' . $config{compiler}}, @{$fs->{link}}, @{$fs->{$config{build}}},
        $config{profile} ? @{$fs->{profile}} : (), @_;
}
sub output { '-o', $_[0]; }

###### STANDARD RULES
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

 # All compilation and linking steps rely implicitly on build-config
subdep sub {
    $_[0] =~ /\.o$|^rata$/ or return ();
    return 'build-config';
};

 # Automatically glean subdeps from #includes
subdep sub {
    my ($file) = @_;
     # Select only C++ files
    $file =~ /\.(?:c(?:pp)?|h)$/ or return ();

    my $base = catpath((splitpath $file)[0, 1], '');
    my @includes = (slurp $file, 2048) =~ /^\s*#include\s*"([^"]*)"/gmi;
    return map rel2abs($_, $base), @includes;
};

include glob 'modules/*';

phony 'test', sub { targetmatch qr/^modules\/.*\/test/ }, sub { };

phony 'clean', sub { targetmatch qr/^modules\/.*\/clean/ }, sub {
    no autodie;
    unlink 'build-config';
};

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
