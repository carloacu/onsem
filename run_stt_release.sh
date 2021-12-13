#!/bin/sh

if [ ! -d "stt/models/${1}" ]
then
    unzip stt/models/${1}.zip -d stt/models/
fi

./stt/write_asr_result_in_a_file.py -m stt/models/${1} -f build-release/out_asr.txt
