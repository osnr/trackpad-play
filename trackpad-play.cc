#include <math.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

#include "udp-flaschen-taschen.h"

extern "C" {
#include "MiniFB.h"
}

typedef struct { float x,y; } mtPoint;
typedef struct { mtPoint pos,vel; } mtReadout;

typedef struct {
  int frame;
  double timestamp;
  int identifier, state, foo3, foo4;
  mtReadout normalized;
  float size;
  int zero1;
  float angle, majorAxis, minorAxis; // ellipsoid
  mtReadout mm;
  int zero2[2];
  float unk2;
} Finger;

typedef void *MTDeviceRef;
typedef int (*MTContactCallbackFunction)(int,Finger*,int,double,int);

extern "C" {
    MTDeviceRef MTDeviceCreateDefault();
    void MTRegisterContactFrameCallback(MTDeviceRef, MTContactCallbackFunction);
    void MTDeviceStart(MTDeviceRef, int); // thanks comex
}

// #define DISPLAY_WIDTH 8
// #define DISPLAY_HEIGHT 8
#define DISPLAY_WIDTH 45
#define DISPLAY_HEIGHT 40

UDPFlaschenTaschen *canvas;

#define SUPER_WIDTH 400
#define SUPER_HEIGHT 400
int super[SUPER_HEIGHT][SUPER_WIDTH];
struct Window *superWindow;

struct Window *smallWindow;
int small[DISPLAY_HEIGHT][DISPLAY_WIDTH];

int isInsideEllipse(float x0, float y0, float a, float b, float angle, float x, float y) {
    float c = cos(angle) * (x - x0) + sin(angle) * (y - y0);
    float d = sin(angle) * (x - x0) - cos(angle) * (y - y0);
    return (c * c) / (a * a) + (d * d) / (b * b) <= 1;
}
void drawEllipse(float x0, float y0, float a, float b, float angle) {
    const float maxR = fmax(a, b) * 1.414;
    for (int x = (int) (x0 - maxR); x < (int) (x0 + maxR); x++) {
        for (int y = (int) (y0 - maxR); y < (int) (y0 + maxR); y++) {
            if (x >= 0 && x < SUPER_WIDTH && y >= 0 && y < SUPER_HEIGHT &&
                isInsideEllipse(x0, y0, a, b, angle, x, y)) {

                super[y][x] = MFB_RGB(255, 0, 0);
            }
        }
    }
}

int callback(int device, Finger *data, int nFingers, double timestamp, int frame) {
    memset(super, 0, SUPER_WIDTH*SUPER_HEIGHT*sizeof(int));

    for (int i=0; i<nFingers; i++) {
        Finger *f = &data[i];
    
        // printf("Frame %7d: Angle %6.2f, ellipse %6.3f x%6.3f; "
        //        "position (%6.3f,%6.3f) vel (%6.3f,%6.3f) "
        //        "ID %d, state %d [%d %d?] size %6.3f, %6.3f?\n",
        //        f->frame,
        //        f->angle * 90 / atan2(1,0),
        //        f->majorAxis,
        //        f->minorAxis,
        //        f->normalized.pos.x,
        //        f->normalized.pos.y,
        //        f->normalized.vel.x,
        //        f->normalized.vel.y,
        //        f->identifier, f->state, f->foo3, f->foo4,
        //        f->size, f->unk2);

        // plotTouch(f->normalized.pos.x, f->normalized.pos.y);
        drawEllipse(((f->normalized.pos.x) * SUPER_WIDTH), ((1.0f - f->normalized.pos.y) * SUPER_HEIGHT),
                    (f->majorAxis * 2.5), (f->minorAxis * 2.5), f->angle);

        // const Color red(255, f->angle * 90 / atan2(1,0), 0);
        // canvas->SetPixel(f->normalized.pos.x * DISPLAY_WIDTH, (1.0f - f->normalized.pos.y) * DISPLAY_HEIGHT, red);
        // canvas->Send();
    }
    // printf("\n");

    memset(small, 0, DISPLAY_WIDTH*DISPLAY_HEIGHT*sizeof(int));
    float xScale = (float) DISPLAY_WIDTH / (float) SUPER_WIDTH;
    float yScale = (float) DISPLAY_HEIGHT / (float) SUPER_HEIGHT;
    for (int x = 0; x < SUPER_WIDTH; x++) {
        for (int y = 0; y < SUPER_HEIGHT; y++) {
            if (super[y][x] != 0) {
                small[(int) (y * yScale)][(int) (x * xScale)] += 1;
            }
        }
    }

    canvas->Clear();

    float binSize = (float) (SUPER_WIDTH * SUPER_HEIGHT) / (float) (DISPLAY_WIDTH * DISPLAY_HEIGHT);
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            float level = (float) small[y][x] / binSize; // out of 1

            int value = 255 * level;
            small[y][x] = MFB_RGB(value, value, value);

            const Color color(value, value, value);
            canvas->SetPixel(x, y, color);
        }
        // printf("\n");
    }
    canvas->Send();

    return 0;
}

int main() {
    superWindow = mfb_open_ex("hi", SUPER_WIDTH, SUPER_HEIGHT, WF_RESIZABLE);

    smallWindow = mfb_open_ex("small", DISPLAY_WIDTH, DISPLAY_HEIGHT, WF_RESIZABLE);

    // const int socket = OpenFlaschenTaschenSocket("10.20.4.57:1337");
    const int socket = OpenFlaschenTaschenSocket("ft.noise:1337");
    canvas = new UDPFlaschenTaschen(socket, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    canvas->SetOffset(0, 0, 10);

    MTDeviceRef dev = MTDeviceCreateDefault();
    MTRegisterContactFrameCallback(dev, callback);
    MTDeviceStart(dev, 0);
    printf("Ctrl-C to abort\n");

    for (;;) {
        mfb_update(superWindow, super);
        mfb_update(smallWindow, small);
    }

    return 0;
}
