// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define PA2IDX(pa) ((uint64)pa - KERNBASE) / PGSIZE
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

struct {
  struct spinlock lock;
  int count[(PGROUNDUP(PHYSTOP) - KERNBASE)/PGSIZE];
} refcnt;

void
krefincr(void *pa)
{
  acquire(&refcnt.lock);
  refcnt.count[PA2IDX(pa)]++;
  release(&refcnt.lock);
}

void
krefdecr(void *pa)
{
  acquire(&refcnt.lock);
  refcnt.count[PA2IDX(pa)]--;
  release(&refcnt.lock);
}

int
krefget(void *pa)
{
  int cnt;
  acquire(&refcnt.lock);
  cnt = refcnt.count[PA2IDX(pa)];
  release(&refcnt.lock);
  return cnt;
}




void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refcnt.lock, "refcnt");
  // ** Must reset count array before freerange
  for (int i = 0; i < (PGROUNDUP(PHYSTOP)-KERNBASE) / PGSIZE; i++)
    refcnt.count[i] = 1;
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
   struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  int cnt = krefget(pa);

  // ** safety check
  if (cnt <= 0){
    panic("kfree_decr");
  } else if(cnt > 1) {
    krefdecr(pa);
  } else{ //cnt == 1
    krefdecr(pa);
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  return;
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
        // ** Set refcnt as 1 when allocate new page
    acquire(&refcnt.lock);
    refcnt.count[PA2IDX(r)] = 1;
    release(&refcnt.lock);
  }
  return (void*)r;
}
