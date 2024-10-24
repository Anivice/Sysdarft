#define FUSE_USE_VERSION 31
#include <fuse/fuse.h>
#include <cstring>
#include <string>
#include <ranges>
#include <filesystem>
#include <debug.h>
#include <res_packer.h>
#include <fuse_operations.h>
#include <resource_file_list.h>
#include <thread>

#if (!SUPRESS_DEBUG_INFO)
#define INVK_NOTIFICATION(__path__) sysdarft_log::log(sysdarft_log::LOG_NORMAL,          \
                                         "[FUSE] Callback invoked: ", __FUNCTION__,      \
                                         " ", (__path__), "\n"); __asm__("nop")
#define EXIT_NOTIFICATION(__path__) sysdarft_log::log(sysdarft_log::LOG_NORMAL,          \
                                         "[FUSE] Callback exited: ", __FUNCTION__,       \
                                         " ", (__path__), "\n"); __asm__("nop")
#else // SUPRESS_DEBUG_INFO
# define INVK_NOTIFICATION(__path__) __asm__("nop")
# define EXIT_NOTIFICATION(__path__) __asm__("nop")
#endif // SUPRESS_DEBUG_INFO

unsigned int if_file_found(std::string pathname /* in the format of ‘/file_res’ */)
{
    if (pathname.empty()) {
        return 0;
    }

    pathname.erase(pathname.begin());
    for (const auto& [key, value] : get_res_file_list())
    {
        if (key == pathname) {
            return value.file_length;
        }
    }

    return 0;
}

// Implementation of "getattr" - Get file attributes.
static int getattr_callback(const char *path, struct stat *stbuf)
{
    INVK_NOTIFICATION(path);
    memset(stbuf, 0, sizeof(struct stat));
    const auto f_len = if_file_found(path);

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
    }
    else if (f_len > 0)
    {
        stbuf->st_mode = S_IFREG | 0555;
        stbuf->st_nlink = 1;
        stbuf->st_size = f_len;
    }
    else
    {
        EXIT_NOTIFICATION(path);
        return -ENOENT;
    }

    EXIT_NOTIFICATION(path);
    return 0;
}

static int readdir_callback(const char *path,
    void *buf, fuse_fill_dir_t filler,
    off_t,
    fuse_file_info *)
{
    INVK_NOTIFICATION(path);

    if (strcmp(path, "/") != 0) {
        EXIT_NOTIFICATION(path);
        return -ENOENT;
    }

    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);

    for (unsigned int index = 0; index < resource_file_count; index++) {
        if (filler(buf, resource_file_list[index], nullptr, 0) != 0) {
            break; // Stop if buffer is full
        }
    }

    EXIT_NOTIFICATION(path);
    return 0;
}

static int access_callback(const char *path, int)
{
    INVK_NOTIFICATION(path);

    if (strcmp(path, "/") == 0)
    {
        return 0;
    }

    if (if_file_found(path) == 0) {
        EXIT_NOTIFICATION(path);
        return -ENOENT;  // File not found
    }

    EXIT_NOTIFICATION(path);
    return 0;  // Access granted
}

// Implementation of "open" - Open file.
static int open_callback(const char *path, struct fuse_file_info *fi)
{
    INVK_NOTIFICATION(path);

    if (if_file_found(path) == 0) {
        return -ENOENT;
    }

    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        EXIT_NOTIFICATION(path);
        return -EACCES;
    }

    EXIT_NOTIFICATION(path);
    return 0;
}

// Implementation of "read" - Read file content.
static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *)
{
    INVK_NOTIFICATION(path);

    std::string pathname = path;
    pathname.erase(pathname.begin());
    const auto element = get_res_file_list().find(pathname);
    if (element == get_res_file_list().end()) {
        EXIT_NOTIFICATION(path);
        return -ENOENT;
    }

    const size_t len = element->second.file_length;

    if (offset < len)
    {
        size_t read_size = 0;

        if (offset + size > len) {
            read_size = len - offset;
        }
        memcpy(buf, element->second.file_content + offset, read_size);
    }
    else
    {
        EXIT_NOTIFICATION(path);
        size = 0;
    }

    EXIT_NOTIFICATION(path);
    return static_cast<int>(size);
}

