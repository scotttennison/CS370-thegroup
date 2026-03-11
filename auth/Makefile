# CS370 Auction Site
# Makefile for auth CGI program

CXX = g++

CXXFLAGS = -Wall -I/opt/homebrew/Cellar/mysql/9.6.0_1/include

LDFLAGS = -L/opt/homebrew/Cellar/mysql/9.6.0_1/lib -L/opt/homebrew/lib -lmysqlclient -lz -lzstd -lssl -lcrypto -lresolv

TARGET = auth.cgi

SOURCE = auth.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)