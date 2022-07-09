# KDE Itinerary

Itinerary and boarding pass management application.

## Using KDE Itinerary

### Pre-built packages

#### Flatpak

Stable flatpaks are available from [Flathub](https://flathub.org/apps/details/org.kde.itinerary).

Nightly flatpaks are available from KDE's Flatpak repository (`flatpak remote-add --if-not-exists kdeapps --from https://distribute.kde.org/kdeapps.flatpakrepo`)

#### Android

Nightly builds are available from KDE's nightly F-Droid Repository https://cdn.kde.org/android/fdroid/repo/?fingerprint=B3EBE10AFA6C5C400379B34473E843D686C61AE6AD33F423C98AF903F056523F

![Link to KDE's nightly build F-Droid repository](nightly-build-fdroid-repo-link.png)

Release builds are available from KDE's release F-Droid Repository
https://cdn.kde.org/android/stable-releases/fdroid/repo/?fingerprint=13784BA6C80FF4E2181E55C56F961EED5844CEA16870D3B38D58780B85E1158F

Alternatively, you can download apks directly from the Binary Factory:
 - Nightly builds for [arm64](https://binary-factory.kde.org/view/Android/job/Itinerary_Nightly_android-arm64/), [arm](https://binary-factory.kde.org/view/Android/job/Itinerary_Nightly_android-arm/), [x86_64](https://binary-factory.kde.org/view/Android/job/Itinerary_Nightly_android-x86_64/) and [x86](https://binary-factory.kde.org/view/Android/job/Itinerary_Nightly_android-x86/)
  - Release builds for [arm64](https://binary-factory.kde.org/view/Android/job/Itinerary_Release_android-arm64/), [arm](https://binary-factory.kde.org/view/Android/job/Itinerary_Release_android-arm/), [x86_64](https://binary-factory.kde.org/view/Android/job/Itinerary_Release_android-x86_64/) and [x86](https://binary-factory.kde.org/view/Android/job/Itinerary_Release_android-x86/)

The version in the Play Store will currently not be updated due to Play Store policies.

### Where do I get data from?

- On Android: via the system calendar, if you use DavDroid to sync to a calendar that has
  events with reservation data created by KMail.
- Manually importing Apple Wallet pass or JSON-LD files.
- By sending data created by KMail to your phone via KDE Connect. This requires a KMail plugin, which is part of [kdepim-addons](https://invent.kde.org/pim/kdepim-addons).


## Contributing

### Building for Android, using Docker

An existing docker image exists with everything set up for compilation to Android: kdeorg/android-sdk.
The following command will compile itinerary with all its dependencies and output an apk to our /tmp directory:

```
docker run -ti --rm -v /tmp:/output kdeorg/android-sdk /opt/helpers/build-generic itinerary
```

### Building for Android, by hand

```
cmake -DQTANDROID_EXPORTED_TARGET=itinerary-app -DANDROID_APK_DIR=<source dir>/src/app  
make  
make install  
make create-apk-itinerary-app  
```

Additional CMake options:

- BREEZEICONS_DIR: breeze icons source dir (by default assumed next to this folder)

If you are using kdesrc-build, the following configuration snippet can be useful to obtain
all external dependencies (on top of the usual KF5 config):
```
module libintl-lite
    repository https://github.com/j-jorge/libintl-lite.git
endmodule

module libical
    repository https://github.com/libical/libical
    branch 2.0
    cmake-options -DICAL_BUILD_DOCS=OFF -DICAL_GLIB=OFF
endmodule

module libqrencode
    repository https://github.com/fukuchi/libqrencode.git
    cmake-options -DWITH_TOOLS=OFF
endmodule

options ki18n
    cmake-options -DBUILD_WITH_QTSCRIPT=OFF
end options

options itinerary
    cmake-options -DQTANDROID_EXPORTED_TARGET=itinerary-app
    make-options create-apk-itinerary-app
end options
```
You will also need OpenSSL in a version matching what your Qt was built against, which unfortunately
is a bit more cumbersome to build, see https://wiki.openssl.org/index.php/Android.

### Building for all other platforms

Works too of course, just the usual cmake/make/make install.
