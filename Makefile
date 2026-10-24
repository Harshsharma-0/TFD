COMPILER=clang++
IN_FILES=main.cpp
OUT_FILE=tfd
OBJECTS= ./test/test1  ./test/test2  ./test/test3  ./test/test4  ./test/test5  ./test/test6 
TEST_ALL = ${OBJECTS}

all:tfd.hpp
	${COMPILER} ${IN_FILES} -o ${OUT_FILE}

docs:
	doxygen Doxyfile

gentest: $(OBJECTS)

#/* TODO : Make tests more reliable and configurable */
test: ${OBJECTS} tfd.hpp

$(OBJECTS): %: %.cpp
	 ${COMPILER} -o $@.test $<
	 ./$@.test
	 @if [ $$? -ne 0 ]; then \
        echo "TFD Tests failed"; \
        exit 1; \
    else \
        echo "TFD Tests passed"; \
    fi

