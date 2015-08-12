There are two kind of memory allocating interface.one is from fridend system, and the other is from slab system.
Friend system provision memory manage based page, and slab system provides memory manage based on objection.
(1)page memory allocation
_get_free_pages  =  alloc_pages + page_address
(2)slab system
kmem_cache_create: create slab object
kmem_cache_alloc: apply memory
kmem_cache_free: free memory
eg. bh_cachep = kmem_cache_create("buffer_head",sizeof(struct buffer_head),0,(SLAB_RECLAIM_ACCOUNT|SLAB_PANIC)
	,init_buffer_head, NULL);
    struct buffer_head *ret = kmem_cache_alloc(bh_cachep,gfp_flags);

(3)vmalloc: make uncontinue address in physical to continue address in logic
