
#include "iunknown.h"
#include "samgr_common.h"

#define OLD_VERSION 0

int IUNKNOWN_AddRef(IUnknown *iUnknown)
{
    if (iUnknown == NULL)
    {
        return EC_INVALID;
    }

    IUnknownEntry *entry = GET_OBJECT(iUnknown, IUnknownEntry, iUnknown);
    entry->ref++;
    return entry->ref;
}

int IUNKNOWN_QueryInterface(IUnknown *iUnknown, int ver, void **target)
{
    if (iUnknown == NULL || target == NULL)
    {
        return EC_INVALID;
    }

    IUnknownEntry *entry = GET_OBJECT(iUnknown, IUnknownEntry, iUnknown);
    if ((entry->ver & (uint16)ver) != ver)
    {
        return EC_INVALID;
    }

    if (ver == OLD_VERSION &&
        entry->ver != OLD_VERSION &&
        (entry->ver & (uint16)DEFAULT_VERSION) != DEFAULT_VERSION)
    {
        return EC_INVALID;
    }

    *target = iUnknown;
    iUnknown->AddRef(iUnknown);
    return EC_SUCCESS;
}

int IUNKNOWN_Release(IUnknown *iUnknown)
{
    if (iUnknown == NULL)
    {
        return EC_INVALID;
    }

    IUnknownEntry *entry = GET_OBJECT(iUnknown, IUnknownEntry, iUnknown);
    int ref = entry->ref - 1;
    if (ref < 0)
    {
        // The iUnknown is already freed, there is some exception;
    }
    else
    {
        if (ref == 0)
        {
            // Nobody reference to the iUnknown, should delete it.
            // But iUnknown may be global variable, so the default version don`t delete it.
        }
        else
        {
            entry->ref = ref;
        }
    }
    return ref;
}