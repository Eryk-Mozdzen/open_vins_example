#!/bin/bash
set -e

# --- CONFIG ---
NDK_DIR="$HOME/android-sdk/ndk/25.2.9519653"
SDK_DIR="$HOME/android-sdk"
API_LEVEL=30
KEYSTORE="$HOME/mykey.jks"          # adjust to your keystore
KEY_ALIAS="mykeyalias"              # adjust to your alias
LIB_NAME="libapp.so"
BUILD_DIR="build"
STAGING_DIR="staging"
RES_DIR="res"
MANIFEST="AndroidManifest.xml"
UNALIGNED_APK="base-unaligned.apk"
ALIGNED_APK="app.apk"
SIGNED_APK="app-signed.apk"

# --- CLEAN ---
rm -rf compiled $STAGING_DIR $UNALIGNED_APK $ALIGNED_APK $SIGNED_APK
mkdir -p compiled

# --- COMPILE RESOURCES ---
echo "[1/6] Compiling resources..."
aapt2 compile -o compiled $RES_DIR/**/*.xml

# --- LINK RESOURCES & MANIFEST ---
echo "[2/6] Linking resources and manifest..."
aapt2 link \
  -o $UNALIGNED_APK \
  --manifest $MANIFEST \
  -I $SDK_DIR/platforms/android-$API_LEVEL/android.jar \
  compiled/*.flat

# --- CREATE MINIMAL DEX ---
echo "[3/6] Creating minimal classes.dex..."
mkdir -p temp/src/com/example temp/classes
echo "package com.example; public class Empty {}" > temp/src/com/example/Empty.java

# Compile Java source to class files
javac -source 1.8 -target 1.8 -d temp/classes temp/src/com/example/Empty.java

# Package classes into JAR (optional)
cd temp/classes
jar cf ../empty.jar .
cd ../..

# Run d8, output to a directory
mkdir -p temp/dex
$SDK_DIR/build-tools/34.0.0/d8 --output temp/dex temp/empty.jar

# Add generated classes.dex to APK
zip -j $UNALIGNED_APK temp/dex/classes.dex

# --- STAGE NATIVE LIB ---
echo "[4/6] Adding native library..."
mkdir -p $STAGING_DIR/lib/arm64-v8a
cp $BUILD_DIR/$LIB_NAME $STAGING_DIR/lib/arm64-v8a/
cd $STAGING_DIR
zip -r ../$UNALIGNED_APK lib/
cd ..

# --- ZIPALIGN ---
echo "[5/6] Aligning APK..."
zipalign -f 4 $UNALIGNED_APK $ALIGNED_APK

# --- SIGN APK ---
echo "[6/6] Signing APK..."

apksigner sign \
  --ks $KEYSTORE \
  --ks-pass pass:123456 \
  --key-pass pass:123456 \
  --ks-key-alias $KEY_ALIAS \
  --out $SIGNED_APK $ALIGNED_APK

echo "APK built successfully: $SIGNED_APK"

# --- INSTALL APK ---
adb install -r $SIGNED_APK

adb logcat -T 1 | grep OpenVINS
