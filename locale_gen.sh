#!/bin/bash

sudo apt-get install locales
sudo apt-get install zip 
sudo locale-gen es_ES.utf8
sudo locale-gen ja_JP.utf8
sudo locale-gen en_US.utf8
sudo update-locale

# End of script