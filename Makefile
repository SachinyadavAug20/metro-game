all:
	$(MAKE) -C raylib all

web:
	$(MAKE) -C raylib web

clean:
	$(MAKE) -C raylib clean

run:
	$(MAKE) -C raylib run

.PHONY: all web clean run
