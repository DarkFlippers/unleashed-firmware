#!/bin/bash

[ -z $1 ] && {
	echo "Specify an image"
	echo "gimp -> export -> c source file -> [x] gunit names"
	exit 2
}

echo $*

for N in $* ; do

	[ ! -f $N ] && {
		echo "!! File missing $N"
		continue
	}

	# filename (sans extension)
	FN=$(basename -- "$N")
	EXT="${FN##*.}"
	NAME="${FN%.*}"

	OUTDIR=img_/
	mkdir -p ${OUTDIR}

	HDR=${OUTDIR}/images.h
	SRC=${OUTDIR}/images.c

	OUT=${OUTDIR}/img_${NAME}.c

	echo -e "\n¦${N}¦  ==  ¦${NAME}¦  ->  ¦${OUT}¦"

	TESTX=test_${NAME}
	TESTC=test_${NAME}.c

	# compile name
	CONV=${NAME}_

	# clean up gimp output
	sed -e "s/gimp_image/img/g" \
		-e 's/guint8/unsigned char/g' \
		-e 's/width/w/g' \
		-e 's/height/h/g' \
		-e 's/bytes_per_pixel/bpp/g' \
		-e 's/pixel_data/b/g' \
		-e 's/guint/unsigned int/g' \
		$N \
		| grep -v ^/ \
		| grep -v ^$ \
		> ${CONV}.c

	# append conversion code
	cat _convert.c >> ${CONV}.c

	# compile & run converter
	rm -f ${CONV}
	gcc ${CONV}.c -DIMGTEST -o ${CONV}
	./${CONV} ${NAME} ${OUT}
	rm -f ${CONV} ${CONV}.c

	# (create &) update header
	[[ ! -f ${HDR} ]] && cp _convert_images.h ${HDR}
	sed -i "/ img_${NAME};/d" ${HDR}
	sed -i "s#//\[TAG\]#//\[TAG\]\nextern  const image_t  img_${NAME};#" ${HDR}

	# sample FZ code
	[[ ! -f images.c ]] && cp _convert_images.c ${SRC}

	# test
	ROOT=${PWD}
	pushd ${OUTDIR} >/dev/null
	sed  "s/zzz/${NAME}/" ${ROOT}/_convert_test.c  > ${TESTC}
	rm -f ${TESTX}
	gcc  ${TESTC}  ${OUT##*/}  -DIMGTEST  -o ${TESTX}
	./${TESTX}
	rm -f ${TESTX} ${TESTC}
	popd >/dev/null

done
