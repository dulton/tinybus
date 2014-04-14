#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "trace.h"

#define TRACE_MAX_BUF  (128 * 1024)
#define MAX_PRINT_SIZE 1024

static uint32_t
async_ring_buf_get_used_space(async_ring_buf_t *buf)
{
  int datasize = 0;
  
  /*
   * 获取数据长度，read_pos == write_pos 时，datasize = 0， 数据为空;
   * read_pos < write_pos时，datasize = write_pos - read_pos;
   * read_pos > write_pos时，datasize = size + write_pos -read_pos;
   */
  if (buf->read_pos > buf->write_pos)
    datasize = buf->size - buf->read_pos + buf->write_pos;
  else
    datasize = buf->write_pos - buf->read_pos;

  return datasize;    
}

static uint32_t
async_ring_buf_get_free_space(async_ring_buf_t *buf)
{
    uint32_t space;

    /*
     * 获取可用空间长度，read_pos == write_pos时，space = size, 空间未用；
     * read_pos < write_pos时，space = size - write_pos + read_pos;
     * read_pos > write_pos时，space = read_pos - write_pos;
     */
    pthread_mutex_lock(&buf->mutex);
    if (buf->read_pos > buf->write_pos)
        space = buf->read_pos - buf->write_pos;
    else
        space = buf->size - buf->write_pos + buf->read_pos;
    pthread_mutex_unlock(&buf->mutex);

    return space;
}


static void *trace_thread_worker(void *self)
{
    int     result, readbytes;
    char    string[MAX_PRINT_SIZE];
    async_ring_buf_t *buf = (async_ring_buf_t *)self;

    for (;;)
    {
        readbytes = 0; // initiate every time
        
        pthread_mutex_lock(&buf->mutex);
        result = pthread_cond_wait(&buf->cond, &buf->mutex);
        if (!result)
        {
            readbytes = async_ring_buf_get_used_space(buf);
            if (readbytes > MAX_PRINT_SIZE - 1)
                readbytes = MAX_PRINT_SIZE - 1;
            
            if ((buf->read_pos + readbytes) <= buf->write_pos)
                strncpy(string, buf->start + buf->read_pos, readbytes);
            else
            {
                strncpy(string, buf->start + buf->read_pos, buf->size - buf->read_pos);
                strncpy(string + buf->size - buf->read_pos, buf->start, readbytes - buf->size + buf->read_pos);                
            }

            buf->read_pos = (buf->read_pos + readbytes) % buf->size;
        }                    
        pthread_mutex_unlock(&buf->mutex);
        
        if (readbytes > 0)
        {
            string[readbytes] = '\0'; // string terminated with NULL
            fprintf(stderr, "%s", string);
        }
        
        atomic_get(&buf->exit);
        if (buf->exit)
            break;
    }

    return NULL;
}

async_ring_buf_t*
async_ring_buf_new(uint32_t size)
{
    async_ring_buf_t *buf;
    int result;
    
    buf = (async_ring_buf_t *)calloc(1, sizeof(async_ring_buf_t));
    if (buf == NULL)
        return NULL;

    buf->start = (char *)calloc(size, sizeof(char));
    if (buf->start == NULL)
    {
        free(buf);
        return NULL;
    }

    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->cond, NULL);
    result = pthread_create(&buf->thread, NULL, trace_thread_worker, buf);
    if (result)
    {
        free(buf->start);
        free(buf);
        return NULL;
    }

    atomic_set(&buf->level, TRACE_DETAIL_LEVEL);
    
    return buf;
}

void
async_ring_buf_free(async_ring_buf_t *buf)
{
    assert(buf);

    atomic_set(&buf->exit, 1);
    pthread_join(buf->thread, NULL);
    
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->cond);

    if (buf->start)
        free(buf->start);

    free(buf);
}

