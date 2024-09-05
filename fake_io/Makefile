CC=gcc
CFLAGS=-I. -g -fPIC -pthread
CXX=g++
CXXFLAGS=-I. -g -fPIC -pthread -std=c++17
LD=ld
LDFLAGS += \
					 -fPIC \
					 -Wl,-no-as-needed \
					 -Wl,-rpath=. \
					 -ldl

all: libfake_io.so

libfake_io.so: fake_io.o
	$(CC) -shared -o $@ ${CFLAGS} ${LDFLAGS} $^

t: t.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

%: %.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf *.o *.pb.o
	-rm -rf *.pb.h *.pb.cc
	-rm -rf *.so
	-rm -rf t

.PHONY: clean
