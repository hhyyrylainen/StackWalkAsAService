#!/usr/bin/env ruby
# Generates a new docker file with the dependencies listed in it. Note: it's not the same
# as the Dockerfile in this folder, but it is a useful reference for updating it.
require_relative 'RubySetupSystem/DockerImageCreator.rb'

require_relative 'stack_walk_dependencies.rb'

runDockerCreate(@libs_list, nil)
