#!/usr/bin/perl
package make;
use strict;
use warnings;
use feature qw(switch say);
use Exporter qw(import);
use Carp qw(croak);
our @EXPORT_OK = qw(rule phony make make_argv stale modtime default);

my %modtime;
my %phony;
my @rules;
my %rules_by_to;
my $picked_defaults;
my @defaults;
my @plan;
my @plan_stack;

sub reset_plan () {
    undef %modtime;
    undef @plan;
    undef @plan_stack;
}

 # Declaring rules
sub rule_with_caller ($$$$$$) {
    my ($package, $file, $line, $to, $from, $recipe) = @_;
    ref $recipe eq 'CODE' or croak "Non-code recipe given to rule";
    my $rule = {
        to => [ref $to eq 'ARRAY' ? @$to : $to],
        from => [ref $from eq 'ARRAY' ? @$from : $from],
        recipe => $recipe,
        package => $package,
        file => $file,
        line => $line,
        planned => 0,
    };
    push @rules, $rule;
    for (@{$rule->{to}}) {
        push @{$rules_by_to{$_}}, $rule;
    }
}
sub show_rule ($) {
    return "@{$_[0]{to}} <- @{$_[0]{from}}";
}
sub debug_rule ($) {
    return "$_[0]{file}:$_[0]{line}: " . show_rule($_[0]);
}
sub rule ($$$) {
    my ($package, $file, $line) = caller;
    my ($to, $from, $recipe) = @_;
    rule_with_caller($package, $file, $line, $to, $from, $recipe);
}
sub phony ($;$$) {
    my ($package, $file, $line) = caller;
    my ($phony, $from, $recipe) = @_;
    my @phony = ref $phony eq 'ARRAY' ? @$phony : $phony;
    for (@phony) {
        $phony{$_} = 1;
    }
    if (defined $from) {
        rule_with_caller($package, $file, $line, $phony, $from, $recipe);
    }
}
 # Dealing with defaults
sub default {
    $picked_defaults = 1;
    push @defaults, @_;
}
 # Testing staleness
sub fexists {
    return 0 if $phony{$_[0]};
    return -e $_[0];
}
sub modtime {
    return $modtime{$_[0]} //= (fexists($_[0]) ? (stat $_[0])[9] : 0);
}

sub stale {
    my $target = $_[0];
    return 1 if (!fexists($_[0]));
    for (@{$rules_by_to{$target}}) {
        for (@{$_->{from}}) {
            return 1 if stale($_) or modtime($target) < modtime($_);
        }
    }
}

 # Making
sub plan_target {
    my ($target) = @_;
     # Make sure the file exists or there's a rule for it
    unless ($rules_by_to{$target} or fexists($target)) {
        my $mess = "$0: Cannot find or make $target" . (@plan_stack ? ", required by\n" : "\n");
        for (@plan_stack) {
            $mess .= "\t" . debug_rule($_) . "\n";
        }
        die $mess;
    }
     # In general, there should be only rule per target, but there can be more.
    return grep plan_rule($_), @{$rules_by_to{$target}};
}
sub plan_rule {
    my ($rule) = @_;
     # detect loops
    if (not defined $rule->{planned}) {
        my $mess = "$0: Dependency loop\n";
        for my $old (reverse @plan_stack) {
            $mess .= "\t" . debug_rule($old) . "\n";
            die $mess if $rule eq $old;  # reference compare
        }
        die $mess . "\t...oh wait, false alarm.  Which means there's a bug in maker.";
    }
    elsif ($rule->{planned}) {
        return 1;  # Already planned, but we'll still cause updates
    }
    push @plan_stack, $rule;
    $rule->{planned} = undef;  # Mark that we're currently planning this
     # always recurse
    my $stale = grep plan_target($_), @{$rule->{from}};
    $stale ||= grep {
        my $to = $_;
        !fexists($to) or grep modtime($to) < modtime($_), @{$rule->{from}}
    } @{$rule->{to}};
    push @plan, $rule if $stale;
     # Done planning this rule
    $rule->{planned} = 1;
    pop @plan_stack;
    return $stale;
}

sub plan_make (@) {
    reset_plan();
    if (@_) {
        grep plan_target($_), @_;
    }
    elsif ($picked_defaults) {
        grep plan_target($_), @defaults;
    }
    else {
        plan_rule($rules[0]);
    }
    return @plan;
}

sub make {
    if (not @rules) {
        say "$0: No rules have been specified yet.";
        return 0;
    }
    my @plan = eval { plan_make(@_) };
    if ($@) {
        warn $@;
        say "$0: Nothing was done due to error.";
        return 0;
    }
    if (not @plan) {
        say "$0: All up to date";
        return 1;
    }
    for (@plan) {
        say "$0: " . show_rule($_);
        eval { $_->{recipe}->($_->{to}, $_->{from}) };
        if ($@) {
            warn $@;
            say "$0: Did not finish due to error.";
            return 0;
        }
    }
    say "$0: Done.";
    return 1;
}

sub make_argv () {
    exit(!make(@ARGV));
}


1;
