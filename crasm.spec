Name: crasm
Summary: Cross assembler for 6800/6801/6803/6502/65C02/Z80.
Version: 1.3
Release: 1
License: GPL
Group: Applications/Internet
URL: http://crasm.sourceforge.net/
Source0: %{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description 
Crasm is a cross assembler for 6800/6801/6803/6502/65C02/Z80. 
It produces binaries in Intel HEX or Motorola S Code.
%prep 
%setup -q

%build 
make

%install 
rm -fr %{buildroot}
install -d %{buildroot}%{_bindir}
install -d %{buildroot}%{_mandir}/man1
%makeinstall

%clean 
rm -fr %{buildroot}

%files 
%defattr(-,root,root) 
%{_bindir}/* 
%{_mandir}/* 
%doc crasm.html
%doc test

%changelog 
