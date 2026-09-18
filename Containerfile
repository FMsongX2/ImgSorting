FROM registry.fedoraproject.org/fedora:42

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc-c++ cmake make SDL3-devel \
        mesa-dri-drivers mesa-libGL mesa-libEGL \
        libX11 libXext libXcursor libXi libXrandr libXfixes libXScrnSaver libxkbcommon libxkbcommon-x11 \
        libwayland-client libwayland-cursor libwayland-egl libdecor \
        pulseaudio-libs pipewire-libs alsa-lib \
        zenity ffmpeg-free \
    && dnf clean all

WORKDIR /app
COPY CMakeLists.txt ./
COPY src src
COPY tests tests
COPY third_party third_party
COPY assets assets

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j"$(nproc)" \
    && ctest --test-dir build --output-on-failure

ENTRYPOINT ["/app/build/ImgSorting"]
