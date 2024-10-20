SOURCES += example.c mallocPlusAI.h
CFLAGS += -lcurl -I/usr/include/curl

mallocPlusAI: $(SOURCES)
	clang $(CFLAGS) $(SOURCES)
