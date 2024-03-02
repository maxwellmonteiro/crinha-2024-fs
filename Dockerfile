FROM ubuntu:23.04

RUN apt-get update && apt-get install -y gcc build-essential nodejs npm uuid-dev && apt-get clean

WORKDIR /usr/local/rinha

COPY llhttp-8.1.1.tar.gz .
RUN tar xzf llhttp-8.1.1.tar.gz
WORKDIR /usr/local/rinha/llhttp
RUN npm ci && make && make install
WORKDIR /usr/local/rinha
COPY jansson-2.14.tar.gz .
RUN tar xzf jansson-2.14.tar.gz
WORKDIR jansson-2.14
RUN ./configure && make && make install
WORKDIR /usr/local/rinha

COPY Makefile .
COPY src src

RUN make clean all