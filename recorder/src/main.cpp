#include "Recorder.hpp"
#include "SourceHardware.hpp"

int main(int argc, char **argv) {
    if(argc != 3) {
        std::cerr << argv[0] << " DURATION OUTPUT" << std::endl;
        return -1;
    }

    Recorder recorder(argv[2]);

    SourceHardware source(&recorder);

    std::this_thread::sleep_for(std::chrono::seconds(std::stoul(argv[1])));
}
