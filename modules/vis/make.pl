#!/usr/bin/perl

use strict;
use warnings;
BEGIN {
    unless (defined $FindBin::Bin) {
        require FindBin;
        require "$FindBin::Bin/../../tool/make.pm";
    }
    make->import(':all');
}
use autodie qw<:all>;
use File::Path qw<remove_tree>;


workflow {

    include '../..';

    cppc_rule('tmp/images.o', 'src/images.cpp');
    cppc_rule('tmp/sprites.o', 'src/sprites.cpp');
    cppc_rule('tmp/models.o', 'src/models.cpp');
    cppc_rule('tmp/shaders.o', 'src/shaders.cpp');
    cppc_rule('tmp/graffiti.o', 'src/graffiti.cpp');
    cppc_rule('tmp/text.o', 'src/text.cpp');
    cppc_rule('tmp/console.o', 'src/console.cpp');

    clean_rule(glob 'tmp/*');

};



