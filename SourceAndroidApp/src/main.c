#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/socket.h>

#include <android/log.h>
#include <android/looper.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <media/NdkImageReader.h>

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "OpenVINS", __VA_ARGS__)

#define STATUS(call)                                                                               \
    do {                                                                                           \
        const int status = call;                                                                   \
        if(status != 0) {                                                                          \
            LOG(#call " status: %d", status);                                                      \
        }                                                                                          \
    } while(0)

typedef struct {
    int64_t timestamp;
    float accel[3];
    float gyro[3];
} frame_imu_t;

typedef struct {
    int socket;
    frame_imu_t imu;
    ASensorEventQueue *queue;
    struct sockaddr_in addr;
} context_t;

static void socket_init(context_t *context) {
    context->socket = socket(AF_INET, SOCK_DGRAM, 0);

    int broadcast = 1;
    setsockopt(context->socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    context->addr.sin_family = AF_INET;
    context->addr.sin_port = htons(4444);

    struct ifaddrs *ifaddr, *ifa;
    if(getifaddrs(&ifaddr) == -1) {
        return;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL)
            continue;
        if(ifa->ifa_addr->sa_family == AF_INET) {
            if(strcmp(ifa->ifa_name, "wlan0") != 0) {
                continue;
            }

            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            struct sockaddr_in *nm = (struct sockaddr_in *)ifa->ifa_netmask;

            const uint32_t ip = sa->sin_addr.s_addr;
            const uint32_t mask = nm->sin_addr.s_addr;

            context->addr.sin_addr.s_addr = (ip & mask) | (~mask);

            freeifaddrs(ifaddr);
            return;
        }
    }

    freeifaddrs(ifaddr);
}

static void socket_transmit(context_t *context, const void *buffer, const size_t size) {
    sendto(context->socket, buffer, size, 0, (struct sockaddr *)&context->addr,
           sizeof(context->addr));
}

static int32_t imu_available(int fd, int events, void *data) {
    (void)fd;
    (void)events;

    context_t *context = data;
    ASensorEvent event;

    while(ASensorEventQueue_getEvents(context->queue, &event, 1) > 0) {
        switch(event.type) {
            case ASENSOR_TYPE_ACCELEROMETER: {
                context->imu.timestamp = event.timestamp;
                context->imu.accel[0] = event.acceleration.x;
                context->imu.accel[1] = event.acceleration.y;
                context->imu.accel[2] = event.acceleration.z;

                // LOG("accel  %20lld %+10.3f %+10.3f %+10.3f", (long long)event.timestamp,
                //     event.acceleration.x, event.acceleration.y, event.acceleration.z);
            } break;
            case ASENSOR_TYPE_GYROSCOPE: {
                context->imu.gyro[0] = event.gyro.x;
                context->imu.gyro[1] = event.gyro.y;
                context->imu.gyro[2] = event.gyro.z;

                // socket_transmit(context, &context->imu, sizeof(context->imu));

                // LOG("gyro   %20lld %+10.3f %+10.3f %+10.3f", (long long)event.timestamp,
                //     event.gyro.x, event.gyro.y, event.gyro.z);
            } break;
        }
    }

    return 1;
}

static void cam_available(void *user, AImageReader *reader) {
    context_t *context = user;

    AImage *image = NULL;
    STATUS(AImageReader_acquireNextImage(reader, &image));

    int64_t timestamp;
    STATUS(AImage_getTimestamp(image, &timestamp));

    uint8_t *buffer;
    int size;
    STATUS(AImage_getPlaneData(image, 0, &buffer, &size));

    LOG("camera %20lld %20p %10d", (long long)timestamp, buffer, size);

    socket_transmit(context, buffer, size);

    AImage_delete(image);
}

void android_main(struct android_app *app) {
    app_dummy();

    app->onAppCmd = NULL;

    context_t context;
    socket_init(&context);

    {
        ASensorManager *manager = ASensorManager_getInstance();
        const ASensor *accel = ASensorManager_getDefaultSensor(manager, ASENSOR_TYPE_ACCELEROMETER);
        const ASensor *gyro = ASensorManager_getDefaultSensor(manager, ASENSOR_TYPE_GYROSCOPE);

        ALooper *looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        context.queue = ASensorManager_createEventQueue(manager, looper, ALOOPER_POLL_CALLBACK,
                                                        imu_available, &context);

        ASensorEventQueue_enableSensor(context.queue, accel);
        ASensorEventQueue_setEventRate(context.queue, accel, 1000000 / 250);

        ASensorEventQueue_enableSensor(context.queue, gyro);
        ASensorEventQueue_setEventRate(context.queue, gyro, 1000000 / 250);
    }

    AImageReader *reader = NULL;
    STATUS(AImageReader_new(640, 480, AIMAGE_FORMAT_YUV_420_888, 4, &reader));

    AImageReader_ImageListener listener = {
        .context = &context,
        .onImageAvailable = cam_available,
    };
    STATUS(AImageReader_setImageListener(reader, &listener));

    ACameraManager *manager = ACameraManager_create();

    ACameraIdList *list = NULL;
    STATUS(ACameraManager_getCameraIdList(manager, &list));
    for(int i = 0; i < list->numCameras; i++) {
        LOG("list[%i] = \"%s\"", i, list->cameraIds[i]);
    }

    ACameraDevice *device = NULL;
    ACameraDevice_StateCallbacks deviceCallbacks = {
        .context = NULL,
        .onDisconnected = NULL,
        .onError = NULL,
    };
    STATUS(ACameraManager_openCamera(manager, list->cameraIds[2], &deviceCallbacks, &device));

    ACameraManager_deleteCameraIdList(list);

    ACaptureSessionOutputContainer *outputContainer = NULL;
    STATUS(ACaptureSessionOutputContainer_create(&outputContainer));

    ANativeWindow *window = NULL;
    STATUS(AImageReader_getWindow(reader, &window));

    ACaptureSessionOutput *sessionOutput = NULL;
    STATUS(ACaptureSessionOutput_create(window, &sessionOutput));
    STATUS(ACaptureSessionOutputContainer_add(outputContainer, sessionOutput));

    ACaptureRequest *request = NULL;
    STATUS(ACameraDevice_createCaptureRequest(device, TEMPLATE_RECORD, &request));

    ACameraOutputTarget *target = NULL;
    STATUS(ACameraOutputTarget_create(window, &target));
    STATUS(ACaptureRequest_addTarget(request, target));

    ACameraCaptureSession *captureSession = NULL;
    ACameraCaptureSession_stateCallbacks captureSessionCallbacks = {
        .context = NULL,
        .onActive = NULL,
        .onReady = NULL,
        .onClosed = NULL,
    };
    STATUS(ACameraDevice_createCaptureSession(device, outputContainer, &captureSessionCallbacks,
                                              &captureSession));

    STATUS(ACameraCaptureSession_setRepeatingRequest(captureSession, NULL, 1, &request, NULL));

    while(1) {
        int events;
        struct android_poll_source *source;
        while(ALooper_pollAll(0, NULL, &events, (void **)&source) >= 0) {
            if(source) {
                source->process(source->app, source);
            }
        }
    }
}
