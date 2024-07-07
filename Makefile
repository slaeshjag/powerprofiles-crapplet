CFLAGS += `pkg-config --cflags gtk+-3.0` -Wall -O0 -g
LDFLAGS += `pkg-config --libs gtk+-3.0` 

default:
	$(CC)  powerprofiles-crapplet.c -o powerprofiles-crapplet $(CFLAGS) $(LDFLAGS)

clean:
	rm -f powerprofiles-crapplet
