#include <cleanup.h>

// System Headers
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers

typedef struct {
    void (*cleanupCallbackWrapper)(void*);
    void* cleanupCallbackArgs;
    struct cleanupCallbackNode* next;
} cleanupCallbackNode;

static cleanupCallbackNode* callbackStackHead = NULL;

// This function will be registered with atExit() and pops callbacks one-by-one off the top of the stack and calls them with the data pointer provided during pushing
void startCleanupCallbacks(void) {
    fprintf(stdout, "Executing cleanup callbacks\n");
    while(callbackStackHead != NULL) {
        cleanupCallbackNode* callbackNode = callbackStackHead;
        callbackStackHead = callbackNode->next;
        callbackNode->cleanupCallbackWrapper(callbackNode->cleanupCallbackArgs);
        free(callbackNode);
    }
}

// Adds a callback to the stack to be run at cleanup
void pushCleanupCallback(void (*cleanupCallbackWrapper)(void*), void* cleanupCallbackArgs) {
    cleanupCallbackNode* newCallbackStackHead = malloc(sizeof(cleanupCallbackNode));
    if(newCallbackStackHead == NULL) {
        fprintf(stdout, "Failed to allocate new cleanup callback node\n");
        cleanupCallbackWrapper(cleanupCallbackArgs);
        exit(EXIT_FAILURE);
    }

    // Set new cleanup callback node data
    newCallbackStackHead->cleanupCallbackWrapper = cleanupCallbackWrapper;
    newCallbackStackHead->cleanupCallbackArgs = cleanupCallbackArgs;

    // Push cleanup callback to stack
    newCallbackStackHead->next = callbackStackHead;
    callbackStackHead = newCallbackStackHead;
}

// Remove and return the top callback on the stack or if the stack is empty return a NULL callback
cleanupCallback popCleanupCallback() {
    if(callbackStackHead != NULL) {
        cleanupCallbackNode* callbackNode = callbackStackHead;
        callbackStackHead = callbackNode->next;
        cleanupCallback callback = (cleanupCallback){
            .callback = callbackNode->cleanupCallbackWrapper,
            .callbackData = callbackNode->cleanupCallbackArgs
        };
        free(callbackNode);
        return callback;
    } else {
        return (cleanupCallback){
            .callback = NULL,
            .callbackData = NULL
        };
    }
}

// Remove and return the top callback on the stack and call it with the data pointer provided during pushing
cleanupCallback popAndCallCleanupCallback() {
    cleanupCallback callback = popCleanupCallback();
    callback.callback(callback.callbackData);
    return callback;
}
