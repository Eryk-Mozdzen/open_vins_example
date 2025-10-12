#!/bin/bash

rsync -avz \
    calibrate \
    mpu6050 \
    vio \
    emozdzen@192.168.168.119:/home/emozdzen
