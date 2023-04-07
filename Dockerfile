from ubuntu:jammy

# build dependencies
RUN apt-get update
RUN apt-get install -y build-essential gcc-mips-linux-gnu cmake libtinyxml2-dev git zip

# download, compile and install mkpsxiso
RUN git clone https://github.com/Lameguy64/mkpsxiso/
WORKDIR mkpsxiso
RUN git submodule update --init --recursive
RUN cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build ./build \
    && cmake --install ./build

WORKDIR /

ENTRYPOINT ["/bin/bash"]