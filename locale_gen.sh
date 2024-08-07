#!/bin/bash

cmake -S . -B ./build
mkdir logs && mkdir logs/archived_logs
cd build && make
sudo apt-get update
sudo apt-get install -y libc-bin
sudo apt-get install -y zip
sudo apt-get install -y locales
sudo locale-gen es_ES.UTF-8
sudo locale-gen ja_JP.UTF-8
sudo locale-gen en_US.UTF-8

# End of script