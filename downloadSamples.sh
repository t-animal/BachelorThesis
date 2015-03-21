#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
	echo "Specify a target subdirectory"
	exit 1;
fi

adb pull /sdcard/Android/data/de.t_animal.goboardreader/files ./files/$1
adb shell "mkdir -p /sdcard/Android/data/de.t_animal.goboardreader/downloaded/$1"
adb shell "mv /sdcard/Android/data/de.t_animal.goboardreader/files/* /sdcard/Android/data/de.t_animal.goboardreader/downloaded/$1"

mkdir -p files/all/unprocessed
mkdir -p files/all/processed
mkdir -p files/$1/unprocessed
mkdir -p files/$1/processed
mv files/$1/*unprocessed.{png,yml} files/$1/unprocessed
mv files/$1/*processed.png files/$1/processed

for i in files/$1/unprocessed/*; do
	ln -s "../../../$i" "files/all/unprocessed/"
done

for i in files/$1/processed/*; do
	ln -s "../../../$i" "files/all/processed/"
done
