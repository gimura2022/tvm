BUILT_SOURCES = as_parser.h as_parser.c as_lexer.c
AM_YFLAGS     = -d

lib_LTLIBRARIES   = libtvm.la
libtvm_la_SOURCES = mem.c tvm.c vm.c bytecode.c utils.h
include_HEADERS   = tvm.h

bin_PROGRAMS = tvmi tvm-dis tvm-as

tvmi_SOURCES = tvmi.c
tvmi_LDADD   = libtvm.la

tvm_dis_SOURCES = tvm-dis.c
tvm_dis_LDADD   = libtvm.la

tvm_as_SOURCES = tvm-as.c as_lexer.l as_parser.y
tvm_as_LDADD   = libtvm.la
