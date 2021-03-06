#ifndef __SHMEM_FS_H
#define __SHMEM_FS_H

#include <linux/file.h>
#include <linux/swap.h>
#include <linux/mempolicy.h>
#include <linux/pagemap.h>
#include <linux/percpu_counter.h>
#include <linux/xattr.h>

/* inode in-kernel data */

struct shmem_inode_info {
	spinlock_t		lock;
	unsigned int		seals;		/* shmem seals */
	unsigned long		flags;
	unsigned long		alloced;	/* data pages alloced to file */
	unsigned long		swapped;	/* subtotal assigned to swap */
	struct list_head	shrinklist;	/* shrinkable hpage inodes */
	struct list_head	swaplist;	/* chain of maybes on swap */
	struct shared_policy	policy;		/* NUMA memory alloc policy */
	struct simple_xattrs	xattrs;		/* list of xattrs */
	atomic_t		recoveries;	/* huge recovery work queued */
	struct inode		vfs_inode;
};

struct shmem_sb_info {
	unsigned long max_blocks;   /* How many blocks are allowed */
	struct percpu_counter used_blocks;  /* How many are allocated */
	unsigned long max_inodes;   /* How many inodes are allowed */
	unsigned long free_inodes;  /* How many are left for allocation */
	spinlock_t stat_lock;	    /* Serialize shmem_sb_info changes */
	umode_t mode;		    /* Mount mode for root directory */
	unsigned char huge;	    /* Whether to try for hugepages */
	kuid_t uid;		    /* Mount uid for root directory */
	kgid_t gid;		    /* Mount gid for root directory */
	struct mempolicy *mpol;     /* default memory policy for mappings */
};

static inline struct shmem_inode_info *SHMEM_I(struct inode *inode)
{
	return container_of(inode, struct shmem_inode_info, vfs_inode);
}

/*
 * Functions in mm/shmem.c called directly from elsewhere:
 */
extern int shmem_init(void);
extern int shmem_fill_super(struct super_block *sb, void *data, int silent);
extern struct file *shmem_file_setup(const char *name,
					loff_t size, unsigned long flags);
extern struct file *shmem_kernel_file_setup(const char *name, loff_t size,
					    unsigned long flags);
extern int shmem_zero_setup(struct vm_area_struct *);
extern unsigned long shmem_get_unmapped_area(struct file *, unsigned long addr,
		unsigned long len, unsigned long pgoff, unsigned long flags);
extern int shmem_lock(struct file *file, int lock, struct user_struct *user);
extern bool shmem_mapping(struct address_space *mapping);
extern void shmem_unlock_mapping(struct address_space *mapping);
extern struct page *shmem_read_mapping_page_gfp(struct address_space *mapping,
					pgoff_t index, gfp_t gfp_mask);
extern void shmem_truncate_range(struct inode *inode, loff_t start, loff_t end);
extern int shmem_unuse(swp_entry_t entry, struct page *page);

extern unsigned long shmem_swap_usage(struct vm_area_struct *vma);
extern unsigned long shmem_partial_swap_usage(struct address_space *mapping,
						pgoff_t start, pgoff_t end);

static inline struct page *shmem_read_mapping_page(
				struct address_space *mapping, pgoff_t index)
{
	return shmem_read_mapping_page_gfp(mapping, index,
					mapping_gfp_mask(mapping));
}

#ifdef CONFIG_TMPFS
extern int shmem_add_seals(struct file *file, unsigned int seals);
extern int shmem_get_seals(struct file *file);
extern long shmem_fcntl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static inline long shmem_fcntl(struct file *f, unsigned int c, unsigned long a)
{
	return -EINVAL;
}
#endif /* CONFIG_TMPFS */

#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && defined(CONFIG_SHMEM)
extern bool shmem_recovery_migrate_page(struct page *new, struct page *page);
# ifdef CONFIG_SYSCTL
struct ctl_table;
extern int shmem_huge, shmem_huge_min, shmem_huge_max;
extern int shmem_huge_recoveries;
extern int shmem_huge_sysctl(struct ctl_table *table, int write,
			     void __user *buffer, size_t *lenp, loff_t *ppos);
# endif /* CONFIG_SYSCTL */
#else
static inline bool shmem_recovery_migrate_page(struct page *new, struct page *p)
{
	return true;	/* Never called: true will optimize out the fallback */
}
#endif /* CONFIG_TRANSPARENT_HUGEPAGE && CONFIG_SHMEM */

#endif
