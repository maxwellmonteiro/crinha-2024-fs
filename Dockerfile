# FROM gcc:13.2
FROM ubuntu:23.04

RUN apt-get update && apt-get install -y gcc build-essential && apt-get clean

COPY Makefile .
COPY src src
COPY include include
COPY lib lib

RUN make clean all