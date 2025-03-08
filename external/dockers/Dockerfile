FROM ubuntu:22.04

LABEL version="0.1"
LABEL description="Docker Image for Building/Testing Verilator"

RUN apt-get update && apt-get install -y locales && rm -rf /var/lib/apt/lists/* \
	&& localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG=en_US.utf8
ENV TZ=Europe/Warsaw

RUN export DEBIAN_FRONTEND=noninteractive && apt-get update -y && apt-get upgrade -y \
 && apt-get install -y make \
 build-essential git help2man python3 make g++ clang cmake ninja-build make \
 git tar bzip2 python3-pip mercurial python3-gi-cairo wget curl libgl1 \
 libvulkan1 xdg-utils graphviz-dev autoconf qt5-qmake libxml2-dev qtbase5-dev \
 qt5-qmake python3-dev pkg-config libpng-dev libfreetype6-dev python3-matplotlib

RUN yes | pip3 install reports cppyy distro requests 
#RUN yes | pip3 install reports cppyy pygraphviz distro requests KiwiDist \
# ipython

RUN useradd -m kl-dev
USER kl-dev
WORKDIR /home/kl-dev/

# Using bake
RUN git clone https://gitlab.com/nsnam/bake.git bake

ENV BAKE_HOME=/home/kl-dev/bake
ENV PATH="$BAKE_HOME/build/bin:$PATH"
ENV PYTHONPATH="$BAKE_HOME/build/lib"
ENV LD_LIBRARY_PATH="$BAKE_HOME/build/lib"

WORKDIR /home/kl-dev/bake
RUN ./bake.py configure -e ns-allinone-3.43
RUN ./bake.py download
RUN ./bake.py build -j16 -vvv
RUN ./bake.py show 

WORKDIR /home/kl-dev/
