#include "fps.h"
#include <time.h>

#define FPS_SAMPLES 60
#define MS_IN_SECOND 1000
#define MAX_FPS 999.0f

typedef struct {
    long frameTimesMs[FPS_SAMPLES];
    int currentIndex;
    long totalTimeMs;
    float currentFps;
    long lastTimeMs;
} FPSCounter;

static FPSCounter fpsCounter;

static long get_current_ms() {
	return (long) (((double) clock() / CLOCKS_PER_SEC) * MS_IN_SECOND);
}

void fps_init() {
	for (int i = 0; i < FPS_SAMPLES; i++) {
		fpsCounter.frameTimesMs[i] = 0;
	}
	fpsCounter.currentIndex = 0;
	fpsCounter.totalTimeMs = 0;
	fpsCounter.currentFps = 0.0f;

	fpsCounter.lastTimeMs = get_current_ms();
}

void fps_update() {
	long currentTime = get_current_ms();
	long deltaMs = currentTime - fpsCounter.lastTimeMs;
	fpsCounter.lastTimeMs = currentTime;

	fpsCounter.totalTimeMs -= fpsCounter.frameTimesMs[fpsCounter.currentIndex];

	fpsCounter.frameTimesMs[fpsCounter.currentIndex] = deltaMs;
	fpsCounter.totalTimeMs += deltaMs;

	fpsCounter.currentIndex = (fpsCounter.currentIndex + 1) % FPS_SAMPLES;

	if (fpsCounter.totalTimeMs > 0) {
		fpsCounter.currentFps = (float) FPS_SAMPLES / ((float) fpsCounter.totalTimeMs / MS_IN_SECOND);
	} else {
		fpsCounter.currentFps = MAX_FPS;
	}
}

float fps_get() {
    return fpsCounter.currentFps;
}
