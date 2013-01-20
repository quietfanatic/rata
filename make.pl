#!/usr/bin/perl

use strict;
use warnings;
BEGIN {
    unless (defined $FindBin::Bin) {
        require FindBin;
        require "$FindBin::Bin/tool/make.pm";
    }
    make->import(':all');
}
use autodie qw(:all);
use File::Path qw<remove_tree>;

my $here;

workflow {
    $here = Cwd::cwd;
    
     # Arch specific stuff.  Currently only g++-4.7 is nominally supported, because
     # we require advanced C++11 features.  All modules' make.pls should include
     # this file.
    sub cppc {
        run "$here/tool/filter_errors.pl", qw<g++-4.7 -std=c++11 -fmax-errors=10 -c -Wall -Wno-format-security -ggdb>, @_;
    }
    sub ld {
        run qw<g++-4.7>, @_;
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

    include glob 'modules/*';

    phony 'test', sub { targetmatch qr/^modules\/.*\/test/ }, sub { };

    phony 'clean', sub { targetmatch qr/^modules\/.*\/clean/ }, sub { };

    defaults 'test';

}


