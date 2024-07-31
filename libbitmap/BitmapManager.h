#ifndef BITMAPMANAGER_H
#define BITMAPMANAGER_H

#include <vector>
#include <QString>
#include <fstream>

struct Bitmap
{
    int width;              // bitmap width in pixels
    int height;             // bitmap height in pixels
    unsigned char *data;    // Pointer to bitmap data. data[j * width + i] is color of pixel in row j and column i.
};

class BitmapManager
{
public:
    BitmapManager();
    ~BitmapManager();
    void encode(const QString &fileName);
    void decode(const QString &fileName);

private:
    Bitmap loadBitmap(const QString &fileName);
    void openFile(const QString &fileName);
    void writeBit(bool bit);
    bool readBit(std::ifstream &file);
    void saveBitmapToBMP(const Bitmap &bitmap, const QString &fileName);

    std::ofstream   m_file;
    unsigned char   m_bitBuffer;
    int             m_bitCount;

    static const unsigned char SIGNATURE[6];
};

#endif // BITMAPMANAGER_H
