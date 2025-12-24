#!/bin/bash

rsync -avz \
    config \
    example \
    mpu6050 \
    emozdzen@192.168.168.119:/home/emozdzen
