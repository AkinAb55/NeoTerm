#ifndef _UK_LINUX_PATH_H
#define _UK_LINUX_PATH_H
struct path { struct vfsmount *mnt; struct dentry *dentry; };
struct vfsmount; struct dentry;
void path_put(const struct path *path);
int path_get(const struct path *path);
#endif
