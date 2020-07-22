Name:          simple-mtpfs
Version:       0.4.0
Release:       1%{?dist}
Summary:       Fuse-based MTP driver
License:       GPLv2+
URL:           https://github.com/phatina/simple-mtpfs
Source0:       https://github.com/phatina/simple-mtpfs/archive/v%{version}.tar.gz

BuildRequires: fuse-devel >= 2.7.3
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
%doc COPYING README.md
%{_bindir}/simple-mtpfs
%{_mandir}/man1/simple-mtpfs.1.gz

%changelog
* Wed Jul 22 2020 Peter Hatina <peter@hatina.eu> - 0.4.0-1
- upgrade to v0.4.0

* Sun Dec 25 2016 Peter Hatina <phatina@gmail.com> - 0.3.0-1
- upgrade to v0.3.0

* Sun Nov 27 2016 Peter Hatina <phatina@gmail.com> - 0.2.1-1
- upgrade to v0.2.1

* Tue Dec  3 2013 Peter Hatina <phatina@redhat.com> - 0.2-1
- upgrade to v0.2

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
