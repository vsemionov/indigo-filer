
# set these to values appropriate for your system
POCO_INCLUDE = /c/Users/Victor/Documents/work/libs/poco/include
POCO_LIB = /c/Users/Victor/Documents/work/libs/poco/lib

DEBUG = 
OPTIMIZATION = -O2

CXXFLAGS = -I $(POCO_INCLUDE) $(DEBUG) $(OPTIMIZATION)
LDFLAGS = -L $(POCO_LIB)

WINDOWS_LIBS = -lwsock32 -liphlpapi

# remove $(WINDOWS_LIBS) on Unix
LDLIBS = -lPocoUtil -lPocoNet -lPocoXML -lPocoFoundation $(WINDOWS_LIBS)

.PHONY: all clean

all: clean
	mkdir build
	g++ $(CXXFLAGS) $(LDFLAGS) -o build/indigo-filer src/*.cpp $(LDLIBS)

clean:
	rm -rf build
