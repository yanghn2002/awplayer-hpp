#include <iostream>
#include <signal.h>
#include "awplayer.hpp"


using namespace AWPlayer;
using namespace std;


TPlayer tplayer;

void playUrl(const char* url) {
    cout << "Playing: " << url << endl;
    tplayer.reset();
    tplayer.play(url);
}
void waitPlaying(void) { while(tplayer.isWorking()); }

int main(int argc, char* argv[]) {

    signal(SIGINT, [](int _){ (void)_; exit(0); });

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <video-source-1> ..." << '\n';
        return 1;
    }

    playUrl(argv[1]);
    for(int i = 2; i < argc; i++) {
        waitPlaying();
        playUrl(argv[i]);
    }
    waitPlaying();

    return 0;

}