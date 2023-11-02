make: emulator linker assembler

emulator: src/cpu.cpp src/interpreter.cpp src/terminal.cpp
	g++ -o $(@) $(^)

linker: src/linker.cpp
	g++ -o $(@) $(^)

assembler: src/parser.cpp src/assembler.cpp src/flex.cpp
	g++ -o $(@) $(^)

src/flex.cpp: misc/flex.l
	flex --outfile=$(@) $(^)

src/parser.cpp: misc/parser.y
	bison -v --defines=inc/parser.hpp --output=$(@) $(^)