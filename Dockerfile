
FROM ubuntu:23.04

RUN apt-get update && apt-get install -y gcc build-essential uuid-dev nodejs npm && apt-get clean

WORKDIR /usr/local/rinha

COPY llhttp-8.1.1.tar.gz .
RUN tar xzf llhttp-8.1.1.tar.gz
WORKDIR llhttp
RUN npm ci && make && make install
WORKDIR /usr/local/rinha
COPY jansson-2.14.tar.gz .
RUN tar xzf jansson-2.14.tar.gz
WORKDIR jansson-2.14
RUN ./configure && make && make install
WORKDIR /usr/local/rinha

RUN rm -f llhttp-8.1.1.tar.gz jansson-2.14.tar.gz
RUN rm -rf ./llhttp ./jansson-2.14
RUN apt-get remove --purge -y nodejs npm

COPY Makefile .
COPY src src

RUN make clean all