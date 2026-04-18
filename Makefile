# 컴파일러와 옵션
CC = g++
CFLAGS = -Wall -g -MMD -MP
LDLIBS = -lncurses

# 최종 실행파일 이름
TARGET = game

# 개별 오브젝트 파일들
OBJS = main.o Server.o Support.o UI.o Character.o gost.o
DEPS = $(OBJS:.o=.d)

# 1. 최종 타겟
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDLIBS)

# 2. 개별 오브젝트 생성 규칙
main.o: main.cpp UI.h
	$(CC) $(CFLAGS) -c $< -o $@

UI.o: UI.cpp UI.h
	$(CC) $(CFLAGS) -c $< -o $@

Support.o: Support.cpp Support.h
	$(CC) $(CFLAGS) -c $< -o $@

Server.o: Server.cpp Server.h
	$(CC) $(CFLAGS) -c $< -o $@

Character.o: Character.cpp Character.h
	$(CC) $(CFLAGS) -c $< -o $@
	
gost.o: gost.cpp gost.h
	$(CC) $(CFLAGS) -c $< -o $@

# 3. 청소용
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)

