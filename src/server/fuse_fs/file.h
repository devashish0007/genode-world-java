/*
 * \brief  File node
 * \author Norman Feske
 * \author Christian Helmuth
 * \author Josef Soentgen
 * \date   2013-11-26
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _FILE_H_
#define _FILE_H_

/* local includes */
#include <mode_util.h>
#include <node.h>

#include <fuse.h>
#include <fuse_private.h>

namespace Fuse_fs {
	class File;
}


class Fuse_fs::File : public Node
{
	private:

		Node *_parent;

		typedef Genode::Path<MAX_PATH_LEN> Path;
		Path                   _path;

		struct fuse_file_info  _file_info;

		void _open_path(char const *path, Mode mode, bool create, bool trunc)
		{
			int res;
			int tries = 0;
			do {
				/* first try to open pathname */
				Libc::with_libc([&] () {
					res = Fuse::fuse()->op.open(path, &_file_info);
				});
				if (res == 0) {
					break;
				}

				/* try to create pathname if open failed and create is true */
				if (create && !tries) {
					mode_t mode = S_IFREG | 0644;
					int res = -1;
					Libc::with_libc([&] () {
						res = Fuse::fuse()->op.mknod(path, mode, 0);
					});
					switch (res) {
						case 0:
							break;
						default:
							Genode::error("could not create '", path, "'");
							throw Lookup_failed();
					}

					tries++;
					continue;
				}

				if (res < 0) {
					throw Lookup_failed();
				}
			}
			while (true);

			if (trunc) {
				Libc::with_libc([&] () {
					res = Fuse::fuse()->op.ftruncate(path, 0, &_file_info);
				});

				if (res != 0) {
					Libc::with_libc([&] () {
						Fuse::fuse()->op.release(path, &_file_info);
					});
					throw Lookup_failed();
				}
			}
		}

		size_t _length()
		{
			struct stat s;
			int res = -1;
			Libc::with_libc([&] () {
				res = Fuse::fuse()->op.getattr(_path.base(), &s);
			});
			if (res != 0)
				return 0;

			return s.st_size;
		}

	public:

		File(Node *parent, char const *name, Mode mode,
		     bool create = false, bool trunc = false)
		:
			Node(name),
			_parent(parent),
			_path(name, _parent->name())
		{
			_open_path(_path.base(), mode, create, trunc);
		}

		~File()
		{
			Libc::with_libc([&] () {
				Fuse::fuse()->op.release(_path.base(), &_file_info);
			});
		}

		struct fuse_file_info *file_info() { return &_file_info; }

		Status status() override
		{
			struct stat s;
			int res = -1;
			Libc::with_libc([&] () {
				res = Fuse::fuse()->op.getattr(_path.base(), &s);
			});
			if (res != 0)
				return Status();

			Status status;
			status.inode = s.st_ino ? s.st_ino : 1;
			status.size = s.st_size;
			status.type = File_system::Node_type::CONTINUOUS_FILE;
			return status;
		}

		size_t read(char *dst, size_t len, seek_off_t seek_offset) override
		{
			/* append mode, use actual length as offset */
			if (seek_offset == ~0ULL)
				seek_offset = _length();

			int ret = -1;
			Libc::with_libc([&] () {
				ret = Fuse::fuse()->op.read(_path.base(), dst, len,
				                            seek_offset, &_file_info);
			});
			return ret < 0 ? 0 : ret;
		}

		size_t write(char const *src, size_t len, seek_off_t seek_offset) override
		{
			/* append mode, use actual length as offset */
			if (seek_offset == ~0ULL)
				seek_offset = _length();

			int ret = -1;
			Libc::with_libc([&] () {
				ret = Fuse::fuse()->op.write(_path.base(), src, len,
				                             seek_offset, &_file_info);
			});
			return ret < 0 ? 0 : ret;
		}

		void truncate(file_size_t size) override
		{
			int res = -1;
			Libc::with_libc([&] () {
				res = Fuse::fuse()->op.ftruncate(_path.base(), size,
				                                 &_file_info);
				});
			if (res == 0)
				mark_as_updated();
		}
};

#endif /* _FILE_H_ */
