#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/tool'};
use MakePl;

include '../..';

mkdir 'tmp';

objects(qw(common tree strings types dynamicism haccable files haccable_standard documents));
tests(qw(tree strings haccable files documents));

ld_rule('tmp/t', [qw[
    tmp/common.o
    tmp/tree.o tmp/tree.t.o
    tmp/strings.o tmp/strings.t.o
    tmp/types.o
    tmp/dynamicism.o
    tmp/haccable.o tmp/haccable.t.o
    tmp/haccable_standard.o
    tmp/files.o tmp/files.t.o
    tmp/documents.o tmp/documents.t.o
    ../tap/tmp/tap.o ../tap/tmp/main.o
    ../util/tmp/integration.o
]]);

test_rule('tmp/t');

clean_rule(glob 'tmp/*');

defaults 'test';

make;
