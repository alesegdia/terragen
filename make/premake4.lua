solution "GenTerreno"
  configurations { "Internal" }
  platforms      { "x64", "x32" }

configuration { "x64" }
  targetdir "../bin64"
  libdirs  { "../bin64" }
  flags { "Symbols" }

configuration { "x32" }
  targetdir "../bin32"
  libdirs  { "../bin32" }
  flags { "Symbols" }

project "mtrand"
  language "C++"
  kind     "StaticLib"

  files    { "../libmt/libmt.cpp" }


project "Terrenos"
  language "C++"
  kind     "ConsoleApp"

  targetname("terrenos");

  files { "../terrenos/terrenos.cpp" }
  links { "noise"  }

  buildoptions { "-Wno-write-strings", "-ffast-math", "-fno-rtti", "-std=c++11" }	--caca libnoise no deja poner "-fno-exceptions"
  linkoptions { "`pkg-config --libs opencv`" }
