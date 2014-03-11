#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/../../tool'};
use MakePl;

include '../..';

mkdir 'tmp';

objects(qw(debug geometry integration));
tests(qw(integration));

ld_rule('tmp/t', [qw[
    tmp/integration.o
    tmp/integration.t.o
    ../hacc/tmp/common.o
    ../hacc/tmp/tree.o
    ../hacc/tmp/types.o
    ../hacc/tmp/strings.o
    ../hacc/tmp/files.o
    ../hacc/tmp/haccable.o
    ../hacc/tmp/dynamicism.o
    ../tap/tmp/tap.o
    ../tap/tmp/main.o
]]);

test_rule('tmp/t');

clean_rule(glob 'tmp/*');

defaults('test');

make;
