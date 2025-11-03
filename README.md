# 🖼 Steganography using LSB Technique (BMP Image Encoding and Decoding)

## 📘 Project Overview

This project implements **Image Steganography** using the **Least Significant Bit (LSB)** technique in the **C programming language**.  
It allows users to **hide (encode)** any secret file (like `.txt`, `.c`, `.h`, `.sh`, etc.) inside a **24-bit BMP image** and later **extract (decode)** it safely — without visibly altering the image.

Both **Encoding** and **Decoding** modules are developed using structured and modular C code to ensure readability, efficiency, and maintainability.

---

## 🧩 Features

* 🔒 **Secure Data Hiding** using LSB bit manipulation.
* 🖼 Supports **24-bit BMP images** only.
* 📄 Handles multiple file formats (`.txt`, `.c`, `.h`, `.sh`).
* ✅ Validates file extensions, names, and image capacity before encoding.
* 🧠 Modular C code separated into logical components (encode/decode/types/common).
* 🔍 **Magic String Verification** to ensure valid decoding.
* 🧰 Error handling for invalid or corrupted images.
* 💡 Informative console messages for easy debugging.

---

## ⚙ System Architecture

The project is organized into modular source and header files as follows:

| File Name   | Description |
| ------------ | ------------ |
| **main.c** | Entry point; controls both encoding and decoding operations. |
| **encode.c** | Contains the full encoding logic. |
| **encode.h** | Header for `encode.c`, defines structures and function prototypes. |
| **decode.c** | Contains the full decoding logic. |
| **decode.h** | Header for `decode.c`, defines structures and function prototypes. |
| **common.h** | Contains macros like `MAGIC_STRING` and constants shared by modules. |
| **types.h** | Defines custom data types, enums (`Status`, `OperationType`, etc.). |

---

## 🧠 Working Principle (LSB Encoding)

Each **Least Significant Bit (LSB)** of an image byte is replaced with a bit from the secret file.  
This results in **minimal visual distortion** but successfully hides the data inside the image pixels.

| Original Byte | Binary | After Encoding Bit ‘1’ |
| -------------- | ------- | ---------------------- |
| 240 | 11110000 | 11110001 |

Each character (8 bits) of the secret file is stored across **8 consecutive bytes** of the BMP image.

---

## 🧮 Encoding Process

1. **Validate Inputs**
   * Source image must be `.bmp`
   * Secret file should be `.txt`, `.c`, `.h`, `.sh`
2. **Open Required Files**
   * `source_image.bmp`, `secret_file.txt`, and output `stego_image.bmp`
3. **Check Image Capacity**
   * Ensure image has enough bytes to hold secret data.
4. **Copy BMP Header**
   * First 54 bytes copied unchanged.
5. **Embed Sequentially:**
   * Magic string (e.g., `#*`)
   * Secret file extension size
   * Secret file extension (e.g., `.txt`)
   * Secret file size
   * Secret file data
6. **Write Remaining Image Data**
   * Unused pixels copied as-is.
7. **Output:**
   * Encoded image file (`stego.bmp`) generated successfully.

---

## 🔍 Decoding Process

1. **Validate Input Stego Image**
2. **Skip BMP Header (first 54 bytes)**
3. **Read and Verify Magic String**
   * Ensures correct encoded image.
4. **Decode Extension Size**
5. **Decode File Extension**
6. **Decode Secret File Size**
7. **Extract and Reconstruct Secret Data**
   * Writes decoded output to a file with original extension.

---

## 🧰 Data Structures

### 🧩 EncodeInfo (from encode.h)
```c
typedef struct _EncodeInfo {
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;

    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[5];
    char secret_data[100000];
    long size_secret_file;

    char *stego_image_fname;
    FILE *fptr_stego_image;
} EncodeInfo;
