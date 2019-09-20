# -*- mode: python -*-
# -*- coding: utf8 -*-

if Platform() == "win32":
    env = Environment(tools=['mingw'], CPPDEFINES=[('_WIN32_WINNT', '0x0601')], LIBS = ['ws2_32', 'mswsock'])
else:
    env = Environment(LINKFLAGS=['-pthread'])

env.Append(CPPDEFINES = ['ASIO_STANDALONE', 'ASIO_NO_DEPRECATED'])
env.Append(CCFLAGS = ['-Wall', '-Wextra'])
#env.Append(CCFLAGS = ['-O0', '-g'])
env.Append(CCFLAGS = ['-O2'])
env.Append(CXXFLAGS = ['-std=c++17'])

env.Program('tcpforward', ['main.cpp', 'server.cpp', 'session.cpp'])
