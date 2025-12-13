#include <iostream>
#include <signal.h>
#include "awplayer.hpp"


using namespace AWPlayer;


int main(int argc, char* argv[]) {

    signal(SIGINT, [](int _){ (void)_; exit(0); });

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <video-source>" << '\n';
        return 1;
    }

    TPlayer tplayer;
    tplayer.play(argv[1]);
    while(true);

    return 0;

}