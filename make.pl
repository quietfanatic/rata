#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/tool'};
use MakePl;
use Cwd 'realpath';
use File::Path qw<remove_tree>;

my $here = cwd;
mkdir 'save';

my @includes = ("$here/modules", "$here/lib/Box2D");

##### COMMAND LINE FLAGS
my %flags = (
    clang => {
        compile => [qw(-std=c++11 -c), map "-I$_", @includes],
        link => [qw(-std=c++11 -lstdc++ -lm)],
        release => [qw(-O3 -Wno-null-conversion -Wno-format-security -fcolor-diagnostics)],
        debug => [qw(-ggdb -Wall -Wno-null-conversion -Wno-unused-variable -Wno-unused-function -Wno-format-security -fcolor-diagnostics)],
        profile => ['-pg'],
    },
    'g++' => {
        compile => [qw(-std=c++11 -c), map "-I$_", @includes],
        link => [qw(-std=c++11)],
        release => [qw(-O3 -Wno-null-conversion -Wno-format-security)],
        debug => [qw(-ggdb -Wall -Wno-null-conversion -Wno-unused-variable -Wno-unused-function -Wno-format-security)],
        profile => ['-pg'],
    },
);

##### DEFAULT CONFIG
my %config = (
    compiler => undef,
    'with-clang' => 'clang',
    'with-g++' => 'g++',
    build => undef,
    profile => 0,
    'compiler-opts' => [],
    'linker-opts' => []
);
##### AUTO CONFIG
config('build-config', \%config, sub {
    my $have_clang = which 'clang';
    my $have_gpp = which 'g++';
    if (!$have_clang and !$have_gpp) {
        die "Neither clang nor g++ were detected.  Please install one or, if one is already installed, point to its location with --with-clang=<command> or --with-g++=<command>\n";
    }
    my @auto_opts;
    unless (defined $config{compiler}) {
        if ($have_clang) {
            push @auto_opts, [qw(compiler clang)];
        }
        elsif ($have_gpp) {
            push @auto_opts, [qw(compiler g++)];
        }
    }
    defined $config{build} or push @auto_opts, [qw(build release)];
    for (@auto_opts) {
        $config{$_->[0]} = $_->[1];
    }
    print 'build-config: setting', map(" --$_->[0]=$_->[1]", @auto_opts), "\n"
        if @auto_opts;
});
$config{profile} = 0;  # Do not preserve this between invokations.
##### MANUAL CONFIG
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
}, "--build=[release|debug] - Select build type (current: " . ($config{build} // 'release') . ')';
option 'profile', \$config{profile},
    "--profile - Add profiling code to the binary (once only)";
option 'compiler-opts', sub {
    $config{'compiler-opts'} = [split /\s+/, $_[0]];
}, "--compiler-opts=<options> - Give extra options to the compiler (once only)";
option 'linker-opts', sub {
    $config{'linker-opts'} = [split /\s+/, $_[0]];
}, "--linker-opts=<options> - Give extra options to the linker (once only)";

##### STANDARD COMMANDS
sub cppc {
    my $fs = $flags{$config{compiler}};
    run $config{'with-' . $config{compiler}}, @{$fs->{compile}}, @{$fs->{$config{build}}},
        $config{profile} ? @{$fs->{profile}} : (), @{$config{'compiler-opts'}}, @_;
}
sub ld {
    my $fs = $flags{$config{compiler}};
    run $config{'with-' . $config{compiler}}, @{$fs->{link}}, @{$fs->{$config{build}}},
        $config{profile} ? @{$fs->{profile}} : (), @{$config{'compiler-opts'}}, @_;
}
sub output { '-o', $_[0]; }

##### STANDARD RULES
my $conf = realpath 'build-config';
sub cppc_rule {
    my ($to, $from) = @_;
    my @from = ref $from eq 'ARRAY' ? @$from : $from;
    rule $to, [@from, $conf], sub {
        cppc((grep /.cpp$/, @from), output($_[0][0]));
    }, {fork => 1}
}
sub ld_rule {
    my ($to, $from, $libs) = @_;
    my @from = ref $from eq 'ARRAY' ? @$from : $from;
    my @libs = defined $libs ? (ref $libs eq 'ARRAY' ? @$libs : $libs) : ();
    rule $to, [@from, $conf], sub {
        ld @from, @libs, output($_[0][0]);
    }, {fork => 1}
}
sub test_rule {
    phony 'test', $_[0], sub { system "./$_[1][0] --test | prove -e '' -"; };
}
sub clean_rule {
    my @tmps = @_;
    phony 'clean', [], sub { remove_tree @tmps; }
}
sub objects {
    for (@_) {
        cppc_rule("tmp/$_.o", "src/$_.cpp");
    }
}
sub tests {
    for (@_) {
        cppc_rule("tmp/$_.t.o", "test/$_.t.cpp");
    }
}

 # Automatically glean subdeps from #includes
subdep sub {
    my ($file) = @_;
     # Select only C++ files
    $file =~ /\.(?:c(?:pp)?|h)$/ or return ();

    my $base = ($file =~ /(.*?)[^\\\/]*$/ and $1);
    my @incs = (slurp $file, 2048) =~ /^\s*#include\s*"([^"]*)"/gmi;
    my @r;
    for (@incs) {
        for my $I (@includes, $base) {
            push @r, realpath("$I/$_") if -e("$I/$_");
        }
    }
    return @r;
};

 # Let modules do their own declarations
include glob 'modules/*';

 # Miscellaneous stuff
phony 'test', sub { grep /modules\/.*\/test$/, targets }, sub { };

phony 'clean', sub { grep /modules\/.*\/clean$/, targets }, sub {
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
