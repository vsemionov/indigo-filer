
POCO_INCLUDE = /c/Users/Victor/Documents/work/libs/poco/include
POCO_LIB = /c/Users/Victor/Documents/work/libs/poco/lib

CXXFLAGS = -I $(POCO_INCLUDE)
LDFLAGS = -L $(POCO_LIB)

WINDOWS_LIBS = -lwsock32 -liphlpapi
LDLIBS = -lPocoUtil -lPocoNet -lPocoXML -lPocoFoundation $(WINDOWS_LIBS)

.PHONY: all clean

all: clean
	mkdir build
	g++ $(CXXFLAGS) $(LDFLAGS) -o build/indigo-filer src/*.cpp $(LDLIBS)

clean:
	rm -rf build
