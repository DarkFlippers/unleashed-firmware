#!/usr/bin/env bash

echo "RUN SYNTAX CHECK INSIDE CONTAINER"
docker-compose exec dev ./docker/syntax_check.sh
