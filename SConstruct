import sys
from os import environ

VariantDir('build/src', 'src', duplicate=0)
VariantDir('build/lib', 'lib', duplicate=0)

is_windows = sys.platform.startswith('win')

if is_windows:
    cxx       = 'clang-cl'
    cxxflags  = ['/Ox', '/std:c++14']
    cppflags  = []
    linkflags = ['/SUBSYSTEM:CONSOLE']
    defines   = ['SDL_MAIN_HANDLED', '_CRT_SECURE_NO_WARNINGS']
else:
    cxx       = 'clang++'
    cxxflags  = ['-O3', '-march=native', '-std=c++14']
    cppflags  = ['-Wno-unused-value']
    linkflags = cxxflags
    defines   = []

env = Environment(ENV       = environ,
                  CXX       = cxx,
                  CPPFLAGS  = cppflags,
                  CXXFLAGS  = cxxflags,
                  LINKFLAGS = linkflags,
                  CPPPATH   = ['#simpleini', '#lib/include', '#src/include'],
                  CPPDEFINES = defines,
                  LIBS      = ['SDL2', 'SDL2_image', 'SDL2_ttf'])

env.Program('laines', Glob('build/*/*.cpp') + Glob('build/*/*/*.cpp'))
