#ifndef CLEANUP_H
#define CLEANUP_H

typedef struct exitCallback {
    void (*callback)(void*);
    void** callbackData;
} exitCallback;

void pushExitCallback(exitCallback callback);

exitCallback popExitCallback();

#endif // CLEANUP_H