uint32_t
async_ring_buf_write(async_ring_buf_t *buf, char *string, size_t len)
{
    uint32_t free_size, writable_size;

    assert(buf);
    free_size = async_print_get_free_space(buf);

    writable_size = (free_size < len) ? free_size : len;
    if (writable_size == 0)
        return 0;

    pthread_mutex_lock(&buf->mutex);

    if(buf->write_pos + writable_size > buf->size)
    {
      memcpy(buf->start + buf->write_pos, string, buf->size - buf->write_pos);
      memcpy(buf->start, string + buf->size - buf->write_pos, 
        writable_size - (buf->size - buf->write_pos));
    }
    else
    {
      memcpy(buf->start + buf->write_pos, string, writable_size);
    }
    
    buf->write_pos = (buf->write_pos + writable_size) % buf->size;

    pthread_cond_signal(&buf->cond);
    pthread_mutex_unlock(&buf->mutex);

    return writable_size;
}

//===========================================================================

async_ring_buf_t *output;
static char *get_level_str(uint32_t level)
{
    static char *info[5] = {
            "DETAIL: ",
            "DEBUG: ",
            "TRACE: ",
            "WARNING: ",
            "ERROR: "};

    switch(level)
    {
        case TRACE_DETAIL_LEVEL:
            return info[0];            
        case TRACE_DEBUG_LEVEL:
            return info[1];
        case TRACE_TRACE_LEVEL:
            return info[2];
        case TRACE_WARNING_LEVEL:
            return info[3];
        case TRACE_ERROR_LEVEL:
            return info[4];
        default:
            return NULL;
    }

    return NULL;        
}

static uint32_t
get_timestamp_str(int level, char *buf, size_t len)
{
    int             result;
    struct tm       *tmTime;
    struct timeval  timestamp;
    
    if (len < 32)
        return 0;
    
    gettimeofday(&timestamp, NULL);
    tmTime = localtime((time_t *)&timestamp.tv_sec);
    
    result = sprintf(buf, "%02d:%02d:%02d.%06d %s ", 
        tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, timestamp.tv_usec,
        get_level_str(level));

    return result;
}

int
async_trace_init(void)
{
    output = async_ring_buf_new(TRACE_MAX_BUF);
    if (output == NULL)
        return -1;

    return 0;
}

void
async_trace_destroy(void)
{
    if (output)
        async_ring_buf_free(output);
}

static void
trace_args(int level, const char *format, va_list args)
{

    char           string[1024];
    uint32_t       len;
    
    if (output == NULL)
        return;
  
    len = get_timestamp_str(level, string, sizeof(string));
    if (len <= 0)
        return;       

    len += vsnprintf(string+len, sizeof(string) - len, format, args);
    if (len > 0)
        async_ring_buf_write(output, string, len);

    return;
}

void
trace(int level, const char *format, ...)
{
    va_list args;

    atomic_get(&output->level);    
    if (level < output->level)
        return;

    va_start(args, format);
    trace_args(level, format, args);
    va_end(args);    
}

void
trace_dump(int level, char *buffer, size_t size)
{
    int           i;      // used to keep track of line lengths
    char *line;  // used to print char version of data
    char ch;     // also used to print char version of data
    
    if (output == NULL)
      return;

    atomic_get(&output->level);        
    if (output->level > level)
      return;
    
    trace(level, "JPU-DUMP: ");
    if (size > 512)
    {
      trace(level, "buffer size is too big(%u bytes), now only display first 512 bytes.", size);
      size = 512;
    }
    trace(level, "\n");
    
    
    i = 0; 
    line = buffer; 
    
    trace(level, "%08X | ", (int)buffer); // print the address we are pulling from
    while (size-- > 0)
    {
      trace(level, "%02X ", *buffer++); // print each char
      if (!(++i % 16) || (size == 0 && i % 16))
      { 
        // if we come to the end of a line...
         
        // if this is the last line, print some fillers.
        if (size == 0)
        {
          while (i++ % 16)
          { 
            trace(level, "__ ");
          }
        }
    
        trace(level, "| ");
          
        while (line < buffer) // print the character version
        {  
          ch = *line++;
          trace(level, "%c", (ch < 33 || ch == 255) ? 0x2E : ch);
        }
          
        // If we are not on the last line, prefix the next line with the address.
        if (size > 0)
        {
          trace(level, "\n%08X | ", (int)buffer);
        }
      }
    }
    trace(level, "\n\n");
    
}

void
trace_adjust(int level)
{
    assert(output);
    
    atomic_set(&output->level, level);

    return;
}

