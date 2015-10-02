%bcond_with wayland
%bcond_with x

%define extension_path %{_libdir}/tizen-extensions-crosswalk
%define injected_bundle_path %{_libdir}/libxwalk_injected_bundle.so

Name:       crosswalk-tizen
Summary:    Crosswalk Runtime and AppShell for Tizen
Version:    1.0.0
Release:    1
Group:      Development/Libraries
License:    Apache-2.0 and BSD-3-Clause
URL:        https://www.tizen.org
Source0:    %{name}-%{version}.tar.gz
Source1001: %{name}.manifest

################ disable builds in X11 repos ###############
# currently, crosswalk-tizen is not needed on X11 profiles
# see TINF-965
%if %{with x}
ExclusiveArch:
%endif
############################################################

BuildRequires: boost-devel
BuildRequires: edje-tools
BuildRequires: gettext
BuildRequires: ninja
BuildRequires: python
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(chromium-efl)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(ecore-wayland)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(libwebappenc)
BuildRequires: pkgconfig(manifest-handlers)
BuildRequires: pkgconfig(manifest-parser)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(uuid)

%description
Crosswalk Runtime and AppShell for Tizen 3.0 and later

%prep
%setup -q

cp %{SOURCE1001} .

%build
export GYP_GENERATORS='ninja'
GYP_OPTIONS="--depth=."

# BuildType: Debug / Release
%if 0%{?tizen_build_devel_mode}
GYP_OPTIONS="$GYP_OPTIONS -Dbuild_type=Debug"
%else
GYP_OPTIONS="$GYP_OPTIONS -Dbuild_type=Release"
%endif

# Extension Path
GYP_OPTIONS="$GYP_OPTIONS -Dextension_path=%{extension_path}"

# Injected bundle
GYP_OPTIONS="$GYP_OPTIONS -Dinjected_bundle_path=%{injected_bundle_path}"

# Build
./tools/gyp/gyp $GYP_OPTIONS xwalk_tizen.gyp
ninja -C out/Default %{?_smp_mflags}

%install

# Prepare directories
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/license
mkdir -p %{buildroot}%{_datadir}/locale
mkdir -p %{buildroot}%{_datadir}/edje/xwalk
mkdir -p %{buildroot}%{extension_path}

# License files
cp LICENSE %{buildroot}%{_datadir}/license/%{name}
cat LICENSE.BSD >> %{buildroot}%{_datadir}/license/%{name}

# xwalk_common
install -p -m 644 out/Default/lib/libxwalk_tizen_common.so %{buildroot}%{_libdir}

# xwalk_extension
install -p -m 755 out/Default/xwalk_extension %{buildroot}%{_bindir}

# widget_plugin
install -p -m 644 out/Default/lib/libwidget_plugin.so %{buildroot}%{extension_path}
install -p -m 644 out/Default/gen/widget.json %{buildroot}%{extension_path}

# xwalk_runtime
install -p -m 755 out/Default/xwalk_runtime %{buildroot}%{_bindir}
ln -s %{_bindir}/xwalk_runtime %{buildroot}%{_bindir}/wrt

# xwalk_runtime_resources
for file in $(find out/Default/gen/locales -type f -name *.mo); do
  install -m 644 -D $file %{buildroot}%{_datadir}/locale/${file#out/Default/gen/locales/}
done
install -p -m 644 out/Default/gen/*.edj %{buildroot}%{_datadir}/edje/xwalk

# xwalk_injected_bundle
install -p -m 755 out/Default/lib/libxwalk_injected_bundle.so %{buildroot}%{_libdir}

%clean
rm -fr %{buildroot}

%files
%manifest %{name}.manifest
%attr(644,root,root) %{_datadir}/license/%{name}
%attr(755,root,root) %{_datadir}/locale/*
%attr(644,root,root) %{_datadir}/edje/xwalk/*.edj
%attr(644,root,root) %{_libdir}/libxwalk_tizen_common.so
%attr(644,root,root) %{_libdir}/libxwalk_injected_bundle.so
%attr(644,root,root) %{extension_path}/libwidget_plugin.so
%attr(644,root,root) %{extension_path}/widget.json
%attr(755,root,root) %{_bindir}/xwalk_extension
%attr(755,root,root) %{_bindir}/xwalk_runtime
%attr(755,root,root) %{_bindir}/wrt
