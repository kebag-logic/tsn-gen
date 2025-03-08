#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
DOCKER_CONTAINER_NAME=kl-ns-3-43-container
DOCKER_IMAGE_NAME=kl/ns-3-43

# build image
cd $SCRIPTPATH

if [ -z "$(docker images -q $DOCKER_IMAGE_NAME:latest 2> /dev/null)" ]; then
	echo "Docker image ${DOCKER_IMAGE_NAME}:latest does not exists..."
	echo "Creating it..."
	docker build -t ${DOCKER_IMAGE_NAME} .  --debug
fi

if [ ! "$(docker ps -q -f name=$DOCKER_CONTAINER_NAME  2> /dev/null)" ]; then
	docker create -it --name $DOCKER_CONTAINER_NAME \
		--user kl-dev ${DOCKER_IMAGE_NAME}:latest /bin/bash
fi
