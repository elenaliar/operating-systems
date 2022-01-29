// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct{ 
  struct spinlock lock;
  int reference_counter[PGROUNDUP(PHYSTOP)/PGSIZE];//metritis anaforon gia kathe fisiki selida opos proteinei to vima3
} ref_counter;

void
acquirereflock()
{
  acquire(&ref_counter.lock);
}

void 
releasereflock()
{
  release(&ref_counter.lock);
}

int 
getrefcounter(void* page)
{
  acquirereflock();
  int refcnt = ref_counter.reference_counter[PAGEINDEX(page)];
  releasereflock();
  return refcnt;
}

void
ref_counter_init()
{
  initlock(&ref_counter.lock, "ref_counter");
  acquire(&ref_counter.lock);
  for(int i=0; i<PGROUNDUP(PHYSTOP)/PGSIZE; i++){
    ref_counter.reference_counter[i] = 0;
  }
  release(&ref_counter.lock);
}

void
kinit()
{
  ref_counter_init();
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

//sinartisi pou auksanei ton metriti anaforon
void
kincref(void* pa)
{
  acquire(&ref_counter.lock);
  ref_counter.reference_counter[PAGEINDEX(pa)] += 1;
  release(&ref_counter.lock);
}

//sinartisi pou meionei ton metriti anaforon
void
kdecref(void* pa)
{
  acquire(&ref_counter.lock);
  ref_counter.reference_counter[PAGEINDEX(pa)] -= 1;
  release(&ref_counter.lock);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    kincref((void*)p);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  kdecref((void*)pa);

  int refcnt = getrefcounter(pa);
  if(refcnt>0)
    return;
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    kincref((void*)r);
  }
  return (void*)r;
}
