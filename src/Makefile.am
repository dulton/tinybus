AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libtinybus.a
libtinybus_a_SOURCES=asyncqueue.c queue.c tinybus.c trace.c
#libtinybus_a_LIBADD=libpthread.a
libtinybus_a_LIBFLAGS=-shared
libtinybus_a_CFLAGS=-fPIC
libtinybus_a_CXXFLAGS=-fPIC

tinybusincludedir=$(includedir)
tinybusinclude_HEADERS=$(top_srcdir)/src/*.h 
