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

static int32_t imu_available(int fd, int events, void *data) {
    (void)fd;
    (void)events;

    ASensorEventQueue **queue = data;
    ASensorEvent event;

    while(ASensorEventQueue_getEvents(*queue, &event, 1) > 0) {
        switch(event.type) {
            case ASENSOR_TYPE_ACCELEROMETER: {
                LOG("accel  %20lld %+10.3f %+10.3f %+10.3f", (long long)event.timestamp,
                    event.acceleration.x, event.acceleration.y, event.acceleration.z);
            } break;
            case ASENSOR_TYPE_GYROSCOPE: {
                LOG("gyro   %20lld %+10.3f %+10.3f %+10.3f", (long long)event.timestamp,
                    event.vector.x, event.vector.y, event.vector.z);
            } break;
        }
    }

    return 1;
}

static void cam_available(void *context, AImageReader *reader) {
    (void)context;

    AImage *image = NULL;

    if(AImageReader_acquireNextImage(reader, &image) == AMEDIA_OK) {
        int64_t timestamp;
        AImage_getTimestamp(image, &timestamp);

        LOG("camera %20lld", (long long)timestamp);

        AImage_delete(image);
    }
}

void android_main(struct android_app *app) {
    app_dummy();

    app->onAppCmd = NULL;

    {
        ASensorManager *manager = ASensorManager_getInstance();
        const ASensor *accel = ASensorManager_getDefaultSensor(manager, ASENSOR_TYPE_ACCELEROMETER);
        const ASensor *gyro = ASensorManager_getDefaultSensor(manager, ASENSOR_TYPE_GYROSCOPE);

        ALooper *looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        ASensorEventQueue *queue = ASensorManager_createEventQueue(
            manager, looper, ALOOPER_POLL_CALLBACK, imu_available, &queue);

        ASensorEventQueue_enableSensor(queue, accel);
        ASensorEventQueue_setEventRate(queue, accel, 1000000 / 250);

        ASensorEventQueue_enableSensor(queue, gyro);
        ASensorEventQueue_setEventRate(queue, gyro, 1000000 / 250);
    }

    AImageReader *reader = NULL;
    STATUS(AImageReader_new(640, 480, AIMAGE_FORMAT_YUV_420_888, 4, &reader));

    AImageReader_ImageListener listener = {
        .context = NULL,
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
