BUILD="`pwd`/common"

all: gluv

gluv: gluv.gyp
	deps/run_gyp gluv.gyp -Goutput_dir=out -Iconfig.gypi --depth=. --generator-output=build -f make
	make -C build
	build/out/Default/gluv

clean:
	rm -rf build

.PHONY:
