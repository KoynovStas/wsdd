#ifndef FILE_UTILS_HEADER
#define FILE_UTILS_HEADER


#include <sys/types.h>





off_t file_size(const char *file_name);
off_t ffile_size(int fd);



int set_blocking(int fd, int blocking);

int set_nonblock_mode(int fd);
int set_block_mode(int fd);



int count_files_in_dir(const char *dir_name);



int write_loop(int fd, char *buf, size_t buf_len, int timeout_in_usec, int count_loop);
int read_loop(int fd, char *buf, size_t buf_len, int timeout_in_usec, int count_loop);





#endif //FILE_UTILS_HEADER
