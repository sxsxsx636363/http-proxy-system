FROM ubuntu:20.04
RUN mkdir /src
ADD . /src/
WORKDIR /src
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get -y  --no-install-recommends install
RUN apt-get update
RUN apt-get -yq install libboost-all-dev 
RUN chmod 777 ./http_caching_proxy/run.sh
