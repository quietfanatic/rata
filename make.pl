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


workflow {
    
     # Arch specific stuff.  Currently only g++-4.7 is nominally supported, because
     # we require advanced C++11 features.  All module's make.pls should include
     # this file.
    sub cppc { run qw<g++-4.7 -std=c++11 -fmax-errors=5 -c -Wall -Wno-format-security -ggdb>, @_; }
    sub ld { run qw<g++-4.7>, @_; }
    sub output { '-o', $_[0]; }

    include glob 'modules/*';

    our @libs = qw(-lGL -lglfw -lSOIL lib/libBox2D.a);

    phony 'test', sub { targetmatch qr/^modules\/.*\/test/ }, sub { };

    phony 'clean', sub { targetmatch qr/^modules\/.*\/clean/ }, sub { };

    defaults 'test';

}


