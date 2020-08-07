echo copying file to latest
copy /y %1 %2
echo deploy as release
windeployqt --no-quick-import --no-translations --no-system-d3d-compiler --no-virtualkeyboard --no-webkit2 --no-opengl-sw %2

