MYSQL_CONCPP_DIR = /usr/local/mysql-connector-c++-8.0.24
SOURCE_DIR = src
INCLUDE_DIR = $(SOURCE_DIR)/include
CRYPTO_DIR = crypto
TEST_DIR = test
OBJECT_DIR = obj
PARSER_DIR = parser

SOURCE_FILE = $(wildcard $(SOURCE_DIR)/*.cc) $(wildcard $(CRYPTO_DIR)/sm4.cc)
OBJECTS = $(patsubst %.o, $(OBJECT_DIR)/%.o, $(notdir $(patsubst %.cc, %.o, $(SOURCE_FILE))))
TEST_FILE = $(wildcard $(TEST_DIR)/*.cc)
TEST_OBJ = $(patsubst %.o, $(OBJECT_DIR)/%.o, $(notdir $(patsubst %.cc, %.o, $(TEST_FILE))))
OUTNAME = $(OBJECT_DIR)/proj

# Designate manually.
CXX = clang++
CC = gcc
INCLUDES = -I $(MYSQL_CONCPP_DIR)/include -I $(INCLUDE_DIR) -I $(PARSER_DIR) -I $(CRYPTO_DIR) -I $(TEST_DIR)
LIBNAMES = -lmysqlcppconn
LDLIBS = -L $(MYSQL_CONCPP_DIR)/lib64 $(LIBNAMES)
CXXFLAGS = -std=c++17 -fPIC -Wno-return-type
BUILD = -c $^

path:
	mkdir -p $(OBJECT_DIR)

$(OBJECTS): $(SOURCE_FILE)
	@echo %.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(BUILD)
	mv *.o $(OBJECT_DIR)

build: path $(OBJECTS)

test: path $(OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(TEST_FILE)
	mv *.o $(OBJECT_DIR)
	$(CXX) $(LDLIBS) -o $(OUTNAME) $(TEST_OBJ) $(OBJECTS)
	$(OBJECT_DIR)/proj

.PHONY: encrypt
encrypt: test

.PHONY: clean  
clean:
	rm -rf $(OBJECT_DIR)