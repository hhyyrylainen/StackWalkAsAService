FROM hhyyrylainen/stackwalk-deps:v2 AS builder

# Copy files to container
RUN mkdir /StackWalk

COPY setup.rb stack_walk_dependencies.rb CMakeLists.txt /StackWalk/
COPY .git /StackWalk/.git
COPY src /StackWalk/src
COPY config /StackWalk/config
COPY RubySetupSystem /StackWalk/RubySetupSystem


# Approach running even cloning in docker
# RUN git clone https://github.com/hhyyrylainen/StackWalkAsAService.git /StackWalk && cd /StackWalk && git submodule init

# The rm part here is to keep disk usage down
# The stripping is done also here to save disk space
RUN cd /StackWalk && ./setup.rb --no-packagemanager && rm -rf /StackWalk/ThirdParty && \
    strip /StackWalk/build/ThirdParty/**/*.so*

RUN cd /StackWalk/build && make install

FROM fedora:35

RUN dnf install -y --setopt=deltarpm=false libjpeg zlib boost GraphicsMagick fcgi glew \
    libharu zlib sqlite libpq pango libunwind openssl libpng && dnf clean all

# Copy binaries and resources from the build image
COPY --from=builder /usr/local/bin/stackwalkwebapp /usr/local/bin/minidump_stackwalk \
    /usr/local/bin/minidump_stackwalk_mingw /usr/local/bin/
# Which of these two works depends on the distro
# COPY --from=builder /usr/local/lib/libwt*.so* /usr/lib/
COPY --from=builder /usr/local/lib64/libwt*.so* /usr/lib64/
COPY --from=builder /usr/local/etc/StackWalkAsAService/wt_config.xml \
    /usr/local/etc/StackWalkAsAService/
COPY --from=builder /usr/local/var/StackWalkAsAService/http \
    /usr/local/var/StackWalkAsAService/http

# WORKDIR /stackwalk/
ENTRYPOINT ["/usr/local/bin/stackwalkwebapp", "--docroot", \
    "/usr/local/var/StackWalkAsAService/http", \
    "-c", "/usr/local/etc/StackWalkAsAService/wt_config.xml", "--http-address", "0.0.0.0"]

