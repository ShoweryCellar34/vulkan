#ifndef MACROS_H
#define MACROS_H

#define DEFINE_CALLBACK_ARGS_0(callback) \
    static void wrapper_##callback(void* voidCastArgs) { \
        voidCastArgs; \
        callback(); \
    }

#define DEFINE_CALLBACK_ARGS_1(callback, arg1Type) \
    typedef struct { \
        arg1Type arg1; \
    } callbackArgs1_##wrapper_##callback; \
    \
    static void wrapper_##callback(void* voidCastArgs) { \
        callbackArgs1_##wrapper_##callback* args = (callbackArgs1_##wrapper_##callback*)voidCastArgs; \
        callback(args->arg1); \
        free(args); \
    }

#define DEFINE_CALLBACK_ARGS_2(callback, arg1Type, arg2Type) \
    typedef struct { \
        arg1Type arg1; \
        arg2Type arg2; \
    } callbackArgs2_##wrapper_##callback; \
    \
    static void wrapper_##callback(void* voidCastArgs) { \
        callbackArgs2_##wrapper_##callback* args = (callbackArgs2_##wrapper_##callback*)voidCastArgs; \
        callback(args->arg1, args->arg2); \
        free(args); \
    }

#define DEFINE_CALLBACK_ARGS_3(callback, arg1Type, arg2Type, arg3Type) \
    typedef struct { \
        arg1Type arg1; \
        arg2Type arg2; \
        arg3Type arg3; \
    } callbackArgs3_##wrapper_##callback; \
    \
    static void wrapper_##callback(void* voidCastArgs) { \
        callbackArgs3_##wrapper_##callback* args = (callbackArgs3_##wrapper_##callback*)voidCastArgs; \
        callback(args->arg1, args->arg2, args->arg3); \
        free(args); \
    }



#define PUSH_CLEANUP_ARGS_0(callback) \
    pushCleanupCallback(wrapper_##callback, NULL)

#define PUSH_CLEANUP_ARGS_1(callback, passedArg1) \
    do { \
        callbackArgs1_##wrapper_##callback* args = malloc(sizeof(callbackArgs1_##wrapper_##callback)); \
        if (args) { \
            args->arg1 = passedArg1; \
            pushCleanupCallback(wrapper_##callback, args); \
        } \
    } while(0)

#define PUSH_CLEANUP_ARGS_2(callback, passedArg1, passedArg2) \
    do { \
        callbackArgs2_##wrapper_##callback* args = malloc(sizeof(callbackArgs2_##wrapper_##callback)); \
        if (args) { \
            args->arg1 = passedArg1; \
            args->arg2 = passedArg2; \
            pushCleanupCallback(wrapper_##callback, args); \
        } \
    } while(0)

#define PUSH_CLEANUP_ARGS_3(callback, passedArg1, passedArg2, passedArg3) \
    do { \
        callbackArgs3_##wrapper_##callback* args = malloc(sizeof(callbackArgs3_##wrapper_##callback)); \
        if (args) { \
            args->arg1 = passedArg1; \
            args->arg2 = passedArg2; \
            args->arg3 = passedArg3; \
            pushCleanupCallback(wrapper_##callback, args); \
        } \
    } while(0)



#define MAKE_SLICE(label, countType, dataType) \
    typedef struct { \
        countType count; \
        dataType* data; \
    } label##Slice;

#define SLICE_SIZE(slice) \
    sizeof(*slice.data) * slice.count

#define SLICE_DESTROY_IF(slice) if(slice.data) {\
        free(slice.data); \
        slice.data = NULL; \
        slice.count = 0; \
    }

MAKE_SLICE(String, uint32_t, const char*)
MAKE_SLICE(uint32_t, uint32_t, uint32_t)
MAKE_SLICE(LayerProperties, uint32_t, VkLayerProperties)
MAKE_SLICE(ExtensionProperties, uint32_t, VkExtensionProperties)
MAKE_SLICE(SurfaceFormatKHR, uint32_t, VkSurfaceFormatKHR)



#define MAKE_OPTIONAL(label, dataType) \
    typedef struct { \
        bool set; \
        dataType data; \
    } label##Optional;



#define CLAMP(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))

#endif // MACROS_H
