#!/bin/bash

rsync -avz \
    mpu6050 \
    vio \
    emozdzen@192.168.0.19:/home/emozdzen
