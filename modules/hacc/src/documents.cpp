#include "../inc/documents.h"

namespace hacc {

    std::unordered_map<std::string, Document*> documents_by_name;

    Document* new_document (std::string filename) {
        return new Document {
            filename,
            nullptr,
            nullptr,
            nullptr,
            true
        };
    }

    Document* doc (std::string filename) {
        auto d = documents_by_name[filename];
        if (!d) {
            Hacc* dat = hacc_from_file(filename);
            
        }
    }

}
