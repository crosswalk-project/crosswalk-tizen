%bcond_with wayland
%bcond_with x

Name:       wrt
Summary:    Runtime for Web Application
Version:    2.0.0
Release:    1
Group:      Development/Libraries
License:    Apache License, Version 2.0 / BSD
URL:        N/A
Source0:    %{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(efl-assist)
BuildRequires: pkgconfig(deviced)
%if %{with x}
BuildRequires: pkgconfig(ecore-x)
%endif
%if %{with wayland}
BuildRequires: pkgconfig(ecore-wayland)
%endif

#web-engine
BuildRequires:  pkgconfig(chromium-efl)

%description
Runtime for Web Application

%prep
%setup -q

%build

%ifarch %{arm}
%define build_dir build-arm
%else
%define build_dir build-x86
%endif

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

cmake .. \
  -DCMAKE_BUILD_TYPE=%{?build_type:%build_type} \
  -DX11_SUPPORT=%{enable_x11} \
  -DWAYLAND_SUPPORT=%{enable_wayland}

make %{?jobs:-j%jobs}

%install
cd %{build_dir}
%make_install

%clean
rm -fr %{buildroot}

%files
/usr/local/bin/wrt

