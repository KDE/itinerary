# KDE Itinerary

Itinerary and boarding pass management application.

## Using KDE Itinerary

### Where do I get data from?

- On Android: via the system calendar, if you use DavDroid to sync to a calendar that has
  events with reservation data created by KMail.
- Manually importing Apple Wallet pass or JSON-LD files.
- By sending data created by KMail to your phone via KDE Connect


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
