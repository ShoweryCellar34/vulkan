#include <cleanup.h>

// System Headers
#include <stddef.h>
#include <stdlib.h>

// Library Headers

// Project Headers

exitCallback* callbackStack  = NULL;
size_t callbackStackSize     = 0;
size_t callbackStackIterator = 0;

void pushExitCallback(exitCallback callback) {
    if(callbackStack == NULL) {
        callbackStack = malloc(sizeof(exitCallback) * 2);
        callbackStackSize = 2;
        callbackStack[callbackStackIterator++] = callback;
    } else {
        callbackStack[callbackStackIterator++] = callback;
        if(callbackStackIterator == callbackStackSize) {
            size_t newCallbackStackSize = (size_t)((float)callbackStackSize * 1.5f);
            callbackStackSize = newCallbackStackSize;
            callbackStack = realloc(callbackStack, newCallbackStackSize);
        }
    }
}

exitCallback popExitCallback() {
}
