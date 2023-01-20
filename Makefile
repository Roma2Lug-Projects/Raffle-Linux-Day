all:
	gcc estrazione.c -Wall -Wextra -pedantic -o estrazione.elf
	gcc editor.c -Wall -Wextra -pedantic -o editor.elf

clean:
	rm -rf *.elf
