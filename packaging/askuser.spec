%if !%{defined with_systemd_daemon}
%define with_systemd_daemon 0
%endif

Name:       askuser
Summary:    Agent service for Cynara 'ask user' policy
Version:    0.1.4
Release:    1
Group:      Security/Access Control
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001:    %{name}.manifest
Source1002:    libaskuser-common.manifest
Source1003:    askuser-plugins.manifest
Source1004:    askuser-test.manifest
Source1005:    askuser-notification.manifest
BuildRequires: cmake
BuildRequires: libwayland-egl
BuildRequires: gettext-tools
BuildRequires: pkgconfig(cynara-agent)
BuildRequires: pkgconfig(cynara-creds-socket)
BuildRequires: pkgconfig(cynara-plugin)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(libsystemd)
BuildRequires: pkgconfig(security-manager)
BuildRequires: pkgconfig(security-privilege-manager)
BuildRequires: coregl

%if !%{defined build_type}
%define build_type RELEASE
%endif

%description
Daemon allowing user to grant or deny acces for given application and privilege

%package -n libaskuser-common
Summary:    Askuser common library

%description -n libaskuser-common
Askuser common library with common functionalities

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

%package -n askuser-test
BuildRequires: pkgconfig(cynara-admin)
BuildRequires: pkgconfig(gmock)
Summary: Tool for testing askuser packages and unit tests for askuser

%description -n askuser-test
Tool for testing askuser packages and unit tests for askuser

%prep
%setup -q
cp -a %{SOURCE1001} .
cp -a %{SOURCE1002} .
cp -a %{SOURCE1003} .
cp -a %{SOURCE1004} .
cp -a %{SOURCE1005} .

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
        -DBUILD_WITH_SYSTEMD_DAEMON=%{?with_systemd_daemon} \
        -DBUILD_WITH_SYSTEMD_JOURNAL=ON \
        -DCMAKE_VERBOSE_MAKEFILE=ON \
        -DLIB_DIR:PATH=%{_libdir} \
        -DBIN_DIR:PATH=%{_bindir} \
        -DSYSTEMD_UNIT_DIR:PATH=%{_unitdir} \
        -DSYSTEMD_USER_UNIT_DIR:PATH=%{_unitdir_user}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
%find_lang %{name}

%post
%if %{with_systemd_daemon}
# todo properly use systemd --user
ln -s /lib/systemd/user/askuser-notification.service \
/usr/lib/systemd/user/default.target.wants/askuser-notification.service 2> /dev/null

systemctl daemon-reload

if [ $1 = 1 ]; then
    systemctl enable askuser.service
fi

systemctl restart askuser.service
%endif

systemctl restart cynara.service

%preun
%if %{with_systemd_daemon}
if [ $1 = 0 ]; then
    systemctl stop askuser.service
fi
%endif

%postun
%if %{with_systemd_daemon}
if [ $1 = 0 ]; then
    systemctl daemon-reload
fi
%endif

systemctl restart cynara.service

%post -n libaskuser-common -p /sbin/ldconfig

%postun -n libaskuser-common -p /sbin/ldconfig

%files -f %{name}.lang
%manifest %{name}.manifest
%license LICENSE
%attr(755, root, root) /usr/bin/askuser
%if %{with_systemd_daemon}
%{_unitdir}/askuser.service
%endif

%files -n askuser-notification
%manifest askuser-notification.manifest
%license LICENSE
%attr(755,root,root) /usr/bin/askuser-notification
%if %{with_systemd_daemon}
%{_unitdir_user}/askuser-notification.service
%endif
/usr/share/locale/en/LC_MESSAGES/askuser.mo
/usr/share/locale/pl/LC_MESSAGES/askuser.mo

%files -n libaskuser-common
%manifest libaskuser-common.manifest
%license LICENSE
%{_libdir}/libaskuser-common.so*

%files -n askuser-plugins
%manifest askuser-plugins.manifest
%license LICENSE
%{_libdir}/cynara/plugin/client/*
%{_libdir}/cynara/plugin/service/*

%files -n askuser-test
%manifest askuser-test.manifest
%license LICENSE
%attr(755,root,root) %{_bindir}/askuser-test
%attr(755,root,root) %{_bindir}/askuser-tests

