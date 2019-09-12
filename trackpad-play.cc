#include <math.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

#include "udp-flaschen-taschen.h"

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

#define DISPLAY_WIDTH 8
#define DISPLAY_HEIGHT 8

UDPFlaschenTaschen *canvas;

int callback(int device, Finger *data, int nFingers, double timestamp, int frame) {
    canvas->Clear();
    for (int i=0; i<nFingers; i++) {
        Finger *f = &data[i];
    
        printf("Frame %7d: Angle %6.2f, ellipse %6.3f x%6.3f; "
               "position (%6.3f,%6.3f) vel (%6.3f,%6.3f) "
               "ID %d, state %d [%d %d?] size %6.3f, %6.3f?\n",
               f->frame,
               f->angle * 90 / atan2(1,0),
               f->majorAxis,
               f->minorAxis,
               f->normalized.pos.x,
               f->normalized.pos.y,
               f->normalized.vel.x,
               f->normalized.vel.y,
               f->identifier, f->state, f->foo3, f->foo4,
               f->size, f->unk2);

        const Color red(255, f->angle * 90 / atan2(1,0), 0);
        canvas->SetPixel(f->normalized.pos.x * DISPLAY_WIDTH, (1.0f - f->normalized.pos.y) * DISPLAY_HEIGHT, red);
        canvas->Send();
    }
    printf("\n");
    return 0;
}

int main() {
    const int socket = OpenFlaschenTaschenSocket("10.20.4.57:1337");
    canvas = new UDPFlaschenTaschen(socket, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    MTDeviceRef dev = MTDeviceCreateDefault();
    MTRegisterContactFrameCallback(dev, callback);
    MTDeviceStart(dev, 0);
    printf("Ctrl-C to abort\n");
    sleep(-1);
    return 0;
}