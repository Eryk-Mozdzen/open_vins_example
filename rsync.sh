#!/bin/bash

rsync -avz \
    config \
    datasets/download.sh \
    example \
    mpu6050 \
    emozdzen@192.168.0.136:/home/emozdzen
