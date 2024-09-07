#include "ohos_types.h"
#include "hiview_cache.h"
#include "hiview_util.h"
#include "string.h"

static uint16 GetReadCursor(HiviewCache *cache);

boolean InitHiviewStaticCache(HiviewCache *cache, HiviewCacheType type, uint8 *buffer, uint16 size)
{
    if (cache == NULL)
    {
        return FALSE;
    }

    cache->mutex = HIVIEW_MutexInit();
    cache->usedSize = 0;
    cache->wCursor = 0;
    cache->buffer = buffer;
    cache->size = size;
    cache->type = type;

    return TRUE;
}

boolean InitHiviewCache(HiviewCache *cache, HiviewCacheType type, uint16 size)
{
    if (cache == NULL)
    {
        return FALSE;
    }

    uint8 *buffer = (uint8 *)HIVIEW_MemAlloc(MEM_POOL_HIVIEW_ID, size);
    if (buffer == NULL)
    {
        return FALSE;
    }

    cache->mutex = HIVIEW_MutexInit();
    cache->usedSize = 0;
    cache->wCursor = 0;
    cache->buffer = buffer;
    cache->size = size;
    cache->type = type;

    return TRUE;
}

int32 WriteToCache(HiviewCache *cache, const uint8 *data, uint16 wLen)
{
    if (cache == NULL || data == NULL || cache->buffer == NULL)
    {
        return -1;
    }

    uint16 firstLen;
    uint16 secondLen;
    HIVIEW_MutexLock(cache->mutex);
    if (cache->size < wLen + cache->usedSize)
    {
        HIVIEW_MutexUnlock(cache->mutex);
        return -1;
    }
    // overflow
    if (cache->wCursor + wLen > cache->size)
    {
        firstLen = cache->size - cache->wCursor;
        if (firstLen > 0)
        {
            memcpy(cache->buffer + cache->wCursor, data, firstLen);
            cache->wCursor += firstLen;
            cache->usedSize += firstLen;
        }
        cache->wCursor = 0;
        secondLen = wLen - firstLen;
        if (secondLen > 0)
        {
            memcpy(cache->buffer + cache->wCursor, data + firstLen, secondLen);
            cache->wCursor += secondLen;
            cache->usedSize += secondLen;
        }
    }
    else
    {
        memcpy(cache->buffer + cache->wCursor, data, wLen);
        cache->wCursor += wLen;
        cache->usedSize += wLen;
    }
    HIVIEW_MutexUnlock(cache->mutex);

    return wLen;
}

int32 ReadFromCache(HiviewCache *cache, uint8 *data, uint16 rLen)
{
    if (cache == NULL || data == NULL || cache->buffer == NULL)
    {
        return -1;
    }

    uint16 firstLen;
    uint16 secondLen;
    uint16 rCursor;
    // This function is the only read operation, so there is no need to lock
    if (cache->usedSize < rLen)
    {
        return -1;
    }
    rCursor = GetReadCursor(cache);
    // overflow
    if (rCursor + rLen > cache->size)
    {
        firstLen = cache->size - rCursor;
        if (firstLen > 0)
        {
            memcpy(data, cache->buffer + rCursor, firstLen);
        }
        secondLen = rLen - firstLen;
        if (secondLen > 0)
        {
            memcpy(data + firstLen, cache->buffer, secondLen);
        }
    }
    else
    {
        memcpy(data, cache->buffer + rCursor, rLen);
    }
    HIVIEW_MutexLock(cache->mutex);
    cache->usedSize -= rLen;
    HIVIEW_MutexUnlock(cache->mutex);

    return rLen;
}

int32 PrereadFromCache(HiviewCache *cache, uint8 *data, uint16 rLen)
{
    if (cache == NULL || data == NULL || cache->buffer == NULL)
    {
        return -1;
    }
    if (cache->usedSize < rLen)
    {
        return -1;
    }

    uint16 firstLen;
    uint16 secondLen;
    uint16 rCursor = GetReadCursor(cache);
    // overflow
    if (rCursor + rLen > cache->size)
    {
        firstLen = cache->size - rCursor;
        if (firstLen > 0)
        {
            memcpy(data, cache->buffer + rCursor, firstLen);
        }
        secondLen = rLen - firstLen;
        if (secondLen > 0)
        {
            memcpy(data + firstLen, cache->buffer, secondLen);
        }
    }
    else
    {
        memcpy(data, cache->buffer + rCursor, rLen);
    }

    return rLen;
}

void DiscardCacheData(HiviewCache *cache)
{
    if (cache == NULL)
    {
        return;
    }
    cache->wCursor = 0;
    cache->usedSize = 0;
}

void DestroyCache(HiviewCache *cache)
{
    if (cache == NULL)
    {
        return;
    }
    if (cache->buffer != NULL)
    {
        HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, cache->buffer);
        cache->buffer = NULL;
    }
    cache->wCursor = 0;
    cache->usedSize = 0;
    cache->size = 0;
}

static uint16 GetReadCursor(HiviewCache *cache)
{
    if (cache == NULL || cache->buffer == NULL)
    {
        return 0;
    }

    uint16 readCursor;
    HIVIEW_MutexLock(cache->mutex);
    if (cache->wCursor >= cache->usedSize)
    {
        readCursor = cache->wCursor - cache->usedSize;
    }
    else
    {
        readCursor = cache->size - (cache->usedSize - cache->wCursor);
    }
    HIVIEW_MutexUnlock(cache->mutex);
    return readCursor;
}
