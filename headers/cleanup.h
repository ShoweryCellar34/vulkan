#ifndef CLEANUP_H
#define CLEANUP_H

// System Headers
#include <stdint.h>

// Library Headers

// Project Headers

// This struct stores the function pointer to the callback and a void pointer as arbitrary data to be passed to that function when it is ran
typedef struct {
    void (*callback)(void*);
    void** callbackArgs;
} cleanupCallback;

// Sets up the callback stack so the callbacks will be called when exiting, this is in first in, last out order
void startCleanupCallbacks(void);

// Add a callback to the top of the stack
void pushCleanupCallback(void (*cleanupCallback)(void*), void* cleanupCallbackArgs);

// Remove a callback from the top of the stack and return its infomation
cleanupCallback popCleanupCallback(size_t depth);

// Remove a callback from the top of the stack and return its infomation and call it with the data pointer provided during pushing
cleanupCallback popAndCallCleanupCallback(size_t depth);

#endif // CLEANUP_H
