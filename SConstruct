from os import environ

flags = ['-O3', '-march=native', '-std=c++11']

env = Environment(ENV       = { 'TERM' : environ['TERM'] },
                  CXX       = 'clang++',
                  CPPPATH   = 'src',
                  CPPFLAGS  = ['-Wno-unused-value', '-Wno-constant-conversion'],
                  CXXFLAGS  = flags,
                  LINKFLAGS = flags,
                  LIBS      = ['SDL2', 'SDL2_image', 'SDL2_ttf'])

env.Program('laines', Glob('src/*.cpp') + Glob('src/*/*.cpp'))
