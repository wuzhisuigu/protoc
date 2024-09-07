#ifndef __HIVIEW_FIFO_LOG_H__
#define __HIVIEW_FIFO_LOG_H__

#define HIVIEW_FIFO_LOG_TRT 0

int fifo_log_init(int id);
int fifo_log_read(int id, char *text, int size);

#endif