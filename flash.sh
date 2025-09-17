#!/bin/bash

rsync -avz --delete \
    install \
    vio/build/vio \
    vio/config.yaml \
    vio/imu.yaml \
    vio/imucam.yaml \
    vio/mask0.png \
    vio/mask1.png \
    vio/datasets \
    emozdzen@192.168.0.19:/home/emozdzen
