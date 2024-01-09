#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>

#define FILES_IN_FOLDER 10
#define NAME_LENGTH 30
#define FOLDERS_IN_FOLDER 10
#define FILE_LENGTH 8192

typedef struct file {
	char data[FILE_LENGTH];
	char name[NAME_LENGTH];
	int size;
	int occupied;
	// 8230 B
} file_t;

typedef struct dir {
	file_t files[FILES_IN_FOLDER];
	char name[NAME_LENGTH];
	int occupied;
	// 82334 B
} subdir_t;

typedef struct topdir {
	subdir_t subdirs[FOLDERS_IN_FOLDER];
	file_t files[FILES_IN_FOLDER];

} topdir_t;

typedef struct fs {
	topdir_t topdir;
} fs_t;

int is_file(const char *path);
int is_dir(const char *path);
subdir_t *get_subdir_from_path(const char *path);
file_t *get_file_from_path(const char *path);
int set_attrib_file(struct stat *st);
int set_attrib_dir(struct stat *st);
int serialize_filesystem(fs_t *filesystem, char *filename);
fs_t *deserialize_filesystem(char *filename);

fs_t *
deserialize_filesystem(char *filename)
{
	FILE *file = fopen(filename, "r");
	fs_t *filesystem = calloc(sizeof(fs_t), 1);
	if (filesystem == NULL)
		return NULL;
	// DESERIALIZAR
	if (file == NULL) {
	} else {
		fread(filesystem, 1, sizeof(fs_t), file);
	}
	return filesystem;
}
int
serialize_filesystem(fs_t *filesystem, char *filename)
{
	FILE *file = fopen(filename, "w");
	return fwrite(filesystem, sizeof(fs_t), 1, file);
}

fs_t *fs;

subdir_t *
get_subdir_from_path(const char *path)
{
	if (path[0] != '/')
		return NULL;  // caso en que la cadena de plano este mal
	if (path[1] == '\0')
		return NULL;  //
	for (int i = 0; i < FOLDERS_IN_FOLDER; i++) {
		if (strcmp(fs->topdir.subdirs[i].name, &path[1]) == 0 &&
		    fs->topdir.subdirs[i].occupied == 1) {
			return &fs->topdir.subdirs[i];
		}
	}
	return NULL;
}
file_t *
get_file_from_path(const char *path)
{
	int string_compare = strcmp(path, "/");
	if (string_compare > 0) {
		for (int i = 0; i < FILES_IN_FOLDER; i++) {
			if (strcmp(&path[1], fs->topdir.files[i].name) == 0 &&
			    fs->topdir.files[i].occupied == 1) {
				return &fs->topdir.files[i];
			}
		}
		int j = 1;
		for (; path[j] != '/'; j++) {
			if (path[j] == '\0') {
				return NULL;
			}
		}
		if (path[j + 1] == '\0') {
			errno = EINVAL;
			return NULL;
		}
		char subdir[NAME_LENGTH];
		char file[NAME_LENGTH];
		strncpy(subdir, &path[1], j - 1);
		strcpy(file, &path[j + 1]);
		subdir[j - 1] = '\0';
		for (int i = 0; i < FOLDERS_IN_FOLDER; i++) {
			if (strcmp(subdir, fs->topdir.subdirs[i].name) == 0) {
				for (int k = 0; k < FILES_IN_FOLDER; k++) {
					if (strcmp(file,
					           fs->topdir.subdirs[i]
					                   .files[k]
					                   .name) == 0 &&
					    fs->topdir.subdirs[i].files[k].occupied ==
					            1) {
						return &fs->topdir.subdirs[i].files[k];
					}
				}
			}
		}
		return NULL;
	} else {
		return NULL;
	}
}

int
is_dir(const char *path)
{
	if (strcmp(path, "/") == 0)
		return 1;
	else if (get_subdir_from_path(path) != NULL)
		return 1;
	else
		return 0;
}
int
is_file(const char *path)
{
	if (get_file_from_path(path) != NULL)
		return 1;
	else
		return 0;
}

