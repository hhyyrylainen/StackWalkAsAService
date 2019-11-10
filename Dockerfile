FROM hhyyrylainen/stackwalk-deps:v1 AS builder

# Copy files to container
RUN mkdir /StackWalk

COPY setup.rb stack_walk_dependencies.rb CMakeLists.txt /StackWalk/
COPY .git /StackWalk/.git
COPY src /StackWalk/src
COPY RubySetupSystem /StackWalk/RubySetupSystem


# Approach running even cloning in docker
# RUN git clone https://github.com/hhyyrylainen/StackWalkAsAService.git /StackWalk && cd /StackWalk && git submodule init

RUN cd /StackWalk && ./setup.rb --no-packagemanager

RUN cd /StackWalk/build && make install

FROM fedora:30

RUN dnf install -y --setopt=deltarpm=false libjpeg zlib boost GraphicsMagick fcgi glew libharu zlib sqlite libpq pango libunwind openssl libpng && dnf clean all

# WORKDIR /stackwalk/
COPY --from=builder /usr/local/bin/stackwalkwebapp /usr/local/bin/minidump_stackwalk /usr/local/bin/
CMD ["/usr/local/bin/stackwalkwebapp"]

