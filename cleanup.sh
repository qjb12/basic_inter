#!/bin/bash

cd ./build && make clean
rm -rf ./*
cd ../logs && find . -type f -delete