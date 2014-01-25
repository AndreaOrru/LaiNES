flags = ['-O4', '-march=native', '-std=c++11']

env = Environment(CXX       = 'clang++',
                  CXXFLAGS  = flags,
                  LINKFLAGS = flags,
                  LIBS      = ['SDL2'])

env.Program('laines', Glob('src/*.cpp'))
