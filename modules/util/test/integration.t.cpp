#include "../inc/integration.h"
#include "../../tap/inc/tap.h"

using namespace util;
using namespace tap;

tap::Tester integration_tester ("util/integration", [](){
    plan(19);
    is(is_absolute("/ab/cd"), true, "is_absolute returns true");
    is(is_absolute("ab/cd"), false, "is_absolute return false");
    is_strcmp(basename("ab/cd").c_str(), "cd", "basename works in normal case");
    is_strcmp(basename("/ab/cd").c_str(), "cd", "basename works with absolute path");
    is_strcmp(basename("ab").c_str(), "ab", "basename works in local case");
    is_strcmp(basename("../ab/cd").c_str(), "cd", "basename works with ..");
    is_strcmp(dirname("ab/cd").c_str(), "ab", "dirname works in normal case");
    is_strcmp(dirname("/ab/cd").c_str(), "/ab", "dirname works with absolute path");
    is_strcmp(dirname("ab").c_str(), ".", "dirname works in local case");
    is_strcmp(dirname("../ab/cd").c_str(), "../ab", "dirname works with ..");
    is_strcmp(abs2rel("/ab/cd/ef/gh", "/ab/cd").c_str(), "ef/gh", "abs2rel does its job");
    is_strcmp(abs2rel("ab/cd/ef/gh", "ab/cd").c_str(), "ef/gh", "abs2rel doesn't actually require relative path");
    is_strcmp(abs2rel("/ab/cd/ef/gh", "/ab/cd/qw").c_str(), "../ef/gh", "abs2rel can generate ..");
    is_strcmp(rel2abs("ef/gh", "/ab/cd").c_str(), "/ab/cd/ef/gh", "rel2abs works");
    is_strcmp(canonicalize("/ab///cd/ef/").c_str(), "/ab/cd/ef", "canonicalize removes extra slashes");
    is_strcmp(canonicalize("/ab/././cd/.").c_str(), "/ab/cd", "canonicalize removes .s");
    is_strcmp(canonicalize(".").c_str(), ".", "canonicalize leaves a single . alone");
    is_strcmp(canonicalize("ab/cd/ef/../gh").c_str(), "ab/cd/gh", "canonicalize processes ..s");
    is_strcmp(canonicalize("../ab/cd").c_str(), "../ab/cd", "canonicalize leaves leading ..s alone");
});

