ABOUT
=====

SIMPLE-MTPFS (Simple Media Transfer Protocol FileSystem) is a file system for
Linux (and other operating systems with a FUSE implementation, such as Mac OS X
or FreeBSD) capable of operating on files on MTP devices attached via USB to
local machine. On the local computer where the SIMPLE-MTPFS is mounted, the
implementation makes use of the FUSE (Filesystem in Userspace) kernel module.
The practical effect of this is that the end user can seamlessly interact with
MTP device files.

LATEST VERSION
==============

Latest sources of the software can be found at: [simple-mtpfs][]

INSTALLATION
============

Simple-mtpfs depends on fuse (version >= **2.7.3**) and libmtp. It also
requires the C++ compiler to support **C++11** standard.

To install the driver, follow these steps:

    $ mkdir build && cd build
    $ ../configure
    $ make
    $ make install (as root)

Due to MTP nature, it is necessary to use a folder, where the temporary files
downloaded will be downloaded. The project can be configured to use custom
directory for such files. To configure the simple-mtpfs to use desired
temporary directory, add `--with-tmpdir=TMPDIR` option to configure script.
Default value for temporary directory is `/tmp`.

If you got the sources from git repository, first you have to run:

    $ ./autogen.sh

MOUNTING
========

To mount MTP-based device to your local filesystem, simply run:

    $ simple-mtpfs mountpoint [options]

If you have more than one MTP device attached to the computer, it is possible
to specify which device, you are willing to mount. Either by entering its **order
number** or special file usually placed in `/dev`:

MOUNTING BY NUMBER
------------------

    $ simple-mtpfs --device <number> mountpoint [options]

Where the `<number>` should contain a numeric order of the device, you are
about to mount. To get a list of all attached devices, execute following:

    $ simple-mtpfs --list-devices
    <number>: <device name>
    <number>: <device name>
    ...

MOUNTING BY SPECIAL FILE
------------------------

Enter special device file as the first argument to simple-mtpfs. The special device
file is usually named as `/dev/libmtp-*`.

    $ simple-mtpfs <device> mountpoint [options]

UNMOUNTING
==========

To unmount MTP device, execute following command:

    $ fusermount -u <mountpoint>

BUG REPORTS
===========

Report bugs to [phatina@gmail.com](mailto:phatina@gmail.com) or
[simple-mtpfs issues][].

[simple-mtpfs]: https://github.com/phatina/simple-mtpfs "simple-mtpfs repository on github"
[simple-mtpfs issues]: https://github.com/phatina/simple-mtpfs/issues "Report a bug"
