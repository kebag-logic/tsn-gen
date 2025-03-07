
FROM ubuntu:22.04

LABEL version="0.1"
LABEL description="Docker Image for Building/Testing Verilator"

RUN 
RUN apt-get update -y && apt-get install -y make build-essential git help2man \
 python3 make clang++ g++ cmake ninja make git tar bungzip2 python3-pip \
 mercurial python-gi-cairo wget curl libgl1 libvulkan1 xdg-utils

RUN pip3 install reports cppyy pygraphviz 

RUN useradd -m kl-dev
USER kl-dev
WORKDIR /home/kl-dev/

# Using bake
RUN git clone https://gitlab.com/nsnam/bake.git

ENV BAKE_HOME="/home/kl-dev/bake"
ENV PATH="$BAKE_HOME/build/bin:$PATH"
ENV PYTHONPATH="$BAKE_HOME/build/lib"
ENV LD_LIBRARY_PATH="$BAKE_HOME/build/lib"

WORKDIR /home/kl-dev/bake
RUN ./bake.py download && ./bake.py build -j16 -vvv && ./bake.py show 

WORKDIR /home/kl-dev/
