#include <cleanup.h>

// System Headers
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers

static cleanupCallback* callbackStack = NULL;
static size_t callbackStackSize       = 0;
static size_t callbackStackIterator   = 0;

// This function will be registered with atExit() and pops callbacks one-by-one off the top of the stack and calls them with the data pointer provided during pushing
void startCleanupCallbacks(void) {
    while(callbackStackIterator > 0) {
        cleanupCallback callback = popCleanupCallback();
        callback.callback(callback.callbackData);
    }
}

// Adds a callback to the stack to be run at cleanup
void pushCleanupCallback(cleanupCallback callback) {
    // If we have not allocated the callback stack we allocate it, or if we have allocated it already, we reallocate it
    if(callbackStack == NULL) {
        // Define a starting size for our stack
        size_t callbackStackStartingSize = 8;
        // Allocate the callback stack
        callbackStack = malloc(sizeof(cleanupCallback) * callbackStackStartingSize);
        // Check if the allocation was successful
        if(callbackStack == NULL) {
            // If the allocation failed we print a message and call the callback requested to be push to the stack as to not miss any cleanup code
            fprintf(stdout, "Failed to allocate callback array of length %zu\n", callbackStackSize);
            callback.callback(callback.callbackData);
            exit(EXIT_FAILURE);
        }
        // If we succeed we add the callback to the stack and increment the iterator to set it to the next stack callback index
        callbackStackSize = callbackStackStartingSize;
        callbackStack[callbackStackIterator++] = callback;
    } else {
        // Add the callback to the stack and increment the iterator to set it to the next stack callback index
        callbackStack[callbackStackIterator++] = callback;
        // If the stack if full we grow the callback stack
        if(callbackStackIterator == callbackStackSize) {
            // Set the new callback stack size to 1.5 times what it was before
            size_t newCallbackStackSize = (size_t)((float)callbackStackSize * 1.5f);
            // Reallocate the callback stack to the new size
            cleanupCallback* newCallbackStack = realloc(callbackStack, sizeof(cleanupCallback) * newCallbackStackSize);
            // If the reallocation failed we exit
            if(newCallbackStack == NULL) {
                fprintf(stdout, "Failed to reallocate callback array to length %zu from original length %zu\n", newCallbackStackSize, callbackStackSize);
                exit(EXIT_FAILURE);
            }
            // If the reallocation succeeded we set the callback stack and size to the new values
            callbackStackSize = newCallbackStackSize;
            callbackStack = newCallbackStack;
        }
    }
}

// Remove and return the top callback on the stack
cleanupCallback popCleanupCallback() {
    // If there are no callbacks in the callback stack we return a NULL callback
    if(callbackStackIterator == 0) {
        return (cleanupCallback){
            .callback =     NULL,
            .callbackData = NULL
        };
    }
    // Decrement the callback stack iterator
    callbackStackIterator--;
    // Set the new callback stack size to 0.67 times what it was before
    size_t newCallbackStackSize = (size_t)((float)callbackStackSize * 0.67f);
    // If the callback stack iterator is smaller than or equal to the new callback stack size then we shrink the callback stack
    if(callbackStackIterator <= newCallbackStackSize) {
        // If the new callback stack size is zero than we destroy the callback stack if it is greater than zero than we shrink the callback stack
        cleanupCallback* newCallbackStack = NULL;
        if(newCallbackStackSize > 0) {
            newCallbackStack = realloc(callbackStack, sizeof(cleanupCallback) * newCallbackStackSize);
            // If shrinking the callback stack failed than we exit and call the callback we popped as to not miss any cleanup functionality
            if(newCallbackStack == NULL) {
                fprintf(stdout, "Failed to reallocate callback array to length %zu from original length %zu\n", newCallbackStackSize, callbackStackSize);
                callbackStack[callbackStackIterator].callback(callbackStack[callbackStackIterator].callbackData);
                exit(EXIT_FAILURE);
            }
        } else {
            free(callbackStack);
            newCallbackStack = NULL;
        }
        // If we succeed then we set the callback stack and size to the new values
        callbackStackSize = newCallbackStackSize;
        callbackStack = newCallbackStack;
    }
    // Finally we return the callback we popped
    return callbackStack[callbackStackIterator];
}

// Remove and return the top callback on the stack and call it with the data pointer provided during pushing
cleanupCallback popAndCallCleanupCallback() {
    cleanupCallback callback = popCleanupCallback();
    callback.callback(callback.callbackData);
    return callback;
}
