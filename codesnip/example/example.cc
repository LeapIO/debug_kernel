/**
 * @file example.cc
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-02-28
 * @copyright Copyright double_D
 * 
 */
#include "example.h"

int main(int argc, char *argv[]){
	std::cout << round_down(6, 4) << std::endl;
	
	// int fd;
	// uint64_t *dst_mem_base_addr;
	// char buff[1024]="double_D's test example!\n";
	// fd = open("/dev/dax0.0", O_RDWR);  // 如果不是创建新文件，即使设置mode也会被忽略掉
	// if (fd < 0) {
	// 	perror("cannot open nvdimm device");
	// 	exit(errno);
	// }
	// dst_mem_base_addr = (uint64_t *)mmap(NULL, 1024*1024*1024, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE, fd, 0);
	// /* do mmap io for nvdimm */
	// memmove(dst_mem_base_addr, buff, 100);
	// close(fd);
	// printf("write finish\n");
	return 0;
}

// int main(int argc, char *argv[])
// {
// 	printf("it is %d\n", OOO);
// 	int fd;
// 	int res;
// 	char buf[] = "double_D learn driver!";
// 	char buf2[100];
// 	char buf3[100];
// 	fd = open("/dev/myep", O_RDWR);
// 	if (fd < 0) {
// 		printf("can't open\n");
// 	}

// 	res = write(fd, buf, 15);
// 	printf("write : %d\n", res);

// 	lseek(fd, 0, SEEK_SET);

// 	res = read(fd, buf2, 15);
// 	buf2[15] = '\0';
// 	printf("read: %d\n", res);
// 	printf("buf2 = %s\n", buf2);

// 	close(fd);
// 	fd = open("/dev/myep", O_RDWR);
// 	if (fd < 0) {
// 		printf("can't open\n");
// 	}

// 	res = read(fd, buf3, 15);
// 	buf3[15] = '\0';
// 	printf("read : %d\n", res);
// 	printf("buf3 = %s\n", buf3);
	
// 	close(fd);
// 	return 0;
// }