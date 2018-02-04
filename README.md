# SharedObjectStore

A header only library providing an object store whose objects can be shared among multiple threads (not yet implemented)

General idea:
- An object is created in a pre-allocated area and paired with a refcount
- The user gets a handle which allows modifications of that object, but the handle can't be copied (only moved)
- Copying the handle (for shareing accross multiple threads) is only possible by turning the handle into a read_only handle - invalidating the original handle
- Read only handles can be copied across multiple threads and as we know, that no one can modify the object, reading from it is safe
- For each copy of the read_only handle, the refcount is increased by one
- Turning a read only handle back into a mutable handle is only allowed if the refcount is 1 (i.e. we are the only one holding a handle to the object)
- If all handles are destroyed, the object is destroyed, too.