void *
fisopfs_init(struct fuse_conn_info *conn)
{
	fs = deserialize_filesystem("archivo");
	return NULL;
}
void
fisopfs_destroy(void *private_data)
{
	printf("[debug] destroy\n");
	serialize_filesystem(fs, "archivo");
}
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	if (strcmp(path, "/") == 0 || is_dir(path) == 1) {
		set_attrib_dir(st);
	} else if (is_file(path) == 1) {
		set_attrib_file(st);
		file_t *file = get_file_from_path(path);
		st->st_size = file->size;
	} else {
		return -ENOENT;
	}
	return 0;
}
int
set_attrib_file(struct stat *st)
{
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time(NULL);
	st->st_mtime = time(NULL);
	st->st_mode = __S_IFREG | 0664;
	st->st_nlink = 1;
	st->st_size = FILE_LENGTH;
	return 0;
}
int
set_attrib_dir(struct stat *st)
{
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time(NULL);
	st->st_mtime = time(NULL);
	st->st_mode = __S_IFDIR | 0777;
	st->st_nlink = 2;
	return 0;
}

static int
fisopfs_fgetattr(const char *path, struct stat *stbuf)
{
	printf("[debug] fgetattr not implemented\n");
	return -1;
}
static int
fisopfs_access(const char *path, int mask)
{
	if (is_file(path) || is_dir(path)) {
		return 0;
	} else {
		errno = ENOENT;
		return -ENOENT;
	}
}
static int
fisopfs_readlink(const char *path, char *buf, size_t size)
{
	printf("[debug] readlink not implemented\n");
	return -1;
}
static int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_opendir - path: %s\n", path);
	return 0;
}
static int
fisopfs_readdir(const char *path,
                void *buf,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);
	int string_compare = strcmp(path, "/");
	struct stat stfile;
	struct stat stdir;
	set_attrib_file(&stfile);
	set_attrib_file(&stdir);
	if (string_compare == 0) {
		filler(buf, ".", &stdir, 0);
		filler(buf, "..", NULL, 0);
		for (int i = 0; i < FOLDERS_IN_FOLDER; i++) {
			if (fs->topdir.subdirs[i].occupied == 1)
				filler(buf, fs->topdir.subdirs[i].name, &stdir, 0);
		}
		for (int i = 0; i < FILES_IN_FOLDER; i++) {
			if (fs->topdir.files[i].occupied == 1) {
				stfile.st_size = fs->topdir.files[i].size;
				filler(buf, fs->topdir.files[i].name, &stfile, 0);
			}
		}
		return 0;
	} else if (string_compare > 0) {
		for (int j = 0; j < FOLDERS_IN_FOLDER; j++) {
			if (strcmp(fs->topdir.subdirs[j].name, &path[1]) == 0 &&
			    fs->topdir.subdirs[j].occupied == 1) {
				filler(buf, ".", &stdir, 0);
				filler(buf, "..", &stdir, 0);
				for (int i = 0; i < FILES_IN_FOLDER; i++) {
					if (fs->topdir.subdirs[j].files[i].occupied ==
					    1) {
						stfile.st_size =
						        fs->topdir.subdirs[j]
						                .files[i]
						                .size;
						filler(buf,
						       fs->topdir.subdirs[j]
						               .files[i]
						               .name,
						       &stfile,
						       0);
					}
				}
				return 0;
			}
		}
		errno = ENOENT;
		return -1;
	} else {
		errno = EINVAL;
		return -1;
	}
}
static int
fisopfs_mknod(const char *path, mode_t mode, dev_t dev)
{
	printf("[debug] fisopfs_mknod not implemented\n");
	return -1;
}
static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	char directory[NAME_LENGTH];
	char file[NAME_LENGTH];
	if (is_file(path)) {
		return -EEXIST;
	}
	if (path[0] != '/') {
		return -EINVAL;
	}
	int slash_location = 0;
	int j = 1;
	for (; path[j] != '\0'; j++) {
		if (path[j] == '/') {
			slash_location = j;
			break;
		}
	}
	if (slash_location == 0) {
		strcpy(file, &path[1]);
		for (int x = 0; x < FILES_IN_FOLDER; x++) {
			if (fs->topdir.files[x].occupied == 0) {
				fs->topdir.files[x].occupied = 1;
				strncpy(fs->topdir.files[x].name, file, NAME_LENGTH);
				return 0;
			}
		}
		return -ENOMEM;
	} else {
		strcpy(file, &path[slash_location + 1]);
		strncpy(directory, &path[1], slash_location - 1);
		for (int y = 0; y < FOLDERS_IN_FOLDER; y++) {
			if (strcmp(directory, fs->topdir.subdirs[y].name) == 0) {
				for (int x = 0; x < FILES_IN_FOLDER; x++) {
					if (fs->topdir.subdirs[y].files[x].occupied ==
					    0) {
						fs->topdir.subdirs[y].files[x].occupied =
						        1;
						strncpy(fs->topdir.subdirs[y]
						                .files[x]
						                .name,
						        file,
						        NAME_LENGTH);
						return 0;
					}
				}
				return -ENOMEM;
			}
		}
		for (int y = 0; y < FOLDERS_IN_FOLDER; y++) {
			if (fs->topdir.subdirs[y].occupied == 0) {
				fs->topdir.subdirs[y].occupied = 1;
				strncpy(fs->topdir.subdirs[y].name,
				        directory,
				        NAME_LENGTH);
				fs->topdir.subdirs[y].files[0].occupied = 1;
				strncpy(fs->topdir.subdirs[y].files[0].name,
				        file,
				        NAME_LENGTH);
				return 0;
			}
		}
		return -ENOMEM;
	}
}
static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	int x = 0;
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	for (int i = 1; path[i] != '\0'; i++) {
		// no se permiten subdirectorios
		if (path[i] == '/') {
			errno = EINVAL;
			return -1;
		}
	}
	for (int i = 0; i < FOLDERS_IN_FOLDER; i++) {
		if (strcmp(fs->topdir.subdirs[i].name, &path[1]) == 0 ||
		    strcmp(fs->topdir.files[i].name, &path[1]) == 0) {
			errno = EEXIST;
			return -1;
		}
	}
	for (; x < FOLDERS_IN_FOLDER; x++) {
		if (fs->topdir.subdirs[x].occupied == 0) {
			fs->topdir.subdirs[x].occupied = 1;
			strcpy(fs->topdir.subdirs[x].name, &path[1]);
			return 0;
		}
	}
	errno = EINVAL;
	return -1;
}
static int
fisopfs_unlink(const char *path)
{
	file_t *file = get_file_from_path(path);
	if (file == NULL) {
		errno = ENOENT;
		return -1;
	}
	file->occupied = 0;
	return 0;
}
static int
fisopfs_rmdir(const char *path)
{
	subdir_t *directory = get_subdir_from_path(path);
	if (directory == NULL) {
		errno = ENOENT;
		return -1;
	}
	for (int x = 0; x < FILES_IN_FOLDER; x++) {
		if (directory->files->occupied == 1) {
			errno = ENOTEMPTY;
			return -1;
		}
	}
	directory->occupied = 0;
	return 0;
}
static int
fisopfs_symlink(const char *to, const char *from)
{
	printf("[debug] symlink not implemented\n");
	return -1;
}
static int
fisopfs_rename(const char *from, const char *to)
{
	printf("[debug] rename not implemented\n");
	return -1;
}
static int
fisopfs_link(const char *from, const char *to)
{
	printf("[debug] link not implemented\n");
	return -1;
}
static int
fisopfs_chmod(const char *path, mode_t mode)
{
	printf("[debug] chmod not implemented\n");
	return -1;
}
static int
fisopfs_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("[debug] chown not implemented\n");
	return -1;
}
static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] truncate - path: %s\n", path);
	file_t *file = get_file_from_path(path);
	if (file == NULL) {
		return -ENOENT;
	}
	file->size = size;
	return 0;
}
static int
fisopfs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	printf("[debug] ftruncate not implemented\n");
	return -1;
}
static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	printf("[debug] utimens not implemented\n");
	return -1;
}
static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_open - path: %s\n", path);
	if (is_file(path) == 1) {
		return 0;
	}
	errno = ENOENT;
	return -1;
}
static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s\n", path);
	file_t *file = get_file_from_path(path);
	if (file == NULL) {
		printf("file not exists");
		errno = ENOENT;
		return -1;
	}
	if (size + offset > FILE_LENGTH) {
		errno = EINVAL;
		return -1;
	}
	char *data = file->data;
	memcpy(buffer, data + offset, size);
	return strlen(data) - offset;
}
static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *)
{
	printf("[debug] fisopfs_write - path: %s\n", path);
	file_t *file = get_file_from_path(path);
	if (file == NULL) {
		errno = ENOENT;
		return -1;
	}
	if (size + offset > FILE_LENGTH) {
		errno = EINVAL;
		return -1;
	}
	char *data = file->data;
	memcpy(data + offset, buf, size);
	file->size = size + offset;
	return size;
}
static int
fisopfs_statfs(const char *path, struct statvfs *stbuf)
{
	printf("[debug] statfs not implemented\n");
	return -1;
}
static int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] release mocked\n");
	return 0;
}
static int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] releasedir mocked\n");
	return 0;
}
static int
fisopfs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	printf("[debug] fsync not implemented\n");
	return -1;
}
static int
fisopfs_fsyncdir(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	printf("[debug] fsyncdir not implemented\n");
	return -1;
}
static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] flush\n");
	serialize_filesystem(fs, "archivo");
	return 0;
}
static int
fisopfs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *locks)
{
	printf("[debug] lock mocked\n");
	return 0;
}
static int
fisopfs_bmap(const char *path, size_t blocksize, uint64_t *blockno)
{
	printf("[debug] bmap not implemented\n");
	return -1;
}
static int
fisopfs_setxattr(const char *path,
                 const char *name,
                 const char *value,
                 size_t size,
                 int flags)
{
	printf("[debug] setxattr not implemented\n");
	return -1;
}
static int
fisopfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
	printf("[debug] getxattr not implemented\n");
	return -1;
}
static int
fisopfs_listxattr(const char *path, char *list, size_t size)
{
	printf("[debug] listxattr not implemented\n");
	return -1;
}
static int
fisopfs_ioctl(const char *path,
              int cmd,
              void *arg,
              struct fuse_file_info *fi,
              unsigned int flags,
              void *data)
{
	printf("[debug] ioctl not implemented\n");
	return -1;
}
static int
fisopfs_poll(const char *path,
             struct fuse_file_info *fi,
             struct fuse_pollhandle *ph,
             unsigned *reventsp)
{
	printf("[debug] poll not implemented\n");
	return -1;
}
static struct fuse_operations operations = {
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.getattr = fisopfs_getattr,
	.access = fisopfs_access,
	.readlink = fisopfs_readlink,
	.opendir = fisopfs_opendir,
	.readdir = fisopfs_readdir,
	.mknod = fisopfs_mknod,
	.create = fisopfs_create,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.symlink = fisopfs_symlink,
	.rename = fisopfs_rename,
	.link = fisopfs_link,
	.chmod = fisopfs_chmod,
	.chown = fisopfs_chown,
	.truncate = fisopfs_truncate,
	.ftruncate = fisopfs_ftruncate,
	.utimens = fisopfs_utimens,
	.open = fisopfs_open,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.statfs = fisopfs_statfs,
	.release = fisopfs_release,
	.releasedir = fisopfs_releasedir,
	.fsync = fisopfs_fsync,
	.fsyncdir = fisopfs_fsyncdir,
	.flush = fisopfs_flush,
	.lock = fisopfs_lock,
	.bmap = fisopfs_bmap,
	//.setxattr = fisopfs_setxattr,
	//.getxattr = fisopfs_getxattr,
	//.listxattr = fisopfs_listxattr,
	.ioctl = fisopfs_ioctl,
	.poll = fisopfs_poll,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
