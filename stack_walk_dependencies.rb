require 'os'

require_relative 'RubySetupSystem/Libraries/SetupWt.rb'
require_relative 'RubySetupSystem/Libraries/SetupBreakpad.rb'

# Setup dependencies settings
THIRD_PARTY_INSTALL = File.join(ProjectDir, 'build', 'ThirdParty')

@breakpad = Breakpad.new(
  # version: "abfe08e78927a5cd8c749061561da3c3c516f979",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

@wt = Wt.new(
  version: '4.1.2',
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  noExamples: true,
  shared: OS.windows? ? false : true,
  noQt: true
)

@libs_list = [@breakpad, @wt]
