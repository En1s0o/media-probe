#! /bin/sh
docker-compose down
docker rmi media-probe:latest
docker-compose up --build -d
