#ifndef HAVE_SHELL_MAIN_H
#define HAVE_SHELL_MAIN_H

#include "hacc/inc/files.h"
#include "hacc/inc/documents.h"

extern hacc::File main_file;
extern hacc::File current_state;

static inline hacc::Document* state_document () {
    return current_state.data();
}

#endif
