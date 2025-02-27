# 编译器和编译选项
CC = gcc
CFLAGS = -I/home/txj/remote_file/linux_driver/proj_file/include -Wall -g -O0

# 链接选项和库路径
#LDFLAGS = -L/home/txj/remote_file/linux_driver/proj_file/lib
LDFLAGS = -L/home/txj/remote_file/linux_driver/proj_file/lib -Wl,-rpath,/home/txj/remote_file/linux_driver/proj_file/lib 
LIBS = -llinksdk -lmysqlclient -lpthread

# 源文件和目标文件
SRCS = rk3568_server_v0.2.c connect_mysql.c threadpool.c
OBJS = $(SRCS:.c=.o)
TARGET = rk3568_server_v0.2

# 默认目标
all: $(TARGET)

# 编译规则
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

# 生成依赖关系的规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)

#超声监控，喇叭状态，led状态，门状态，风扇状态
