Name:       askuser2
Summary:    askuser services
Version:    0.2.0
Release:    1
Group:      Security/Access Control
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: libwayland-egl
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(cynara-agent)
BuildRequires: pkgconfig(cynara-creds-socket)
BuildRequires: pkgconfig(cynara-plugin)
%{?systemd_requires}

%if !%{defined build_type}
%define build_type RELEASE
%endif

%description
Daemon allowing user to grant or deny acces for given application and privilege

%package -n askuser-notification
Summary: aaaaaaaaaaaaaaaaaaaaaaaaaaaa

%description -n askuser-notification
ghhtrh erth rio thjiortjh ioetrj hioe trio tu

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

%if %{?build_type} == "DEBUG"
export CXXFLAGS="$CXXFLAGS -Wp,-U_FORTIFY_SOURCE"
%endif

export LDFLAGS+="-Wl,--rpath=%{_libdir}"

%cmake . \
        -DCMAKE_BUILD_TYPE=%{?build_type} \
        -DCMAKE_VERBOSE_MAKEFILE=ON
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
# %find_lang %{name}

%files
%attr(755, root, root) /usr/bin/askuserd

%files -n askuser-notification
%attr(755,root,root) /usr/bin/askuser-notification
