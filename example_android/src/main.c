#include <android/log.h>
#include <android/looper.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "OpenVINS", __VA_ARGS__)

static int32_t handle(int fd, int events, void *data) {
    (void)fd;
    (void)events;

    ASensorEventQueue **queue = data;
    ASensorEvent event;

    while(ASensorEventQueue_getEvents(*queue, &event, 1) > 0) {
        switch(event.type) {
            case ASENSOR_TYPE_ACCELEROMETER: {
                LOG("accel %20lld %+10.3f %+10.3f %+10.3f", (long long)event.timestamp,
                    event.acceleration.x, event.acceleration.y, event.acceleration.z);
            } break;
            case ASENSOR_TYPE_GYROSCOPE: {
                LOG("gyro  %20lld %+10.3f %+10.3f %+10.3f", (long long)event.timestamp,
                    event.vector.x, event.vector.y, event.vector.z);
            } break;
        }
    }

    return 1;
}

void android_main(struct android_app *app) {
    app_dummy();

    app->onAppCmd = NULL;

    ASensorManager *mgr = ASensorManager_getInstance();
    const ASensor *accel = ASensorManager_getDefaultSensor(mgr, ASENSOR_TYPE_ACCELEROMETER);
    const ASensor *gyro = ASensorManager_getDefaultSensor(mgr, ASENSOR_TYPE_GYROSCOPE);

    ALooper *looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ASensorEventQueue *queue =
        ASensorManager_createEventQueue(mgr, looper, ALOOPER_POLL_CALLBACK, handle, &queue);

    if(accel) {
        ASensorEventQueue_enableSensor(queue, accel);
        ASensorEventQueue_setEventRate(queue, accel, 1000000 / 200);
    }

    if(gyro) {
        ASensorEventQueue_enableSensor(queue, gyro);
        ASensorEventQueue_setEventRate(queue, gyro, 1000000 / 200);
    }

    while(1) {
        ALooper_pollAll(-1, NULL, NULL, NULL);
    }
}
