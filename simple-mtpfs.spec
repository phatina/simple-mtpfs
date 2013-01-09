Name:          simple-mtpfs
Version:       0.1
Release:       5%{?dist}
Summary:       Fuse-based MTP driver
License:       GPLv2+
URL:           https://github.com/phatina/simple-mtpfs
Source0:       https://github.com/downloads/phatina/simple-mtpfs/%{name}-%{version}.tar.gz

BuildRequires: fuse-devel >= 2.8
BuildRequires: libmtp-devel

%description
SIMPLE-MTPFS (Simple Media Transfer Protocol FileSystem) is a file system for
Linux (and other operating systems with a FUSE implementation, such as Mac OS X
or FreeBSD) capable of operating on files on MTP devices attached via USB to
local machine. On the local computer where the SIMPLE-MTPFS is mounted, the
implementation makes use of the FUSE (Filesystem in Userspace) kernel module.
The practical effect of this is that the end user can seamlessly interact with
MTP device files.

%prep
%setup -q -n %{name}-%{version}

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT

%files
%doc COPYING README
%{_bindir}/simple-mtpfs
%{_mandir}/man1/simple-mtpfs.1.gz

%changelog
* Tue Oct 23 2012 Peter Hatina <phatina@redhat.com> - 0.1-5
- changed license to GPLv2

* Sun Oct 07 2012 Peter Hatina <phatina@redhat.com> - 0.1-4
- removed defattr

* Sat Oct 06 2012 Peter Hatina <phatina@redhat.com> - 0.1-3
- removed gcc dependency

* Fri Oct 05 2012 Peter Hatina <phatina@redhat.com> - 0.1-2
- removed autoconf dependency

* Wed Oct 03 2012 Peter Hatina <phatina@redhat.com> - 0.1-1
- initial import
