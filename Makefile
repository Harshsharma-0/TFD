COMPILER=clang++
IN_FILES=main.cpp
OUT_FILE=tfd

all:
	${COMPILER} ${IN_FILES} -o ${OUT_FILE}

docs:
	doxygen Doxyfile
