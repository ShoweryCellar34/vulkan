#ifndef CLEANUP_H
#define CLEANUP_H

// This struct stores the function pointer to the callback and a void pointer as arbitrary data to be passed to that function when it is ran
typedef struct cleanupCallback {
    void (*callback)(void*);
    void** callbackData;
} cleanupCallback;

// Sets up the callback stack so the callbacks will be called when exiting, this is in first in, last out order
void setupExitCallbacks();

// Add a callback to the top of the stack
void pushExitCallback(cleanupCallback callback);

// Remove a callback from the top of the stack and return its infomation
cleanupCallback popExitCallback();

#endif // CLEANUP_H
