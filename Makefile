all: jsmn_test

jsmn_test: jsmn_test.c jsmn.c jsmn.h
	gcc  -std=c99 -O0 -g -Wall ${CFLAGS} jsmn_test.c jsmn.c -o jsmn_test
	@echo "valgrind --track-origins=yes  --leak-check=yes ./jsmn_test --show-leak-kinds=all --leak-check=full"

clean:
	rm jsmn_test

.PHONY: clean
