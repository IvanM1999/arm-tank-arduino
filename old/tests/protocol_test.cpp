#include <cassert>
#include <cstdio>

struct Frame {
    int base;
    int reach;
    int elevation;
    int velocity;
    int direction;
};

static bool parseFrame(const char *payload, Frame &frame) {
    int fields = std::sscanf(payload, "%d,%d,%d,%d,%d", &frame.base,
                              &frame.reach, &frame.elevation, &frame.velocity,
                              &frame.direction);
    return fields == 5 && frame.base >= 0 && frame.base <= 180 &&
           frame.reach >= 0 && frame.reach <= 180 && frame.elevation >= 0 &&
           frame.elevation <= 180 && frame.velocity >= -100 &&
           frame.velocity <= 100 && frame.direction >= -100 &&
           frame.direction <= 100;
}

static int clamp(int value, int low, int high) {
    return value < low ? low : value > high ? high : value;
}

static int pwmFor(int command) {
    command = clamp(command, -100, 100);
    return (command + 100) * 255 / 200;
}

int main() {
    Frame frame{};
    assert(parseFrame("90,120,80,-30,20", frame));
    assert(frame.velocity == -30 && frame.direction == 20);
    assert(!parseFrame("90,120,80,-30", frame));
    assert(!parseFrame("90,120,181,0,0", frame));
    assert(!parseFrame("90,120,80,-101,0", frame));
    assert(pwmFor(clamp(-30 + 20, -100, 100)) == 114);
    assert(pwmFor(clamp(-30 - 20, -100, 100)) == 63);
    assert(pwmFor(500) == 255);
    std::puts("protocol_test: ok");
}