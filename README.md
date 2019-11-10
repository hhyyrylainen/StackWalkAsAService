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


First make sure that the directory you use in the symbol command
contains the symbol files as subdirectories, otherwise StackWalk won't
find them.

API
---



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
./Setup.rb

```

On OSes other than Ubuntu or Fedora you will want to run the setup
with `--no-packagemanager` parameter.

Note: Windows build might work but is untested.
