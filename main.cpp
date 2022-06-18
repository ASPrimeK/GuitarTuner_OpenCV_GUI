
#include "main.hpp"

int main(int argc, char **argv) {
    initProcess(60, 400);
    initAudio();
    initGUI();
    initGuitarInterval();

    while (isExit(1)) {
        updateTarget();
        updateGUI();
    }
}