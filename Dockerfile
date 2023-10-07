FROM debian:bullseye

RUN apt-get update
RUN apt-get install -y gcc make

COPY . /app

WORKDIR /app
