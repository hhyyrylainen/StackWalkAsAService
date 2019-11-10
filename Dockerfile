FROM fedora:30 as builder

RUN dnf install -y --setopt=deltarpm=false ruby ruby-devel xcb-util-image-devel systemd-devel libjpeg-devel libvorbis-devel flac-devel openal-soft-devel mesa-libGL-devel libXcomposite libXtst libXScrnSaver atk at-spi2-core-devel at-spi2-atk-devel alsa-lib autoconf automake bzip2 cmake freetype-devel gcc gcc-c++ git libtool make mercurial nasm pkgconfig zlib-devel yasm vulkan-headers vulkan-loader vulkan-loader-devel vulkan-tools vulkan-validation-layers libuuid-devel libX11-devel libXcursor-devel libXrandr-devel libXi-devel mesa-libGLU-devel boost-devel SDL2-devel ImageMagick libXfixes-devel subversion doxygen libXmu-devel git-lfs redhat-lsb-core svn p7zip gcc make redhat-rpm-config fedora-repos-rawhide clang && dnf clean all

# RUN git lfs install

RUN gem install os colorize rubyzip json sha3


