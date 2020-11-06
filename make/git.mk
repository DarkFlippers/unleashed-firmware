GIT_COMMIT		= $(shell git describe --always  --exclude '*' || echo 'unknown')
GIT_BRANCH		= $(shell git rev-parse --abbrev-ref HEAD || echo 'unknown')
GIT_BRANCH_NUM	= $(shell git rev-list --count $(GIT_BRANCH) || echo 'nan')
BUILD_DATE		= $(shell date '+%d-%m-%Y' || echo 'unknown')

CFLAGS			+= -DGIT_COMMIT="\"$(GIT_COMMIT)\"" -DGIT_BRANCH="\"$(GIT_BRANCH)\"" -DGIT_BRANCH_NUM="\"$(GIT_BRANCH_NUM)\""
CFLAGS			+= -DBUILD_DATE="\"$(BUILD_DATE)\"" -DTARGET="\"$(TARGET)\""