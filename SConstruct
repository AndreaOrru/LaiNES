from os import environ

flags = ['-O3', '-march=native', '-std=c++11']

env = Environment(ENV       = { 'TERM' : environ['TERM'] },
                  CXX       = 'clang++',
                  CPPPATH   = 'src',
                  CXXFLAGS  = flags,
                  LINKFLAGS = flags,
                  LIBS      = ['SDL2'])

env.Program('laines', Glob('src/*.cpp'))
