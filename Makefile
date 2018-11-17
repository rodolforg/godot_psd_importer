all: demo/addons/psd_animation/bin demo/addons/psd_animation/bin/libpsd.so demo/addons/psd_animation/bin/libpsdump.so demo/addons/psd_animation/bin/libpsd_importer.so

PSDDUMP_PATH=psdump
LIBPSD_PATH=${PSDDUMP_PATH}/libpsd-0.9

CFLAGS=-std=c11 -fPIC -O2
CXXFLAGS=-std=c++11 -fPIC -O2

INCLUDES=-I godot_headers -I ${LIBPSD_PATH}/include
LIBS=-L demo/addons/psd_animation/bin -lpsd -lpsdump

src/register_types.o: src/register_types.c
	$(CC) -c -${CFLAGS} ${INCLUDES} $^ -o $@

src/psd_importer.o: src/psd_importer.c
	$(CC) -c ${CFLAGS} ${INCLUDES} $^ -o $@

src/psd_parser.o: src/psd_parser.cpp
	$(CXX) -c ${CXXFLAGS} ${INCLUDES} -I ${PSDDUMP_PATH}/src $^ -o $@

demo/addons/psd_animation/bin/libpsd_importer.so: src/register_types.o src/psd_importer.o src/psd_parser.o
	$(LD) -shared -rpath=addons/psd_animation/bin ${LIBS} $^ -o $@

demo/addons/psd_animation/bin/libpsd.so: ${LIBPSD_PATH}/src/*.c
	$(CC) -shared ${CFLAGS} ${INCLUDES} -I ${LIBPSD_PATH}/src $^ -o $@

demo/addons/psd_animation/bin/libpsdump.so: ${PSDDUMP_PATH}/src/Record.cpp ${PSDDUMP_PATH}/src/Layer.cpp ${PSDDUMP_PATH}/src/LayerGroup.cpp ${PSDDUMP_PATH}/src/Document.cpp ${PSDDUMP_PATH}/src/build_path.cpp ${PSDDUMP_PATH}/src/parser/PsdParser.cpp  ${PSDDUMP_PATH}/src/lodepng/lodepng.cpp
	$(CXX) -shared ${CXXFLAGS} ${INCLUDES} -I ${LIBPSD_PATH}/src $^ -o $@

demo/addons/psd_animation/bin:
	mkdir -f $@

clean:
	rm -f src/*.o
	rm -f ${BIN_PATH}/*.so
	rm -f ${BIN_PATH}/*.dll
