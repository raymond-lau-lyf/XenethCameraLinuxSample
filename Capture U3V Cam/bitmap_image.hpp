#pragma once

#ifndef BITMAP_IMAGE
#define BITMAP_IMAGE

#include <stdio.h>
#include <string>

class BitmapImage {

public:
    BitmapImage() :_fileHeaderSize(14), _infoHeaderSize(40), bytesPerPixel(3) {};

    void generateBitmapFromRgbImage(unsigned char *image, int height, int width, std::string &imageFileName);
    void generateBitmapFrom8bitImage(unsigned char *image, int height, int width, std::string &imageFileName);
    const int bytesPerPixel; // = 3; /// red, green, blue

private:
    unsigned char* _createBitmapFileHeader(int height, int width);
    unsigned char* _createBitmapInfoHeader(int height, int width);

    const int _fileHeaderSize; // = 14;
    const int _infoHeaderSize; // = 40;
};


void BitmapImage::generateBitmapFromRgbImage(unsigned char *image, int height, int width, std::string &imageFileName){

    unsigned char* fileHeader = _createBitmapFileHeader(height, width);
    unsigned char* infoHeader = _createBitmapInfoHeader(height, width);
    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4-(width*bytesPerPixel)%4)%4;

#ifdef _WIN32
    FILE* imageFile;
    errno_t err = fopen_s(&imageFile, imageFileName.c_str(), "wb");
#else
    FILE* imageFile = fopen(imageFileName.c_str(), "wb");
#endif
    
    fwrite(fileHeader, 1, _fileHeaderSize, imageFile);
    fwrite(infoHeader, 1, _infoHeaderSize, imageFile);

    for(int i=0; i<height; i++){
        fwrite(image+(i*width*bytesPerPixel), bytesPerPixel, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}


void BitmapImage::generateBitmapFrom8bitImage(unsigned char *image8bit, int height, int width, std::string &imageFileName) {

    //unsigned char image[height][width][bytesPerPixel];

    unsigned char *image = new unsigned char[height * width * bytesPerPixel]();

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
//            image[i][j][2] = image8bit[i*width + j];
//            image[i][j][1] = image8bit[i*width + j];
//            image[i][j][0] = image8bit[i*width + j];
//            image[height-1 -i][width-1 - j][2] = image8bit[i*width + j];
//            image[height-1 -i][width-1 - j][1] = image8bit[i*width + j];
//            image[height-1 -i][width-1 - j][0] = image8bit[i*width + j];

              image[(height-1 -i) * (width*bytesPerPixel) + (width-1 - j) * bytesPerPixel + 2] = image8bit[i*width + j];
              image[(height-1 -i) * (width*bytesPerPixel) + (width-1 - j) * bytesPerPixel + 1] = image8bit[i*width + j];
              image[(height-1 -i) * (width*bytesPerPixel) + (width-1 - j) * bytesPerPixel + 0] = image8bit[i*width + j];
        }
    }

    generateBitmapFromRgbImage((unsigned char *)image, height, width, imageFileName);

    delete[]image;
}


unsigned char* BitmapImage::_createBitmapFileHeader(int height, int width){

    int fileSize = _fileHeaderSize + _infoHeaderSize + bytesPerPixel*height*width;

    static unsigned char fileHeader[] = {
            0,0, /// signature
            0,0,0,0, /// image file size in bytes
            0,0,0,0, /// reserved
            0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize    );
    fileHeader[ 3] = (unsigned char)(fileSize>> 8);
    fileHeader[ 4] = (unsigned char)(fileSize>>16);
    fileHeader[ 5] = (unsigned char)(fileSize>>24);
    fileHeader[10] = (unsigned char)(_fileHeaderSize + _infoHeaderSize);

    return fileHeader;
}


unsigned char* BitmapImage::_createBitmapInfoHeader(int height, int width){

    static unsigned char infoHeader[] = {
            0,0,0,0, /// header size
            0,0,0,0, /// image width
            0,0,0,0, /// image height
            0,0, /// number of color planes
            0,0, /// bits per pixel
            0,0,0,0, /// compression
            0,0,0,0, /// image size
            0,0,0,0, /// horizontal resolution
            0,0,0,0, /// vertical resolution
            0,0,0,0, /// colors in color table
            0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(_infoHeaderSize);
    infoHeader[ 4] = (unsigned char)(width    );
    infoHeader[ 5] = (unsigned char)(width>> 8);
    infoHeader[ 6] = (unsigned char)(width>>16);
    infoHeader[ 7] = (unsigned char)(width>>24);
    infoHeader[ 8] = (unsigned char)(height    );
    infoHeader[ 9] = (unsigned char)(height>> 8);
    infoHeader[10] = (unsigned char)(height>>16);
    infoHeader[11] = (unsigned char)(height>>24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(bytesPerPixel*8);

    return infoHeader;
}

#endif // BITMAP_IMAGE

