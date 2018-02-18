# KDE Itinerary

Itinerary and boarding pass management application.

## Building for Android

```sh
cmake -DQTANDROID_EXPORTED_TARGET=itinerary-app -DANDROID_APK_DIR=<source dir>/src/app
make
make install
make create-apk-itinerary-app
```

Additional CMake options:
- BREEZEICONS_DIR: breeze icons source dir (by default assumed next to this folder)
