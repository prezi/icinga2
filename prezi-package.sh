#!/bin/bash

mkdir -p debs
rm -rf debs/*

if docker ps | grep samba-server >/dev/null 2>/dev/null; then
    echo "samba sharing container is running"
else
    echo "samba sharing container is NOT running, starting one"
    docker run -v /data --name sharing busybox true
    docker run --rm -v /usr/local/bin/docker:/docker -v /var/run/docker.sock:/docker.sock svendowideit/samba sharing
fi
run_opts="--volumes-from sharing"

docker build -t icinga2-build .
docker run $run_opts icinga2-build sh -c 'mkdir -p /data/icinga2-debs && rm -rf /data/icinga2-debs/* && cp /opt/icinga2-build/*.deb /data/icinga2-debs && ls /data/icinga2-debs'

echo "The debs are now in a shared Samba folder /data/icinga2-debs accessible through the docker container called 'sharing'."
echo "On boot2docker it's also accessible through boot2docker."
