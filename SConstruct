import os

env = Environment(CXX      = 'clang++',
                  CXXFLAGS = '-std=c++1y -stdlib=libc++',
                  LIBS     = 'c++abi',
                  ENV      = {'TERM': os.environ['TERM']})

env.Program('laines', Glob('src/*.cpp'))
