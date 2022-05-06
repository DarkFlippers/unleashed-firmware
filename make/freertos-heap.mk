LDFLAGS		+= -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r -Wl,--wrap,_calloc_r -Wl,--wrap,_realloc_r
