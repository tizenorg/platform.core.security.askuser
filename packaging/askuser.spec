Name:       askuser
Summary:    Agent service for Cynara 'ask user' policy
Version:    0.2.0
Release:    1
Group:      Security/Access Control
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001:    %{name}.manifest
Source1002:    askuser-notification.manifest
Source1003:    askuser-plugins.manifest
BuildRequires: cmake
BuildRequires: libwayland-egl
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(cynara-agent)
BuildRequires: pkgconfig(cynara-creds-socket)
BuildRequires: pkgconfig(cynara-plugin)
BuildRequires: pkgconfig(libsystemd-journal)
BuildRequires: pkgconfig(security-privilege-manager)
BuildRequires: pkgconfig(security-manager)
%{?systemd_requires}

%if !%{defined build_type}
%define build_type RELEASE
%endif

%description
Daemon allowing user to grant or deny acces for given application and privilege

%package -n askuser-notification
Summary: User daemon which shows popup with privilege request

%description -n askuser-notification
User daemon which shows popup with privilege request

%package -n askuser-plugins
Requires:   cynara
Requires:   libcynara-client
Summary:    Askuser cynara plugins

%description -n askuser-plugins
Askuser plugin library with cynara service and client side plugins

%prep
%setup -q
cp -a %{SOURCE1001} .
cp -a %{SOURCE1002} .
cp -a %{SOURCE1003} .

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

%files
%manifest %{name}.manifest
%license LICENSE
%attr(755, root, root) /usr/bin/askuser

%files -n askuser-notification
%manifest askuser-notification.manifest
%license LICENSE
%attr(755,root,root) /usr/bin/askuser-notification
