#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 500

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>
#include <libgen.h>
#include <sys/xattr.h>

typedef struct fusefs_private_data {
	char *rootdir;
    char *mount_point;
} fusefs_pdata;

static char *fusefs_fullpath(const char *path) {
	char *fullpath = (char *)malloc((strlen(((fusefs_pdata *)(fuse_get_context()->private_data))->rootdir) + strlen(path) + 1) * sizeof(char));
	strcpy(fullpath, ((fusefs_pdata *)(fuse_get_context()->private_data))->rootdir);
	strcat(fullpath, path);
    return fullpath;
}

static int fusefs_getattr(const char *path, struct stat *stbuf) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = lstat(fullpath, stbuf);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_access(const char *path, int mask) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = access(fullpath, mask);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_readlink(const char *path, char *buf, size_t size) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = readlink(fullpath, buf, size - 1);
	free(fullpath);
	if(ret == -1)
		return -errno;
	buf[ret] = '\0';
	return 0;
}


static int fusefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	DIR *dp;
	struct dirent *de;
	char *fullpath;
	struct stat st;

	fullpath = fusefs_fullpath(path);
	dp = opendir(fullpath);
	free(fullpath);
	if(dp == NULL)
		return -errno;
	while ((de = readdir(dp)) != NULL) {
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if(filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int fusefs_mknod(const char *path, mode_t mode, dev_t rdev) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = mknod(fullpath, mode, rdev);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_mkdir(const char *path, mode_t mode){
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = mkdir(fullpath, mode);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_unlink(const char *path) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = unlink(fullpath);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_rmdir(const char *path) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = rmdir(fullpath);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

/*//This function has not been implement see man 3 symlink, if you would like to implement it
static int fusefs_symlink(const char *from, const char *to) {
}*/

static int fusefs_rename(const char *from, const char *to) {
	int ret;
	char *fullfrom;
	char *fullto;

	fullfrom = fusefs_fullpath(from);
	fullto = fusefs_fullpath(to);
	ret = rename(fullfrom, fullto);
	free(fullfrom);
	free(fullto);
	if(ret == -1)
		return -errno;
	return 0;
}


/*//This function has not been implement see man 3 link, if you would like to implement it
static int fusefs_link(const char *from, const char *to) {
}*/

static int fusefs_chmod(const char *path, mode_t mode) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = chmod(fullpath, mode);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_chown(const char *path, uid_t uid, gid_t gid) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = lchown(fullpath, uid, gid);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_truncate(const char *path, off_t size) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = truncate(fullpath, size);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_utimens(const char *path, const struct timespec ts[2]) {
	int ret;
	struct timeval tv[2];
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	ret = utimes(fullpath, tv);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;
}

static int fusefs_open(const char *path, struct fuse_file_info *fi) {
	int ret;
	char *fullpath;
	char buf[2];
	struct stat st;
	struct stat st2;

	buf[0] = '0';
	buf[1] = '\0';
	fullpath = fusefs_fullpath(path);
	ret = open(fullpath, fi->flags);
	free(fullpath);
	if(ret == -1)
		return -errno;
	close(ret);
	return 0;
}

static int fusefs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	int fd;
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	fd = open(fullpath, O_RDONLY);
	free(fullpath);
	if(fd == -1)
		return -errno;
	ret = pread(fd, buf, size, offset);
	if(ret == -1)
		ret = -errno;
	close(fd);
	return ret;
}

static int fusefs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	int fd;
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	fd = open(fullpath, O_WRONLY);
	free(fullpath);
	if(fd == -1)
		return -errno;

	ret = pwrite(fd, buf, size, offset);
	close(fd);
	return ret;
}

static int fusefs_statfs(const char *path, struct statvfs *stbuf) {
	int ret;
	char *fullpath;

	fullpath = fusefs_fullpath(path);
	ret = statvfs(fullpath, stbuf);
	free(fullpath);
	if(ret == -1)
		return -errno;
	return 0;	
}

static struct fuse_operations fusefs_oper = {
	.getattr	= fusefs_getattr,
	.access		= fusefs_access,
	.readlink	= fusefs_readlink,
	.readdir	= fusefs_readdir,
	.mknod		= fusefs_mknod,
	.mkdir		= fusefs_mkdir,
	//.symlink	= fusefs_symlink,
	.unlink		= fusefs_unlink,
	.rmdir		= fusefs_rmdir,
	.rename		= fusefs_rename,
	//.link		= fusefs_link,
	.chmod		= fusefs_chmod,
	.chown		= fusefs_chown,
	.truncate	= fusefs_truncate,
	.utimens	= fusefs_utimens,
	.open		= fusefs_open,
	.read		= fusefs_read,
	.write		= fusefs_write,
	.statfs		= fusefs_statfs,
};

int main(int argc, char *argv[])
{
	fusefs_pdata *pdata;
	struct stat st;

	if(argc != 2)
	{
		printf("USAGE: ./fusefs mountdir\n");
		return 1;
	}
	if(stat("/tmp/fusefs", &st) != 0)
	{
		mkdir("/tmp/fusefs", 0755);
	}
	pdata = (fusefs_pdata *)malloc(sizeof(fusefs_pdata));
	pdata->rootdir = strdup("/tmp/fusefs");
    pdata->mount_point = realpath(argv[1], NULL);
	umask(0);
	return fuse_main(argc, argv, &fusefs_oper, pdata);
}
