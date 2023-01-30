#!/bin/bash

gcc -lusb-1.0 KD100.c -o KD100
gcc -lX11 -lXtst -lXinerama -lxkbcommon -Lxdo.h -Lxdo.c handler.c -o handler
