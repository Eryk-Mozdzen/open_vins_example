#!/bin/bash

wget -c --no-check-certificate https://cdn3.vision.in.tum.de/tumvi/exported/euroc/512_16/dataset-corridor1_512_16.tar
tar -xf dataset-corridor1_512_16.tar

wget -c --no-check-certificate https://www.research-collection.ethz.ch/bitstreams/7b2419c1-62b5-4714-b7f8-485e5fe3e5fe/download
unzip -o machine_hall.zip
unzip -o machine_hall/MH_01_easy/MH_01_easy.zip -d MH_01_easy
