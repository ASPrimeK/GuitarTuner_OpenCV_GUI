#include "interval.hpp"

// Extern variables
double target_freq;
double target_distnace;

extern int target_mode;  // [0:5] : (i+1)th string, 6 : auto

vector<pair<string, double>> target_pair;
string target_interval;

void initGuitarInterval() {
    target_pair.push_back({"E2", 82.41});
    target_pair.push_back({"A2", 110.00});
    target_pair.push_back({"D3", 146.83});
    target_pair.push_back({"G3", 196.00});
    target_pair.push_back({"B3", 246.94});
    target_pair.push_back({"E4", 329.63});
}

// void initUkuleleInterval() {
//     target_pair.push_back({"G4", 392});
//     target_pair.push_back({"C4", 262});
//     target_pair.push_back({"E4", 330});
//     target_pair.push_back({"A4", 440});
// }

double getDistance(double freq, double target) {
    return log2(freq / target);
}

void updateClosestInterval(double freq) {
    int num_of_target = (int)target_pair.size();

    int closest_idx = 0;
    double min_distance = 9999999;

    for (int i = 0; i < num_of_target; i++) {
        double dist = getDistance(freq, target_pair[i].second);
        if (abs(dist) < abs(min_distance)) {
            min_distance = dist;
            closest_idx = i;
        }
    }

    target_interval = target_pair[closest_idx].first;
    target_freq = target_pair[closest_idx].second;
    target_distnace = min_distance;
}

void updateTarget() {
    if (target_mode == 6) {
        updateClosestInterval(current_pitch);
    } else {
        target_interval = target_pair[target_mode].first;
        target_freq = target_pair[target_mode].second;
        target_distnace = getDistance(current_pitch, target_freq);
    }
}