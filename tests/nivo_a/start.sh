cd /home/ss/Desktop/ss-projekat-finale
make
cd /home/ss/Desktop/ss-projekat-finale/tests/nivo_a

ASSEMBLER=/home/ss/Desktop/ss-projekat-finale/assembler
LINKER=/home/ss/Desktop/ss-projekat-finale/linker
EMULATOR=/home/ss/Desktop/ss-projekat-finale/emulator


${ASSEMBLER} -o main.o main.s
${ASSEMBLER} -o math.o math.s
${ASSEMBLER} -o ivt.o ivt.s
${ASSEMBLER} -o isr_reset.o isr_reset.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${ASSEMBLER} -o isr_user0.o isr_user0.s
${LINKER} -hex -o program.hex ivt.o math.o main.o isr_reset.o isr_terminal.o isr_timer.o isr_user0.o
${EMULATOR} program.hex
