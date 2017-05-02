from __future__ import with_statement

from functools import wraps
import os
import sys
import errno
import logging
from datetime import datetime

from fuse import FUSE, FuseOSError, Operations
import inspect


log = logging.getLogger(__name__)


def logged(f):
    @wraps(f)
    def wrapped(*args, **kwargs):
        log.info('%s(%s)', f.__name__, ', '.join([repr(item) for item in args[1:]]))
        return f(*args, **kwargs)
    return wrapped



class Passthrough(Operations):
    def __init__(self, root):
        self.root = root

    def destroy(self, path):
        pass

    def _full_path(self, partial):
        if partial.startswith("/"):
            partial = partial[1:]
        path = os.path.join(self.root, partial)
        return path

    def _unique_full_path(self, partial):
        if partial.startswith("/"):
            partial = partial[1:]

        now = datetime.now()
        formatted = now.strftime('%Y%m%d%H%M%S%f')
        path = os.path.join(self.root, formatted, partial)
        return path

    @logged
    def access(self, path, mode):
        full_path = self._full_path(path)
        if not os.access(full_path, mode):
            raise FuseOSError(errno.EACCES)

    @logged
    def chmod(self, path, mode):
        full_path = self._full_path(path)
        return os.chmod(full_path, mode)

    @logged
    def chown(self, path, uid, gid):
        full_path = self._full_path(path)
        return os.chown(full_path, uid, gid)

    @logged
    def getattr(self, path, fh=None):
        full_path = self._full_path(path)
        st = os.stat(full_path)
        return dict((key, getattr(st, key)) for key in ('st_atime', 'st_ctime',
               'st_gid', 'st_mode', 'st_mtime', 'st_nlink', 'st_size', 'st_uid'))

    @logged
    def readdir(self, path, fh):
        full_path = self._full_path(path)

        dirents = ['.', '..']
        if os.path.isdir(full_path):
            dirents.extend(os.listdir(full_path))
        for r in dirents:
            yield r

    @logged
    def readlink(self, path):
        pathname = os.readlink(self._full_path(path))
        if pathname.startswith("/"):
            # Path name is absolute, sanitize it.
            return os.path.relpath(pathname, self.root)
        else:
            return pathname

    @logged
    def mknod(self, path, mode, dev):
        return os.mknod(self._full_path(path), mode, dev)

    @logged
    def rmdir(self, path):
        full_path = self._full_path(path)
        return os.rmdir(full_path)

    @logged
    def mkdir(self, path, mode):
        return os.mkdir(self._full_path(path), mode)

    @logged
    def statfs(self, path):
        full_path = self._full_path(path)
        stv = os.statvfs(full_path)
        return dict((key, getattr(stv, key)) for key in ('f_bavail', 'f_bfree',
          'f_blocks', 'f_bsize', 'f_favail', 'f_ffree', 'f_files', 'f_flag',
          'f_frsize', 'f_namemax'))

    @logged
    def unlink(self, path):
        return os.unlink(self._full_path(path))

    @logged
    def symlink(self, target, name):
        return os.symlink(self._full_path(target), self._full_path(name))

    @logged
    def rename(self, old, new):
        return os.rename(self._full_path(old), self._full_path(new))

    @logged
    def link(self, target, name):
        return os.link(self._full_path(target), self._full_path(name))

    @logged
    def utimens(self, path, times=None):
        return os.utime(self._full_path(path), times)

    @logged
    def open(self, path, flags):
        full_path = self._full_path(path)
        return os.open(full_path, flags)

    @logged
    def create(self, path, mode, fi=None):
        full_path = self._full_path(path)
        now = datetime.now().strftime('%Y%m%d%H%M%S%f')
        z = os.path.basename(path)
        x = os.path.join(self.root, now)
        y = os.path.join(now, z)
        w = os.path.join(self.root, now, z)

        print(x, y, z, w)
        os.makedirs(x)
        os.symlink(y, full_path)
        return os.open(w, os.O_WRONLY | os.O_CREAT, mode)

    @logged
    def read(self, path, length, offset, fh):
        os.lseek(fh, offset, os.SEEK_SET)
        return os.read(fh, length)

    @logged
    def write(self, path, buf, offset, fh):
        os.lseek(fh, offset, os.SEEK_SET)
        return os.write(fh, buf)

    @logged
    def truncate(self, path, length, fh=None):
        full_path = self._full_path(path)
        with open(full_path, 'r+') as f:
            f.truncate(length)

    @logged
    def flush(self, path, fh):
        return os.fsync(fh)

    @logged
    def release(self, path, fh):
        return os.close(fh)

    @logged
    def fsync(self, path, fdatasync, fh):
        return self.flush(path, fh)

def main(mountpoint, root):
    logging.basicConfig(level=logging.INFO)

    FUSE(Passthrough(root), mountpoint, foreground=True)

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Start the FUSE plugin with a target directory and mount point.')
    parser.add_argument('target', help='The target directory to view at the mount point.')
    parser.add_argument('mount', help='The mount point to view the target directory.')
    args = parser.parse_args()
    main(args.mount, args.target)
