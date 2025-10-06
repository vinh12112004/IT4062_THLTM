# Makefile cho DNS Resolver

# Compiler và flags
CC = gcc
CFLAGS = -Wall -D_WIN32_WINNT=0x0600
LDFLAGS = -lws2_32

# Tên file
TARGET = resolver.exe
SOURCE = resolver.c
OBJECT = resolver.o

# Log file
LOG_FILE = resolver_log.txt

# Quy tắc mặc định
all: $(TARGET)

# Biên dịch chương trình
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Executable: $(TARGET)"

# Biên dịch với object file (tùy chọn)
object: $(OBJECT)
	$(CC) $(OBJECT) -o $(TARGET) $(LDFLAGS)
	@echo "Build from object file successful!"

$(OBJECT): $(SOURCE)
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)

# Chạy chương trình ở chế độ tương tác
run: $(TARGET)
	./$(TARGET)

# Chạy với file test (tạo file test.txt nếu chưa có)
test: $(TARGET)
	@if not exist test.txt (echo google.com > test.txt && echo 8.8.8.8 >> test.txt && echo facebook.com youtube.com >> test.txt)
	./$(TARGET) test.txt

# Dọn dẹp các file tạm
clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist $(OBJECT) del $(OBJECT)
	@echo "Cleaned build files"

# Dọn dẹp toàn bộ (bao gồm cả log)
cleanall: clean
	@if exist $(LOG_FILE) del $(LOG_FILE)
	@if exist test.txt del test.txt
	@echo "Cleaned all files"

# Xem log
log:
	@if exist $(LOG_FILE) (type $(LOG_FILE)) else (echo No log file found)

# Rebuild: clean và build lại
rebuild: clean all

# Hiển thị thông tin
help:
	@echo ========================================
	@echo DNS Resolver - Makefile Commands
	@echo ========================================
	@echo make          - Build the program
	@echo make run      - Build and run interactive mode
	@echo make test     - Build and run with test.txt
	@echo make clean    - Remove build files
	@echo make cleanall - Remove all files (including log)
	@echo make log      - Display log file
	@echo make rebuild  - Clean and rebuild
	@echo make help     - Show this help
	@echo ========================================

.PHONY: all run test clean cleanall log rebuild help object