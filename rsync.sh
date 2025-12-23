#!/bin/bash

rsync -avz \
    mpu6050 \
    example \
    emozdzen@192.168.168.119:/home/emozdzen
