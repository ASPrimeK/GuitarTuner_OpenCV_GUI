#include "record.hpp"

#include <string>
using namespace std;

extern string target_interval;

extern double current_pitch;
extern double target_freq;
extern double target_distnace;

extern double* fft_magnitude;

extern int target_mode;
extern bool activated;

#define WIDTH 600
#define HEIGHT 400
#define MAX_MAGNITUDE (1 << 16)

bool isExit(int wait);

void initGUI();

void updateGUI();