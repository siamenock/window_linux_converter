CC = gcc
SRC_PATH = src
BUILD_PATH = build
OUTPUT_PATH = output

.PHONY: all

all : ecdh rsa

clean:
	rm $(BUILD_PATH)/ecdh $(BUILD_PATH)/rsa $(OUTPUT_PATH)/'RSA Result.txt'

ecdh: 
	$(CC) $(SRC_PATH)/big_integer.c $(SRC_PATH)/ecdh.c -o $(BUILD_PATH)/ecdh

rsa: 
	$(CC) $(SRC_PATH)/big_integer.c $(SRC_PATH)/rsa.c -o $(BUILD_PATH)/rsa

run_ecdh: ecdh
	./$(BUILD_PATH)/ecdh

run_rsa: rsa
	./$(BUILD_PATH)/rsa