// FUSE operation definitions.
fuse_operations res_packer_fs_operations = {
    .getattr    = getattr_callback,
    .readlink   = nullptr,
    .getdir     = nullptr,
    .mknod      = nullptr,
    .mkdir      = nullptr,
    .unlink     = nullptr,
    .rmdir      = nullptr,
    .symlink    = nullptr,
    .rename     = nullptr,
    .link       = nullptr,
    .chmod      = nullptr,
    .chown      = nullptr,
    .truncate   = nullptr,
    .utime      = nullptr,
    .open       = open_callback,
    .read       = read_callback,
    .write      = nullptr,
    .statfs     = nullptr,
    .flush      = nullptr,
    .release    = nullptr,
    .fsync      = nullptr,
    .setxattr   = nullptr,
    .getxattr   = nullptr,
    .listxattr  = nullptr,
    .removexattr = nullptr,
    .opendir    = nullptr,
    .readdir    = readdir_callback,
    .releasedir = nullptr,
    .fsyncdir   = nullptr,
	.init       = nullptr,
    .destroy    = nullptr,
    .access     = access_callback,
    .create     = nullptr,
    .ftruncate  = nullptr,
    .fgetattr   = nullptr,
    .lock       = nullptr,
    .utimens    = nullptr,
    .bmap       = nullptr,
    .flag_nullpath_ok       = 0,
    .flag_nopath            = 0,
    .flag_utime_omit_ok     = 0,
    .flag_reserved          = 0,
    .ioctl      = nullptr,
    .poll       = nullptr,
	.write_buf  = nullptr,
	.read_buf   = nullptr,
    .flock      = nullptr,
    .fallocate  = nullptr,
};

void fuse_start_handler()
{
    std::error_code err;
    err.clear();
    if (!std::filesystem::create_directories(RESOURCE_PACK_TMP_DIR), err) {
        if (std::filesystem::exists(RESOURCE_PACK_TMP_DIR)) {
            err.clear();
        }

        throw sysdarft_error_t(sysdarft_error_t::FILESYSTEM_CREATE_DIRECTORIES_FAILED);
    }

    int argc;
#if (SUPRESS_DEBUG_INFO)
    const char * argv[] = { FUSE_FSNAME, "-o", "allow_other", "-f", RESOURCE_PACK_TMP_DIR };
    argc = 5;
#else // SUPRESS_DEBUG_INFO
    const char * argv[] = { FUSE_FSNAME, "-o", "allow_other", "-d", RESOURCE_PACK_TMP_DIR };
    argc = 5;
#endif // SUPRESS_DEBUG_INFO

    sysdarft_log::log(sysdarft_log::LOG_NORMAL, sysdarft_log::BOLD, sysdarft_log::CYAN,
        "[FUSE] Starting fuse_main, mount point being " RESOURCE_PACK_TMP_DIR, "\n", sysdarft_log::REGULAR);

    const int ret = fuse_main(argc, (char**)argv, &res_packer_fs_operations, nullptr);
    if (ret != 0) {
        sysdarft_log::log(sysdarft_log::LOG_ERROR, sysdarft_log::BOLD, sysdarft_log::RED,
            "[FUSE] fuse_main exited with ", ret, "\n", sysdarft_log::REGULAR);
        throw sysdarft_error_t(sysdarft_error_t::FUSE_SERVICE_FAILED_TO_START);
    }
}

void fuse_start()
{
    std::thread Thread(fuse_start_handler);
    Thread.detach();
}

void fuse_stop()
{
    sysdarft_log::log(sysdarft_log::LOG_NORMAL, sysdarft_log::BOLD, sysdarft_log::CYAN,
        "[FUSE] Ready to stop FUSE service...\n", sysdarft_log::REGULAR);

    if (std::system("sudo umount -l " RESOURCE_PACK_TMP_DIR) != 0) {
        sysdarft_log::log(sysdarft_log::LOG_ERROR, sysdarft_log::BOLD, sysdarft_log::RED,
            "[FUSE] umount failed!\n", sysdarft_log::REGULAR);
        // throw sysdarft_error_t(sysdarft_error_t::FUSE_SERVICE_FAILED_TO_STOP);
    }

    sysdarft_log::log(sysdarft_log::LOG_NORMAL, sysdarft_log::BOLD, sysdarft_log::CYAN,
        "[FUSE] fuse service is stopped.\n", sysdarft_log::REGULAR);
}

int mount_main(int argc, char ** argv)
{
    initialize_resource_filesystem();
    const int ret = fuse_main(argc, argv, &res_packer_fs_operations, nullptr);
    return ret;
}
