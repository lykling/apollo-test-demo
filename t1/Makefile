t1: t1.o t1.pb.o
	g++ -o $@ $^ -lprotobuf

%.pb.cc %.pb.h: %.proto
	protoc --cpp_out=. $<

%.o: %.cc
	g++ -c -o $@ $<

t1.o: t1.pb.o

clean:
	rm -f t1 t1.o t1.pb.o t1.pb.h t1.pb.cc

.PHONY: clean
