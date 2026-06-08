#include <cleanup.h>

// System Headers
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers

cleanupCallback* callbackStack  = NULL;
size_t callbackStackSize     = 0;
size_t callbackStackIterator = 0;

void cleanupCallbackStack() {
    if(callbackStack != NULL) {
        free(callbackStack);
        callbackStackSize = 0;
        callbackStackIterator = 0;
    }
}

void runExitCallbacks() {
    while(callbackStackIterator > 0) {
        cleanupCallback callback = popExitCallback();
        callback.callback(callback.callbackData);
    }
}

void setupExitCallbacks() {
    atexit(cleanupCallbackStack);
    atexit(runExitCallbacks);
}

void pushExitCallback(cleanupCallback callback) {
    if(callbackStack == NULL) {
        size_t callbackStackStartingSize = 8;
        callbackStack = malloc(sizeof(cleanupCallback) * callbackStackStartingSize);
        if(callbackStack == NULL) {
            fprintf(stdout, "Failed to allocate callback array of length %zu\n", callbackStackSize);
            callback.callback(callback.callbackData);
            exit(EXIT_FAILURE);
        }
        callbackStackSize = callbackStackStartingSize;
        callbackStack[callbackStackIterator++] = callback;
    } else {
        callbackStack[callbackStackIterator++] = callback;
        if(callbackStackIterator == callbackStackSize) {
            size_t newCallbackStackSize = (size_t)((float)callbackStackSize * 1.5f);
            callbackStackSize = newCallbackStackSize;
            cleanupCallback* newCallbackStack = realloc(callbackStack, sizeof(cleanupCallback) * newCallbackStackSize);
            if(newCallbackStack == NULL) {
                fprintf(stdout, "Failed to reallocate callback array to length %zu from original length %zu\n", newCallbackStackSize, callbackStackSize);
                exit(EXIT_FAILURE);
            }
            callbackStack = newCallbackStack;
        }
    }
}

cleanupCallback popExitCallback() {
    if(callbackStackIterator == 0) {
        return (cleanupCallback){
            .callback =     NULL,
            .callbackData = NULL
        };
    }
    callbackStackIterator--;
    size_t newCallbackStackSize = (size_t)((float)callbackStackSize * 0.67f);
    if(callbackStackIterator < newCallbackStackSize) {
        callbackStackSize = newCallbackStackSize;
        cleanupCallback* newCallbackStack = NULL;
        if(newCallbackStackSize > 0) {
            newCallbackStack = realloc(callbackStack, sizeof(cleanupCallback) * newCallbackStackSize);
            if(newCallbackStack == NULL) {
                fprintf(stdout, "Failed to reallocate callback array to length %zu from original length %zu\n", newCallbackStackSize, callbackStackSize);
                exit(EXIT_FAILURE);
            }
        } else {
            free(callbackStack);
            newCallbackStack = NULL;
        }
        callbackStack = newCallbackStack;
    }
    return callbackStack[callbackStackIterator];
}
