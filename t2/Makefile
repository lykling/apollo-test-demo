CXX=g++
CXXFLAGS=-I. -g -fPIC -pthread -std=c++17
LD=ld
LDFLAGS += \
					 -fPIC \
					 -pthread \
					 -Wl,-no-as-needed \
					 -Wl,-rpath=. \
					 -ldl

all: shm_pthread_rwlock_w shm_pthread_rwlock_r

shm_pthread_rwlock_w: shm_pthread_rwlock_w.o libcommon.so

shm_pthread_rwlock_r: shm_pthread_rwlock_r.o libcommon.so

lib%.so: %.o
	$(CXX) -shared -o $@ ${CXXFLAGS} ${LDFLAGS} $^

%: %.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf *.o
	-rm -rf libcommon.so
	-rm -rf shm_pthread_rwlock_w shm_pthread_rwlock_r

.PHONY: clean
