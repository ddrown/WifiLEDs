#!/bin/sh

USERAUTH="root:changeme"

# candy cane
#curl --user $USERAUTH -X POST -d "brightness=255&pattern=4&static_color_0=#FF0000&static_color_1=#FFFFFF" http://esp8266fs.lan/post/user

# red/green twinkle
#curl --user $USERAUTH -X POST -d "brightness=255&pattern=4&static_color_0=#FF0000&static_color_1=#00FF00" http://esp8266fs.lan/post/user

# red/white/green/black circle
#curl --user $USERAUTH -X POST -d "brightness=255&pattern=7&static_color_0=#FF0000&static_color_1=#FFFFFF&static_color_2=#00FF00&static_color_3=#000000" http://esp8266fs.lan/post/user

# red/yellow/green/yellow circle
#curl --user $USERAUTH -X POST -d "brightness=255&pattern=7&static_color_0=#FF0000&static_color_1=#808000&static_color_2=#00FF00&static_color_3=#808000" http://esp8266fs.lan/post/user

# rainbow circle
#curl --user $USERAUTH -X POST -d "brightness=255&pattern=7&static_color_0=#0000FF&static_color_1=#00FF00&static_color_2=#FFFF00&static_color_3=#FF0000" http://esp8266fs.lan/post/user

# off
#curl --user $USERAUTH -X POST -d "brightness=0" http://esp8266fs.lan/post/user
