Summary: Hardware Dossier Tool
Name: dossier
Version: 1.0
Release: 111
Group: System Environment/Tools
License: MIT
Vendor: Karl N. Redgate
Packager: Karl N. Redgate <Karl.Redgate@gmail.com>
%define _topdir %(echo $PWD)/rpm
BuildRoot: %{_topdir}/BUILDROOT
%define Exports %(echo $PWD)/exports

%description
Tools for gathering hardware and BIOS information.

%prep
%build

%install
tar -C %{Exports} -cf - . | (cd $RPM_BUILD_ROOT; tar xf -)

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,0755)
/usr/share/man
/usr/share/dossier
/usr/sbin/dossier

%post
[ "$1" = 1 ] && {
    : New install
}

[ "$1" -gt 1 ] && {
    : Upgrading
}

%changelog

* Tue Sep 20 2011 Karl N. Redgate <kredgate.github.com>
- Initial build

# vim:autoindent expandtab sw=4
