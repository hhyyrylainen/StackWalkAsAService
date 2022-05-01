StackWalk as a Service
======================

A basic web service with an API for performing Google Breakpad crash
dump decoding using the StackWalk tool.

Runs in a docker container for maximum portability.

Doesn't handle uploading symbol files. That's left to the user.

Running
-------

Available from docker hub with `docker pull hhyyrylainen/stackwalk:latest`

Running:
```sh
docker run -itd -p 127.0.0.1:9090:9090 -v $(pwd)/build/src/Symbols:/Symbols:ro --restart always --name stackwalkweb hhyyrylainen/stackwalk:latest --http-port 9090
```

The three part port syntax is ensured so that the container is listening only
for local connections. It's not safe to expose this service to the internet
directly so this is a recommended safety precaution.

Before you run the command adjust the path for the mounted symbol
folder (the host path is the first path listed in the `-v` option), as
well as make sure the folder exists on the host and contains the
needed symbol files.

That starts a container that is in the background running the service
and automatically restarting when needed. You can use `docker ps` to
see the running container's name in order to stop it, for example when
upgrading the service version.

### Podman

Alternatively the container can be run with `podman` with a command 
similar to this:
```sh
podman run -d --rm -p 127.0.0.1:9090:9090 --mount type=bind,src=$(pwd)/build/src/Symbols,destination=/Symbols,ro=true,relabel=shared --name stackwalkweb hhyyrylainen/stackwalk:latest --http-port 9090
```

Note that to make it persistent you need to create a systemd service 
(separate account rather than running as root is recommended):
```sh
useradd stackwalk -s /sbin/nologin
podman generate systemd --new --name stackwalkweb > /etc/systemd/system/stackwalkweb.service
# Edit to run as our stackwalk user
# Add `User=stackwalk` to the service section and replace `%t/` with `/home/stackwalk/`
emacs /etc/systemd/system/stackwalkweb.service
podman stop stackwalkweb
systemctl daemon-reload
# You need to run the container at least once as the user you specified in the service file
# the service failing to start is not a problem as long as the container managed to start correctly
su - stackwalk -s /usr/bin/bash
podman run --rm --name stackwalkweb hhyyrylainen/stackwalk:latest
exit
systemctl enable --now stackwalkweb
systemctl status stackwalkweb
```

More info about that can be found elsewhere, for example:
https://www.tutorialworks.com/podman-systemd/

API
---

The API is accessed by adding `/api/v1` to the web server path. The
API allows uploading one crash dump file at a time as form data.

For example using httpie client:
```sh
http -f POST http://localhost:9090/api/v1 file@1df625f6-6e6e-4e0a-58e8e6b0-9ab2b5f0.dmp
```

Change the URL to match where the service is hosted if not running
locally, and replace the file name after the `@` character to the file
you have.

Building
--------

### Docker build

```
git submodule update --init
docker build .
```


### Native build

It's also possible to compile and run this on the host provided all
the needed libraries and tools are installed.

Rough steps:

```sh
gem install os colorize rubyzip json sha3
./setup.rb

```

On OSes other than Ubuntu or Fedora you will want to run the setup
with `--no-packagemanager` parameter.

Note: Windows build might work but is untested.

You can then for example run the compiled program like this:
```sh
build/src/stackwalkwebapp --docroot build/src/ --http-address 0.0.0.0 --http-port 9090 -c config/wt_config.xml
```

Note that you may need to specify a different symbol directory or create
the folder `build/src/Symbols`.

You can check the `Makefile` for example commands how to rebuild and
run after minimal changes.
