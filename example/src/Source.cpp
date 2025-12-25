#include "Source.hpp"

Source::Source(Listener &listener) : listener{listener} {
}

bool Source::available() const {
    return true;
}
