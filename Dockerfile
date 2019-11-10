FROM hhyyrylainen/stackwalk-deps:v1 as builder

RUN git clone https://github.com/hhyyrylainen/StackWalkAsAService.git ~/StackWalk && cd ~/StackWalk && git submodule init

RUN cd ~/StackWalk && ./setup.rb --no-packagemanager

RUN cd ~/StackWalk/build && make install

FROM fedora:30

RUN dnf install -y --setopt=deltarpm=false libjpeg zlib boost GraphicsMagick fcgi glew libharu zlib sqlite libpq pango libunwind openssl libpng && dnf clean all

# WORKDIR /stackwalk/
COPY --from=builder /usr/local/bin/stackwalkwebapp /usr/local/bin/
COPY --from=builder ~/StackWalk/build/ThirdParty/bin/minidump_stackwalk /usr/local/bin/
CMD ["/usr/local/bin/stackwalkwebapp"]

