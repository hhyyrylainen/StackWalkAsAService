#!/usr/bin/env ruby

# Setup script for Stackwalk
# NOTE: sudo is required on linux (unless skipping package manager installs)
require 'English'

# RubySetupSystem Bootstrap
if !File.exist? 'RubySetupSystem/RubySetupSystem.rb'
  puts 'Initializing RubySetupSystem'
  system 'git submodule init && git submodule update'

  if $CHILD_STATUS.exitstatus != 0
    abort('Failed to initialize or update git submodules. ' \
          'Please make sure git is in path and ' \
          'you have an ssh key setup for your github account')
  end
else
  # Make sure RubySetupSystem is up to date
  # This may make debugging RubySetupSystem harder so feel free to comment out
  system 'git submodule update'
end

require 'fileutils'

require_relative 'RubySetupSystem/RubyCommon.rb'

def checkRunFolder(suggested)
  random_file = File.join(suggested, 'setup.rb')

  onError('Not ran from base directory!') unless File.exist?(random_file)

  thirdPartyFolder = File.join suggested, 'ThirdParty'

  FileUtils.mkdir_p thirdPartyFolder
  FileUtils.mkdir_p File.join suggested, 'build', 'ThirdParty'

  thirdPartyFolder
end

def projectFolder(baseDir)
  File.expand_path File.join(baseDir, '../')
end

require_relative 'RubySetupSystem/RubySetupSystem.rb'

require_relative 'stack_walk_dependencies.rb'

# All the objects
installer = Installer.new(
  @libs_list
)

installer.run

onError "'StackWalk' folder is missing" unless File.exist? ProjectDir

Dir.chdir(ProjectDir) do
  FileUtils.mkdir_p 'build'
end

success 'StackWalk folder and assets are good to go'

info 'Compiling StackWalk'

# Build directory is made earlier
Dir.chdir(File.join(ProjectDir, 'build')) do
  onError 'Failed to configure main project' unless runCMakeConfigure []

  onError 'Failed to compile main project' unless TC.runCompiler
end

success 'Done compiling StackWalk'

success 'All done.'

if OS.linux?

  info "To compile again just run 'make' in ./build"
  puts 'You may need to run this setup again from time to time'

else

  info 'Open build/StackWalkAsAService.sln and start coding'

end

exit 0
