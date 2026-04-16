#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

void* vmm_alloc(size_t pages);

#endif /* VMM_H */