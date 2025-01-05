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
  version: '1ab24bcc817ebe629bf77daa53529d02361cb1e9',
  installPath: File.join(THIRD_PARTY_INSTALL, 'breakpad_mingw'),
  noInstallSudo: true
)

@wt = Wt.new(
  version: '4.11.1',
  installPath: THIRD_PARTY_INSTALL,
  noInstallSudo: true,
  noExamples: true,
  shared: OS.windows? ? false : true,
  noQt: true
)

@libs_list = [@breakpad, @breakpad_mingw, @wt]
