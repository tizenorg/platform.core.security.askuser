Name:       askuser
Summary:    Agent service for Cynara 'ask user' policy
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
Summary: aaaaaaaaaaaaaaaaaaaaaaaaaaaa

%description -n askuser-notification
ghhtrh erth rio thjiortjh ioetrj hioe trio tu

%package -n askuser-test
Summary: aaaaaaaaaaaaaaaaaaaaaaaaaaaa
BuildRequires: pkgconfig(cynara-admin)

%description -n askuser-test
ghhtrh erth rio thjiortjh ioetrj hioe trio tu

%package -n askuser-plugins
Requires:   cynara
Requires:   libcynara-client
Summary:    Askuser commons library

%description -n askuser-plugins
askuser plugin library with cynara service and client side plugins

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

%post
# todo properly use systemd --user
ln -s /lib/systemd/user/askuser-notification.service \
/usr/lib/systemd/user/default.target.wants/askuser-notification.service

systemctl daemon-reload

if [ $1 = 1 ]; then
    systemctl enable askuser.service
fi

systemctl restart askuser.service

%files
%attr(755, root, root) /usr/bin/askuser
/usr/lib/systemd/system/askuser.service

%files -n askuser-notification
%attr(755,root,root) /usr/bin/askuser-notification
/usr/lib/systemd/user/askuser-notification.service

%files -n askuser-test
%attr(755,root,root) /usr/bin/askuser-test

%files -n askuser-plugins
%{_libdir}/cynara/plugin/client/*
%{_libdir}/cynara/plugin/service/*
