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
Source1002: wrt.loader

BuildRequires: boost-devel
BuildRequires: edje-tools
BuildRequires: gettext
BuildRequires: ninja
BuildRequires: python
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-ui-efl-util)
BuildRequires: pkgconfig(v8)
BuildRequires: pkgconfig(chromium-efl)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(ecore-wayland)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libwebappenc)
BuildRequires: pkgconfig(wgt-manifest-handlers)
BuildRequires: pkgconfig(manifest-parser)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(uuid)
BuildRequires: pkgconfig(launchpad)
BuildRequires: pkgconfig(ttrace)
BuildRequires: pkgconfig(jsoncpp)

%if "%{?profile}" == "mobile"
%define tizen_feature_rotary_event_support     0
%define tizen_feature_web_ime_support          0
%define tizen_feature_watch_face_support       0
%endif

%if "%{?profile}" == "wearable"
%define tizen_feature_rotary_event_support     1
%define tizen_feature_web_ime_support          1
%define tizen_feature_watch_face_support       1
%endif

%if "%{?profile}" == "tv"
%define tizen_feature_rotary_event_support     0
%define tizen_feature_web_ime_support          1
%define tizen_feature_watch_face_support       0
%endif

%if 0%{?tizen_feature_web_ime_support}
BuildRequires: pkgconfig(capi-ui-inputmethod)
%endif

%if 0%{?tizen_feature_watch_face_support}
BuildRequires: pkgconfig(capi-appfw-watch-application)
BuildRequires: pkgconfig(appcore-watch)
%endif


Requires: /usr/bin/systemctl

%description
Crosswalk Runtime and AppShell for Tizen 3.0 and later

%prep
%setup -q

cp %{SOURCE1001} .

%build
export GYP_GENERATORS='ninja'
GYP_OPTIONS="--depth=.
-Dprofile=%{profile}"

# BuildType: Debug / Release
%if 0%{?tizen_build_devel_mode}
GYP_OPTIONS="$GYP_OPTIONS -Dbuild_type=Debug"
%else
GYP_OPTIONS="$GYP_OPTIONS -Dbuild_type=Release"
%endif

# Feature flags
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_rotary_event_support=%{?tizen_feature_rotary_event_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_web_ime_support=%{?tizen_feature_web_ime_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_watch_face_support=%{?tizen_feature_watch_face_support}"

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

# Loader script file
mkdir -p %{buildroot}%{_datadir}/aul/
cp %{SOURCE1002} %{buildroot}%{_datadir}/aul/

# xwalk_common
install -p -m 644 out/Default/lib/libxwalk_tizen_common.so %{buildroot}%{_libdir}

# widget_plugin
install -p -m 644 out/Default/lib/libwidget_plugin.so %{buildroot}%{extension_path}
install -p -m 644 out/Default/gen/widget.json %{buildroot}%{extension_path}

# screen_plugin
install -p -m 644 out/Default/lib/libsplash_screen_plugin.so %{buildroot}%{extension_path}
install -p -m 644 out/Default/gen/splash_screen.json %{buildroot}%{extension_path}

# xwalk_runtime
install -p -m 755 out/Default/xwalk_runtime %{buildroot}%{_bindir}
ln -s %{_bindir}/xwalk_runtime %{buildroot}%{_bindir}/wrt
ln -s %{_bindir}/xwalk_runtime %{buildroot}%{_bindir}/wrt-loader

# xwalk extension shared
install -p -m 644 out/Default/lib/libxwalk_extension_shared.so %{buildroot}%{_libdir}

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
%attr(644,root,root) %{_libdir}/libxwalk_extension_shared.so
%attr(644,root,root) %{extension_path}/libwidget_plugin.so
%attr(644,root,root) %{extension_path}/widget.json
%attr(644,root,root) %{extension_path}/libsplash_screen_plugin.so
%attr(644,root,root) %{extension_path}/splash_screen.json
%attr(755,root,root) %{_bindir}/xwalk_runtime
%attr(755,root,root) %{_bindir}/wrt
%attr(755,root,root) %{_bindir}/wrt-loader
%attr(644,root,root) %{_datadir}/aul/wrt.loader
%caps(cap_mac_admin,cap_mac_override,cap_setgid=ei) %{_bindir}/xwalk_runtime
