cc -c main.c -o main.o
cc -c cJSON.c -o cJSON.o
cc -c terminal_support.c -o terminal.o
cc -o weather_cli main.o cJSON.o terminal.o -lcurl -lm
