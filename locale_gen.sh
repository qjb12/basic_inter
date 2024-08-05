#!/bin/bash

apk add --no-cache libc-utils
apk add --no-cache zip
localedef -i es_ES -f UTF-8 es_ES.UTF-8
localedef -i ja_JP -f UTF-8 ja_JP.UTF-8
localedef -i en_US -f UTF-8 en_US.UTF-8

# End of script