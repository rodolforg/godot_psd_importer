BIN_PATH = demo/addons/psd_animation/bin

all: ${BIN_PATH} ${BIN_PATH}/libpsd.so ${BIN_PATH}/libpsd_importer.so

LIBPSD_PATH=libpsd

INCLUDES=-I godot_headers -I ${LIBPSD_PATH}/include
LIBS=-L ${BIN_PATH} -lpsd

CFLAGS=-std=c11 -fPIC -O2 ${INCLUDES}

src/register_types.o: src/register_types.c
	$(CC) -c -${CFLAGS} ${INCLUDES} $^ -o $@

src/psd_importer.o: src/psd_importer.c
	$(CC) -c ${CFLAGS} ${INCLUDES} $^ -o $@

src/psd_document.o: src/psd_document.c
	$(CC) -c ${CFLAGS} ${INCLUDES} $^ -o $@

${BIN_PATH}/libpsd_importer.so: src/simple_list.o src/register_types.o src/psd_document.o src/psd_layer.o src/psd_importer.o
	$(LD) -shared -rpath=addons/psd_animation/bin ${LIBS} $^ -o $@

${BIN_PATH}/libpsd.so: ${LIBPSD_PATH}/src/*.c
	$(CC) -shared ${CFLAGS} ${INCLUDES} -I ${LIBPSD_PATH}/src $^ -o $@

${BIN_PATH}:
	mkdir -f $@

clean:
	rm -f src/*.o
	rm -f ${BIN_PATH}/*.so
	rm -f ${BIN_PATH}/*.dll
