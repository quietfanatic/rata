#!/usr/bin/perl
package make;
use strict;
use warnings;
use feature qw(switch say);
use Exporter qw(import);
use Carp qw(croak);
our @EXPORT = qw(rule phony make make_argv);
our @EXPORT_OK = qw(visibly stale modtime default);

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
sub rule_with_caller ($$$@) {
    my ($package, $file, $line, $to, $from, @recipe) = @_;
    validate_recipe(@recipe);
    my $rule = {
        to => [ref $to eq 'ARRAY' ? @$to : $to],
        from => [ref $from eq 'ARRAY' ? @$from : $from],
        recipe => [@recipe],
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
sub rule ($$@) {
    my ($package, $file, $line) = caller;
    rule_with_caller($package, $file, $line, @_);
}
sub phony ($;$@) {
    my ($package, $file, $line) = caller;
    my ($phony, $from, @recipe) = @_;
    my @phony = ref $phony eq 'ARRAY' ? @$phony : $phony;
    for (@phony) {
        $phony{$_} = 1;
    }
    if (defined $from) {
        rule_with_caller($package, $file, $line, $phony, $from, @recipe);
    }
}
 # Dealing with defaults
sub default {
    $picked_defaults = 1;
    push @defaults, @_;
}
sub fabricate_target {
    $rules_by_to{_DEFAULT_BECAUSE_FIRST} = {
    }
}
 # Executing
sub getfun {
    no strict 'refs';
    given (ref $_[0]) {
        return $_[0] when 'CODE';
        return \&{"$_[0]"} when '';
        default { croak "Expected a CODE but got a $_"; }
    }
}

sub validate_recipe {
    @_ == 0 and croak "$0: No recipe given";
    my ($recipe, @args) = @_;
    defined $recipe or croak "$0: Recipe command was undefined";
    given (ref $recipe) {
        when ('') { }  # okay (string name of function)
        when ('CODE') { }  # okay
        when ('ARRAY') {
            
        }
        default { croak "$0: Recipe command was a $_ instead of a CODE, ARRAY, or scalar" }
    }
}

sub do_recipe {
    if (ref $_[0] eq 'ARRAY') {
        &do_recipe(@$_) for @_;
    }
    else {
        my ($code, @args) = @_;
        getfun($code)->(@args);
    }
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
sub plan_make (@) {
    my @targets = @_;
    if (!@targets) {
        if ($picked_defaults) {
            @targets = @defaults;
        }
        else {
            return plan_check_rule($rules[0]);
        }
    }
    my $stale = 0;
    for my $target (@targets) {
         # Make sure the file exists or there's a rule for it
        unless ($rules_by_to{$target}
             or $phony{$target}
             or -e $target) {
            my $mess = "$0: Cannot find or make $target" . (@plan_stack ? ", required by\n" : "\n");
            for (@plan_stack) {
                $mess .= "\t" . debug_rule($_) . "\n";
            }
            die $mess;
        }
         # In general, there should be only rule per target, but there can be more.
        for my $rule (@{$rules_by_to{$target}}) {
            plan_check_rule($rule) and $stale = 1;
        }
    }
    return $stale;
}
sub plan_check_rule {
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
    my $stale = plan_make(@{$rule->{from}});
    if (!$stale) {
        for my $to (@{$rule->{to}}) {
            for my $from (@{$rule->{from}}) {
                if (modtime($to) < modtime($from)) {
                    $stale = 1;
                }
            }
        }
    }
    push @plan, $rule if $stale;
     # Done planning this rule
    $rule->{planned} = 1;
    pop @plan_stack;
    return $stale;
}

sub make {
    reset_plan();
    if (not @rules) {
        say "$0: No rules have been specified yet.";
        return 0;
    }
    eval { plan_make(@_) };
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
        eval { do_recipe($_->{recipe}) };
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
    make(@ARGV);
}


1;
