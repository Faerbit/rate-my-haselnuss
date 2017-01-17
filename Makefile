all: run

.PHONY: build
build:
	mkdir -p build
	cd build; cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..; ninja

run: build
	./build/haselnuss

clean:
	rm -rf build
