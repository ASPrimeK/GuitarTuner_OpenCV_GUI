
#include "gui.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <cstring>

using namespace cv;

Mat screen;

Rect buttons[6];
bool button_state[6];
Scalar button_colors[] = {  // default, selected
    {150, 150, 150},
    {250, 250, 250}};

Rect bars[17];
int bar_states[17];  // off, red, gray, green
int bar_idx;

string buttons_str[6];
string window_name = "Guitar Tuner";

// extern variables
int target_mode = 6;  // [0:5] : (i+1)th string, 6 : auto

Scalar state_colors[] = {
    {100, 100, 100},
    {0, 0, 139},
    {230, 230, 230},
    {0, 242, 0},
};

Scalar darkred(0, 0, 139);
Scalar gray(200, 200, 200);
Scalar green(0, 242, 0);

bool isExit(int wait) {
    return (waitKey(wait) == -1);
}

void mouseCallback(int event, int x, int y, int flags, void* param) {
    if (event == EVENT_LBUTTONDOWN) {
        int clicked_button_idx = -1;
        Point click_position(x, y);

        for (int i = 0; i < 6; i++) {
            if (buttons[i].contains(click_position)) {
                clicked_button_idx = i;
                break;
            }
        }

        if (target_mode == clicked_button_idx) {
            target_mode = 6;
        } else {
            target_mode = clicked_button_idx;
        }
    }
}

void initGUI() {
    screen = Mat::zeros(HEIGHT, WIDTH, CV_8UC3);

    // Buttons
    for (int i = 0; i < 6; i++) {
        buttons[i].x = 100 * i + 10;
        buttons[i].y = 10;
        buttons[i].width = 80;
        buttons[i].height = 80;
    }
    buttons_str[0] = "6:E2";
    buttons_str[1] = "5:A2";
    buttons_str[2] = "4:D3";
    buttons_str[3] = "3:G3";
    buttons_str[4] = "2:B3";
    buttons_str[5] = "1:E4";

    // Bars
    for (int i = 0; i < 17; i++) {
        bars[i].x = 290 + 30 * (i - 8);
        bars[i].y = 200;
        bars[i].width = 20;
        bars[i].height = 100;
    }

    namedWindow(window_name);
    setMouseCallback(window_name, mouseCallback);
}

void drawButtons() {
    for (int i = 0; i < 6; i++) {
        button_state[i] = (target_mode == 6 || target_mode == i);
        rectangle(screen, buttons[i], button_colors[button_state[i]], -1);
        putText(screen, buttons_str[i], {buttons[i].x + 10, 60}, FONT_HERSHEY_DUPLEX, 0.8, {0, 0, 0});
    }
}

void drawBars() {
    for (int i = 0; i < 17; i++) {
        rectangle(screen, bars[i], state_colors[bar_states[i]], -1);
    }
}

void drawText() {
    char temp_str[20] = {};

    sprintf_s(temp_str, "%.2lf Hz", current_pitch);
    putText(screen, temp_str, {240, 350}, FONT_HERSHEY_DUPLEX, 1, {255, 255, 255}, 1);

    putText(screen, target_interval, {260, 180}, FONT_HERSHEY_DUPLEX, 2, {255, 255, 255}, 2);

    sprintf_s(temp_str, "=%.2lf Hz", target_freq);
    putText(screen, temp_str, {350, 180}, FONT_HERSHEY_DUPLEX, 1, {255, 255, 255}, 1);

    sprintf_s(temp_str, "%.3lf ", target_distnace);
    putText(screen, temp_str, {50, 180}, FONT_HERSHEY_DUPLEX, 0.8, {255, 255, 255}, 1);
}

void updateBarState() {
    int bar_idx_tmp = 8;
    double abs_dist = abs(target_distnace);

    if (abs_dist <= (double)1 / 12) {
        for (int i = 0; i <= 8; i++) {
            if ((double)i / 96 <= abs_dist && abs_dist < double(i + 1) / 96) {
                bar_idx_tmp = i;
                break;
            }
        }
    }
    if (target_distnace < 0) bar_idx_tmp = -bar_idx_tmp;
    bar_idx = bar_idx_tmp + 8;

    // Button Color state update
    for (int i = 0; i < 17; i++) {
        bar_states[i] = 0;
    }
    if (bar_idx == 8) {
        bar_states[8] = 3;
    } else if (bar_idx > 8) {
        bar_states[bar_idx] = 2;
        for (int i = 8; i < bar_idx; i++) {
            bar_states[i] = 1;
        }
    } else {
        bar_states[bar_idx] = 2;
        for (int i = 8; i > bar_idx; i--) {
            bar_states[i] = 1;
        }
    }
}

void updateGUI() {
    screen = Mat::zeros(HEIGHT, WIDTH, CV_8UC3);

    drawButtons();

    updateBarState();
    drawBars();

    drawText();

    imshow(window_name, screen);
}