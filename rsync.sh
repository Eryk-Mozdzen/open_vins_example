#!/bin/bash

rsync -avz \
    calibrate \
    mpu6050 \
    vio \
    emozdzen@10.42.0.164:/home/emozdzen
