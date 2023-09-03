#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/stat.h>

#include "file_utils.h"





/*
 * des: Get size of a file in bytes.
 *
 * in:  file_name  - The full path to file
 *
 * ret:  0 - success
 *      -1 - failure (see errno) off_t is a signed type so this is possible.
 */
off_t file_size(const char *file_name)
{
    struct stat st;

    if(stat(file_name, &st) == 0)
        return st.st_size;


    return -1; //error Can't get info for file (see errno)
}



/*
 * des: Get size of a file in bytes.
 *
 * in:  fd - The file descriptor
 *
 * ret:  0 - success
 *      -1 - failure (see errno) off_t is a signed type so this is possible.
 */
off_t ffile_size(int fd)
{
    struct stat st;

    if(fstat(fd, &st) == 0)
        return st.st_size;


    return -1; //error Can't get info for file (see errno)
}



/*
 * des: Set a file descriptor to blocking or non-blocking mode.
 *
 * in:  fd        - The file descriptor
 *      blocking  - 0 set non-blocking mode for fd
 *                - 1 set blocking mode for fd
 *
 * ret:  0 - success
 *      -1 - failure (see errno)
 */
int set_blocking(int fd, int blocking)
{
    // Save the current flags
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        return -1;


    if(blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;


    return fcntl(fd, F_SETFL, flags);
}



int set_nonblock_mode(int fd)
{
    return set_blocking(fd, 0);
}



int set_block_mode(int fd)
{
    return set_blocking(fd, 1);
}



/*
 * des:  function returns the number of files in a directory
 *
 * in:   const char *dir_name - the full path to the directory
 *
 * ret:  >= 0 - valid value
 *         -1 - failure (see errno)
 */
int count_files_in_dir(const char *dir_name)
{

    DIR *dir;
    struct dirent *entry;
    int count = 0;


    if( !dir_name )
    {
        errno = EINVAL;
        return -1;
    }


    dir = opendir(dir_name);
    if(dir == NULL)
        return -1;


    while( (entry = readdir(dir)) )
        count++;


    closedir(dir);
    return count;
}



/*
 * des:  The write_loop() function shall attempt to write buf_len bytes from the buffer
 *       pointed to by buf to the file associated with the open file descriptor, fd.
 *
 * in:   fd              - file descriptor
 *       buf             - pointer to data buffer
 *       buf_len         - count bytes for writing
 *       timeout_in_usec - timeout between cycles writing
 *       count_loop      - number of attempts to writing
 *
 * ret:  >= 0 - the number of bytes actually written to the file associated with fd
 *         -1 - failure (see errno)
 */
int write_loop(int fd, char *buf, size_t buf_len, int timeout_in_usec, int count_loop)
{

    int   n, len;


    if( !buf || !buf_len )
    {
        errno = EINVAL;
        return -1;
    }


    len = buf_len;

    while( (len > 0) && (count_loop-- > 0) )
    {

        n = write(fd, buf, len);

        if( (n == -1) && (errno != EAGAIN) )
            return -1;                       // error writing


        if( n > 0 )
        {
            buf += n;
            len -= n;
        }


        if(len <= 0)
            break;


        usleep(timeout_in_usec);
    }


    return (buf_len - len);  //return length of data writing
}



/*
 * des:  The read_loop() function shall attempt to read buf_len bytes from
 *       the file associated with the open file descriptor fd,
 *       into the buffer pointed to by buf.
 *
 * in:   fd              - file descriptor
 *       buf             - pointer to data buffer
 *       buf_len         - count bytes for reading
 *       timeout_in_usec - timeout between cycles reading
 *       count_loop      - number of attempts to reading
 *
 * ret:  >= 0 - number of bytes actually read from the file associated with fd
 *         -1 - failure (see errno)
 */
int read_loop(int fd, char *buf, size_t buf_len, int timeout_in_usec, int count_loop)
{

    int   n, len;


    if( !buf || !buf_len )
    {
        errno = EINVAL;
        return -1;
    }


    len = buf_len;

    while( (len > 0) && (count_loop-- > 0) )
    {

        n = read(fd, buf, len);


        if( (n == -1) && (errno != EAGAIN) )
            return -1;                       // error reading


        if( n > 0 )
        {
            buf += n;
            len -= n;
        }


        if(len <= 0)
            break;


        usleep(timeout_in_usec);
    }


    return (buf_len - len);  //return length of data reading
}
