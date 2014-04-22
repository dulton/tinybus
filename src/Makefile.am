AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libtinybus.a
libtinybus_a_SOURCES=asyncqueue.c queue.c tinybus.c slot.c trace.c threadpool.c
libtinybus_a_LIBADD=
libtinybus_a_LIBFLAGS=-shared
libtinybus_a_CFLAGS=-g -fPIC -Wall -D_GNU_SOURCE -D_THREAD_POOL_DEBUG_
libtinybus_a_CXXFLAGS=-g -fPIC -Wall -D_GNU_SOURCE -D_THREAD_POOL_DEBUG_

tinybusincludedir=$(includedir)
tinybusinclude_HEADERS=$(top_srcdir)/src/*.h
