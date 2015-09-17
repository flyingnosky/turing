//wendyfs.c

/*
   * 本模块代码大部分拷贝自fs/ramfs/inode.c和fs/ramfs/file-mmu.c。
   * 主要实现了一个RAM文件系统的基本功能（除了文件重命名 mv fileA fileB）
   * 本模块可以帮助我们学习Linux文件系统。 
*/


#include <linux/fs.h> /* super_operations */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pagemap.h> /*mapping_set_unevictable() */
#include <linux/time.h> /* CURRENT_TIME */ 
#include <linux/backing-dev.h> 



//should be defined in the heaeder files
#define WENDYFS_MAGIC     0xa5a5a5a5


struct inode *wendyfs_get_inode(struct super_block *sb,
                const struct inode *dir, umode_t mode, dev_t dev);
int myset_page_dirty_no_writeback(struct page *page);


/* Wendyfs's address space operations */
const struct address_space_operations wendyfs_aops = {
    .readpage	= simple_readpage,
    .write_begin	= simple_write_begin,
    .write_end		= simple_write_end,
    .set_page_dirty	= myset_page_dirty_no_writeback,
};

static struct backing_dev_info wendyfs_backing_dev_info = { 
    .name       = "wendyfs",
    .ra_pages   = 0,    /* No readahead */
    .capabilities   = BDI_CAP_NO_ACCT_AND_WRITEBACK |
              BDI_CAP_MAP_DIRECT | BDI_CAP_MAP_COPY |
              BDI_CAP_READ_MAP | BDI_CAP_WRITE_MAP | BDI_CAP_EXEC_MAP,
};


/*
 * For address_spaces which do not use buffers nor write back.
 */
int myset_page_dirty_no_writeback(struct page *page)
{
    if (!PageDirty(page))
        return !TestSetPageDirty(page);
    return 0;
}


/* File creation */
static int wendyfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
  int error = -ENOSPC;
  struct inode* inode = wendyfs_get_inode(dir->i_sb, dir, mode, dev);

  if (inode) {
    d_instantiate(dentry, inode);
    dget(dentry);   /* Extra count - pin the dentry in core */
    error = 0;
    dir->i_mtime = dir->i_ctime = CURRENT_TIME;
  }

  return error;
}

static int wendyfs_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
{
  int retval = wendyfs_mknod(dir, dentry, mode | S_IFDIR, 0); // DIR inode
  	if (!retval)
      inc_nlink(dir);

  return retval;
}


static int wendyfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    return wendyfs_mknod(dir, dentry, mode | S_IFREG, 0); 
}


static const struct inode_operations wendyfs_dir_inode_operations = {
  .create		= wendyfs_create,
  .lookup		= simple_lookup,
  .mkdir		= wendyfs_mkdir,
  .rmdir		= simple_rmdir,
  .link		= simple_link,
  .unlink		= simple_unlink,
//	.rename		= simple_rename, 
};


const struct inode_operations wendyfs_file_inode_operations = {
  .setattr	= simple_setattr,
  .getattr	= simple_getattr,
};


const struct file_operations wendyfs_file_operations = {
    .read       = do_sync_read,
    .aio_read   = generic_file_aio_read,
    .write      = do_sync_write,
    .aio_write  = generic_file_aio_write,
    .fsync      = noop_fsync,
    .llseek     = generic_file_llseek,
};

struct inode *wendyfs_get_inode(struct super_block *sb,
                const struct inode *dir, umode_t mode, dev_t dev) 
{
  /* Allocate one inode */
    struct inode * inode = new_inode(sb); 

  /* Init the inode */
    if (inode) {
    inode->i_ino = get_next_ino();

    /* Init uid,gid,mode for new inode according to posix standards */
      inode_init_owner(inode, dir, mode);
    
    /* Set the address space operation set */
    inode->i_mapping->a_ops = &wendyfs_aops;
    
    /* Set the backing device info */
    inode->i_mapping->backing_dev_info = &wendyfs_backing_dev_info;
    
    /* The pages wendyfs covered will be placed on unevictable_list. So
       these pages will not be reclaimed. */
    mapping_set_unevictable(inode->i_mapping);
    inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
    
    /* Set inode and file operation sets */
    switch (mode & S_IFMT) {
    default: 
        init_special_inode(inode, mode, dev);
        break;
    case S_IFDIR:
        /* dir inode operation set */
        inode->i_op = &wendyfs_dir_inode_operations;
        /* dir operation set */
        inode->i_fop = &simple_dir_operations;
        inc_nlink(inode);
        break;
    case S_IFREG:
        /* regular file inode operation set */
        inode->i_op = &wendyfs_file_inode_operations;
        /* regular file operation set */
        inode->i_fop = &wendyfs_file_operations;
        break;
    }
  }

  return inode;
}

/* Super Block related operations */
static const struct super_operations wendyfs_ops = { 
    .statfs     = simple_statfs,
    .drop_inode = generic_delete_inode,
    .show_options   = generic_show_options,
};


int wendyfs_fill_super(struct super_block* sb, void* data, int silent)
{
  struct inode *inode;

  sb->s_maxbytes		= 4096;
  sb->s_blocksize		= 4096;
  sb->s_blocksize_bits	= 12;
  sb->s_magic			= WENDYFS_MAGIC;
  /* Set super block operations */
  sb->s_op			= &wendyfs_ops;
  sb->s_time_gran		= 1;

  /* Create and initialize the root inode */
  inode = wendyfs_get_inode(sb, NULL, S_IFDIR, 0);
  sb->s_root = d_make_root(inode);
  if (!sb->s_root)
      return -ENOMEM;

  return 0;
}



struct dentry* wendyfs_mount(struct file_system_type *fs_type,
        int flags, const char* dev_name, void* data)
{
  return mount_nodev(fs_type, flags, data, wendyfs_fill_super);
}

static void wendyfs_kill_sb(struct super_block* sb)
{
  kill_litter_super(sb);
}

static struct file_system_type	wendy_fs_type = {
    .name		= "wendyfs",
    .mount		= wendyfs_mount,
    .kill_sb	= wendyfs_kill_sb,
};


static int __init init_wendy_fs(void)
{
    return register_filesystem(&wendy_fs_type);
}

static void __exit exit_wendy_fs(void)
{
  unregister_filesystem(&wendy_fs_type);
}


module_init(init_wendy_fs)
module_exit(exit_wendy_fs)

MODULE_AUTHOR("Yang Honggang, <eagle.rtlinux@gmail.com>");
MODULE_LICENSE("GPL");
