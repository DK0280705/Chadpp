#!/bin/sh

SCRIPT=$(readlink -f $0)
BASEDIR=$(dirname $SCRIPT)
ENV=$(cat $BASEDIR/.env | xargs -d '\n')
cd $BASEDIR/build && env $ENV ./chadpp && cd ..
