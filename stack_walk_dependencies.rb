require 'os'

require_relative 'RubySetupSystem/Libraries/SetupWt'
require_relative 'RubySetupSystem/Libraries/SetupBreakpad'
require_relative 'RubySetupSystem/Libraries/SetupBreakpadMinGW'

# Setup dependencies settings
THIRD_PARTY_INSTALL = File.join(ProjectDir, 'build', 'ThirdParty')

@breakpad = Breakpad.new(
  # version: "abfe08e78927a5cd8c749061561da3c3c516f979",
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true
)

@breakpad_mingw = BreakpadMinGW.new(
  version: 'cdb9214e3a81223c48a913fd18c05a8d48404620',
  installPath: File.join(THIRD_PARTY_INSTALL, 'breakpad_mingw'),
  noInstallSudo: true
)

@wt = Wt.new(
  version: '4.7.1',
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  noExamples: true,
  shared: OS.windows? ? false : true,
  noQt: true
)

@libs_list = [@breakpad, @breakpad_mingw, @wt]
