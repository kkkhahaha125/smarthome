#编译器
CC = aarch64-linux-gnu-gcc

#所有源文件路径
SRC = $(shell find src -name "*.c")

#所有头文件的路径
INC := ./inc \
	./3rd/usr/local/include \
	./3rd/usr/include \
	./3rd/usr/include/aarch64-linux-gnu \
	./3rd/usr/include/aarch64-linux-gnu/python3.10 \
	./3rd/usr/include/python3.10 \
	./3rd/usr/include/python3.10/cpython \
	./3rd/usr/include/python3.10/internal

#所有动态库/静态库的路径
LIBS_PATH = ./3rd/usr/local/lib \
			./3rd/lib/aarch64-linux-gnu \
			./3rd/usr/lib/aarch64-linux-gnu \
			./3rd/usr/lib/python3.10/config-3.10-aarch64-linux-gnu
#所有目标文件路径
OBJ := $(subst src/,obj/,$(SRC:.c=.o))

TARGET = obj/smarthome

#所有头文件路径面前加上-I
CFLAGS := $(foreach item,$(INC),-I$(item))

#所有库文件路径面前加上-L
LDFLAGS := $(foreach item,$(LIBS_PATH),-L$(item)) \
			-Wl,-rpath-link=./3rd/lib/aarch64-linux-gnu

#指定要链接的动态库
LIBS := -lwiringPi -lpython3.10 -lpthread 

#生成目标文件的规则
obj/%.o : src/%.c
	mkdir -p obj
	$(CC) -o $@ -c $< $(CFLAGS)

#链接所有目标文件和库文件
$(TARGET) : $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

compile : $(TARGET)

clean :
	rm $(TARGET) obj -rf


debug :
	echo $(CC)
	echo $(SRC)
	echo $(INC)
	echo $(LIBS_PATH)
	echo $(OBJ)
	echo $(TARGET)
	echo $(CFLAGS)
	echo $(LDFLAGS)
	echo $(LIBS)





.PHONY : compile clean debug

