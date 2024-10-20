SOURCES += example.c mallocPlusAI.h
CFLAGS += -Wall -Wpedantic -lcurl -I/usr/include/curl

mallocPlusAI: $(SOURCES)
	clang $(CFLAGS) $(SOURCES)
