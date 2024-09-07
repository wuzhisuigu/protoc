#include "dbg_fifo.h"
#include "stdint.h"
#include "ohos_errno.h"
#include "bsp_mm.h"
#include "hiview_def.h"

#include "stdio.h"

#define DBG_FIFO_NUM_MAX 3

struct dbg_fifo *g_fifos[DBG_FIFO_NUM_MAX];
struct dbg_fifo_reader g_fifo_readers[DBG_FIFO_NUM_MAX];

int fifo_log_init(int id)
{
    void *fifo_ptr = NULL;
    int fifo_size = 0;
    int ret = 0;

    if (id >= DBG_FIFO_NUM_MAX) {
        return EC_INVALID;
    }

    if (id != BSP_MM_REGION_TRT_LOG) {
        // TODO support more fifo in the future
        return EC_FAILURE;
    }

    bsp_mm_region_get(BSP_MM_REGION_TRT_LOG, &fifo_ptr, &fifo_size);

    struct dbg_fifo_init_settings init_settings = { 0 };
    init_settings.buffer = fifo_ptr;
    /* max string len of a log, 256 should be enough for most logs */
    /* need to left the space of atomic in element size */
    init_settings.element_size = LOG_FMT_MAX_LEN - sizeof(struct atomic);
    init_settings.nelements = (fifo_size / LOG_FMT_MAX_LEN); // 256
    init_settings.buffer_size = fifo_size; // should be 32K

    ret = dbg_fifo_init(&g_fifos[id], &init_settings);
    if (ret) {
        return EC_NOMEMORY;
    }

    dbg_fifo_reader_init(&g_fifo_readers[id], g_fifos[id]);
    return 0;
}

int fifo_log_read(int id, char *text, int size)
{
    int ret;
    struct dbg_fifo_reader *reader;
    if (id >= DBG_FIFO_NUM_MAX) {
        return EC_INVALID;
    }
    reader = &g_fifo_readers[id];
    if (!reader) {
        /* not open yet */
        return EC_ILLEGALSTATE;
    }

    ret = dbg_fifo_reader_read(reader, text, size);

    if (ret == DBG_FIFO_OVERFLOW) {
        dbg_fifo_reader_resync(reader);
    }

    return ret;
}
