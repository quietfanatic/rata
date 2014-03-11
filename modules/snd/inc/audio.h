#ifndef HAVE_SND_AUDIO_H
#define HAVE_SND_AUDIO_H

#include <string>
#include "util/inc/organization.h"
namespace hacc { struct Document; }

namespace snd {

    void init ();

    struct Sample {
        int16 l;
        int16 r;
    };

     // Currently, only raw 16-bit 44.1khz interleaved-stereo PCM is supported
    struct Audio {
        std::string filename;
        Sample* samples = NULL;
        uint32 length = 0;

        Audio () { }
        void finish ();
        ~Audio () { if (samples) delete[] samples; }
    };

     // All changes to this should be reflected live.
    struct Voice : util::Link<Voice> {
        Audio* audio = NULL;
        uint32 pos = 0;
        bool paused = false;
        hacc::Document* auto_delete_from = NULL;
        Voice () { }
        void finish ();
        Voice (Audio* audio);  // Calls finish
        ~Voice ();
    };

}

#endif
