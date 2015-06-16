%bcond_with wayland
%bcond_with x

Name:       wrt
Summary:    Runtime for Web Application
Version:    2.0.0
Release:    1
Group:      Development/Libraries
License:    Apache-2.0 and BSD-3-Clause
URL:        https://www.tizen.org
Source0:    %{name}-%{version}.tar.gz

################ disable builds in X11 repos ###############
# currently (june 2015), nwrt is not needed on X11 profiles
# see TINF-965
%if %{with x}
ExclusiveArch:
%endif
############################################################

BuildRequires: cmake
BuildRequires: edje-tools
BuildRequires: gettext
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(manifest-parser)
BuildRequires: pkgconfig(manifest-handlers)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(efl-assist)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(cert-svc)
BuildRequires: pkgconfig(uuid)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(notification)
BuildRequires: boost-devel
BuildRequires: python
%if %{with x}
BuildRequires: pkgconfig(ecore-x)
%endif
%if %{with wayland}
BuildRequires: pkgconfig(ecore-wayland)
%endif

#web-engine
BuildRequires: pkgconfig(chromium-efl)

%description
Runtime for Web Application

%prep
%setup -q

%define extension_path %{_libdir}/tizen-extensions-crosswalk
%define injected_bundle_path %{_libdir}/libwrt-injected-bundle.so

%build

%define build_dir cmake_build_dir

%if %{with x}
%define enable_x11 On
%else
%define enable_x11 Off
%endif

%if %{with wayland}
%define enable_wayland On
%else
%define enable_wayland Off
%endif

mkdir -p %{build_dir}
cd %{build_dir}

cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix} \
         -DLIB_INSTALL_DIR=%{_libdir} \
         -DCMAKE_BUILD_TYPE=%{?build_type:%build_type} \
         -DX11_SUPPORT=%{enable_x11} \
         -DWAYLAND_SUPPORT=%{enable_wayland} \
         -DEXTENSION_PATH=%{extension_path} \
         -DINJECTED_BUNDLE_PATH=${injected_bundle_path}

make %{?jobs:-j%jobs}

%install
%define license_dir %{build_dir}%{_datadir}/license
mkdir -p %{license_dir}
cp LICENSE %{license_dir}/%{name}
cat LICENSE.BSD >> %{license_dir}/%{name}
cd %{build_dir}
%make_install

%clean
rm -fr %{buildroot}

%files
%attr(755,root,root) %{_bindir}/wrt
%attr(755,root,root) %{_bindir}/wrt-popup-test
%attr(644,root,root) %{_datadir}/edje/wrt/wrt.edj
%attr(644,root,root) %{injected_bundle_path}
%attr(644,root,root) %{extension_path}/libwidget-plugin.so
%attr(755,root,root) %{_datadir}/locale/*
