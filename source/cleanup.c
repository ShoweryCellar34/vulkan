#include <cleanup.h>

// System Headers
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Library Headers

// Project Headers

struct cleanupCallbackNode {
    void (*cleanupCallback)(void*);
    void* cleanupCallbackArgs;
    struct cleanupCallbackNode* next;
    struct cleanupCallbackNode* previous;
};

static struct cleanupCallbackNode* callbackStackHead = NULL;

// This function will be registered with atExit() and pops callbacks one-by-one off the top of the stack and calls them with the data pointer provided during pushing
void startCleanupCallbacks(void) {
    fprintf(stdout, "Executing cleanup callbacks\n");
    while(callbackStackHead != NULL) {
        popAndCallCleanupCallback(0);
    }
}

// Adds a callback to the stack to be run at cleanup
void pushCleanupCallback(void (*cleanupCallback)(void*), void* cleanupCallbackArgs) {
    struct cleanupCallbackNode* newCallbackStackHead = malloc(sizeof(struct cleanupCallbackNode));
    if(newCallbackStackHead == NULL) {
        fprintf(stdout, "Failed to allocate new cleanup callback node\n");
        cleanupCallback(cleanupCallbackArgs);
        exit(EXIT_FAILURE);
    }

    // Set new cleanup callback node data
    newCallbackStackHead->cleanupCallback = cleanupCallback;
    newCallbackStackHead->cleanupCallbackArgs = cleanupCallbackArgs;
    newCallbackStackHead->next = callbackStackHead;
    newCallbackStackHead->previous = NULL;

    // Push cleanup callback to stack
    if(callbackStackHead != NULL) {
        callbackStackHead->previous = newCallbackStackHead;
    }
    callbackStackHead = newCallbackStackHead;
}

// Remove and return the callback down the stack from the top by depth on the stack or if the stack is empty return a NULL callback
cleanupCallback popCleanupCallback(size_t depth) {
    struct cleanupCallbackNode* target = callbackStackHead;
    for(size_t i = 0; i < depth; i++) {
        if(target != NULL) {
            target = target->next;
        } else {
            return (cleanupCallback){
                .callback = NULL,
                .callbackArgs = NULL
            };
        }
    }
    if(target != NULL) {
        callbackStackHead = NULL;
        if(target->next != NULL) {
            target->next->previous = target->previous;
            if(target == callbackStackHead) {
                callbackStackHead = target->next;
            }
        }
        if(target->previous != NULL) {
            target->previous->next = target->next;
            if(target == callbackStackHead) {
                callbackStackHead = target->previous;
            }
        }

        cleanupCallback callback = (cleanupCallback){
            .callback = target->cleanupCallback,
            .callbackArgs = target->cleanupCallbackArgs
        };
        free(target);
        return callback;
    } else {
        return (cleanupCallback){
            .callback = NULL,
            .callbackArgs = NULL
        };
    }
}

// Remove and return the callback down the stack from the top by depth on the stack or if the stack is empty return a NULL callback and call it
cleanupCallback popAndCallCleanupCallback(size_t depth) {
    cleanupCallback callback = popCleanupCallback(depth);
    callback.callback(callback.callbackArgs);
    return callback;
}
