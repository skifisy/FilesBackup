FROM fedora:40

WORKDIR /root

RUN dnf -y update \
    && set -x; dnf install -y git cmake make g++  \
    gtest gtest-devel openssl-devel qt5-qtbase-devel \
    diffutils langpacks-zh_CN

RUN git clone https://github.com/skyfishine/FilesBackup

RUN cd FilesBackup && mkdir thirdparty && cd thirdparty \
    && git clone https://github.com/nlohmann/json

RUN cd FilesBackup && mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make -j && ctest && make install

WORKDIR /root/FilesBackup