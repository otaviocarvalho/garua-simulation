CC = clang
CFLAGS = -lsimgrid

all: simulation

simulation:
	$(CC) $(CFLAGS) simulation.c -o simulation

run:
	./simulation ./hostfile.xml ./platform.xml --cfg=network/latency-factor:1.0 --cfg=network/bandwidth-factor:1.0

clean:
	rm -f *~ *.o simulation
