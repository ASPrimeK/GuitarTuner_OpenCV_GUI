
#include <cmath>
#include <vector>
#include <string>

using namespace std;

extern double current_pitch;

extern double target_freq;
extern string target_interval;
extern double target_distnace;

extern double current_pitch;
extern int target_mode;
extern bool activated;

void initUkuleleInterval();
void initGuitarInterval();

void updateClosestInterval(double freq);

void updateTarget();