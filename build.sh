cc -c main.c -o main.o
cc -c cJSON.c -o cJSON.o
cc -o weather_cli main.o cJSON.o -lcurl -lm
