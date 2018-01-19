CFLAGS = -Wall -std=gnu99

all: sortSeq sortProcess sortThread

sortSeq: sortSeq.c
	gcc ${CFLAGS} -o sortSeq sortSeq.c

sortProcess: sortProcess.c
	gcc ${CFLAGS} -o sortProcess sortProcess.c

sortThread: sortThread.c
	gcc ${CFLAGS} -o sortThread sortThread.c

clean:
	rm -f sortSeq sortProcess sortThread
