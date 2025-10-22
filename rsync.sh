#!/bin/bash

rsync -avz \
    mpu6050 \
    vio \
    recorder \
    emozdzen@192.168.168.119:/home/emozdzen
