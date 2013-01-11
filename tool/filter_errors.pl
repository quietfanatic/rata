#!/usr/bin/perl

use strict;
use warnings;
use IPC::Open3;
use IO::Select;
use v5.14;

my $pid = open3('<&', my $out, my $err, @ARGV);

my $selector = IO::Select->new($out, $err);
my $done = 0;
while ($selector->count) {
    for ($selector->can_read()) {
        when ($out) {
            my $_ = readline($out);
            if (defined $_) {
                s/error:/\e[1m\e[31merror\e[0m:/g;
                s/warning:/\e[1m\e[33mwarning\e[0m:/g;
                s/note:/\e[1m\e[33mwarning\e[0m:/g;
                s/required from here/required from \e[1m\e[31mhere\e[0m/g;
                print;
            }
            else {
                $selector->remove($out);
            }
        }
        when ($err) {
            my $_ = readline($out);
            if (defined $_) {
                s/error:/\e[1m\e[31merror\e[0m:/g;
                s/warning:/\e[1m\e[33mwarning\e[0m:/g;
                s/note:/\e[1m\e[33mwarning\e[0m:/g;
                s/required from here/\e[1m\e[33mrequired from here\e[0m/g;
                warn $_;
            }
            else {
                $selector->remove($err);
            }
        }
    }
}
waitpid($pid, 0);
exit($? >> 8);



