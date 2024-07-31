#include "BitmapManager.h"
#include <iostream>
#include <stdexcept>
#include <climits>
#include <cstring>

const unsigned char BitmapManager::SIGNATURE[6] = {'B', 'I', 'T', 'M', 'A', 'P',};

BitmapManager::BitmapManager() : m_bitBuffer(0), m_bitCount(0)
{
}

BitmapManager::~BitmapManager()
{
}

void BitmapManager::encode(const QString &fileName)
{
    Bitmap bitmap = loadBitmap(fileName);
    QString outputFileName = fileName + "_packed.barch";

    openFile(outputFileName);

    m_bitBuffer = 0;
    m_bitCount = 0;

    m_file.write(reinterpret_cast<const char*>(SIGNATURE), sizeof(SIGNATURE));
    m_file.write(reinterpret_cast<const char*>(&bitmap.width), sizeof(bitmap.width));
    m_file.write(reinterpret_cast<const char*>(&bitmap.height), sizeof(bitmap.height));

    // write indexEmptyRow
    bool indexEmptyRow[bitmap.height];

    for(int j = 0; j < bitmap.height; ++j)
    {
        bool emptyRow = true;

        for(int i = 0; i < bitmap.width; ++i)
        {
            if(bitmap.data[j * bitmap.width + i] != 0xff)
            {
                emptyRow = false;
                break;
            }
        }

        indexEmptyRow[j] = emptyRow;
        writeBit(emptyRow);
    }

    for(int j = 0; j < bitmap.height; ++j)
    {
        if(indexEmptyRow[j])
            continue;

        for(int i = 0; i < bitmap.width; i += 4)
        {
            int ind = j * bitmap.width + i;

            // end of file may not be 4 bytes
            if(i + 4 < bitmap.width && bitmap.data[ind] == bitmap.data[ind + 1]
                    && bitmap.data[ind] == bitmap.data[ind + 2] && bitmap.data[ind] == bitmap.data[ind + 3])
            {
                if(bitmap.data[ind] == 0xff)
                {
                    writeBit(0);
                    continue;
                }
                else if(bitmap.data[ind] == 0x00)
                {
                    writeBit(1);
                    writeBit(0);
                    continue;
                }
            }
            writeBit(1);
            writeBit(1);

            for(int tmp = 0; tmp + i < bitmap.width && tmp < 4; ++tmp)
            {
                unsigned char pixel = bitmap.data[tmp + ind];

                for(int b = CHAR_BIT - 1; b >= 0; --b)
                    writeBit((pixel >> b) & 1);
            }
        }
    }

    // write less bits
    if(m_bitCount > 0)
    {
        m_bitBuffer <<= (CHAR_BIT - m_bitCount);
        m_file.write(reinterpret_cast<const char*>(&m_bitBuffer), 1);
    }

    m_file.close();

    delete [] bitmap.data;
}

void BitmapManager::decode(const QString &fileName)
{
    std::ifstream file(fileName.toStdString(), std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error("Cannot open file: " + fileName.toStdString());

    unsigned char signature[6];
    file.read(reinterpret_cast<char*>(signature), sizeof(signature));

    // check signature
    if(std::memcmp(signature, SIGNATURE, sizeof(SIGNATURE)) != 0)
        throw std::runtime_error("Invalid file signature: " + fileName.toStdString());

    Bitmap bitmap;

    file.read(reinterpret_cast<char*>(&bitmap.width), sizeof(bitmap.width));
    file.read(reinterpret_cast<char*>(&bitmap.height), sizeof(bitmap.height));

    m_bitBuffer = 0;
    m_bitCount = 0;
    bool indexEmptyRow[bitmap.height];
    bitmap.data = new unsigned char[bitmap.width * bitmap.height];

    for(int j = 0; j < bitmap.height; ++j)
        indexEmptyRow[j] = readBit(file);

    for(int j = 0; j < bitmap.height; ++j)
    {
        if(indexEmptyRow[j])
        {
            for(int i = 0; i < bitmap.width; ++i)
                bitmap.data[j * bitmap.width + i] = 0xff;
        }
        else
        {
            for(int i = 0; i < bitmap.width;)
            {
                bool firstBit = readBit(file);

                if(!firstBit) // 0
                {
                    for(int byte = 0; byte < 4; ++byte)
                        bitmap.data[j * bitmap.width + i++] = 0xff;
                }
                else if(!readBit(file)) // 10
                {
                    for(int byte = 0; byte < 4; ++byte)
                        bitmap.data[j * bitmap.width + i++] = 0x00;
                }
                else // 11
                {
                    for(int byte = 0; byte < 4 && i < bitmap.width; ++byte && ++i)
                        for(int bit = 0; bit < CHAR_BIT; ++bit)
                            bitmap.data[j * bitmap.width + i] = (bitmap.data[j * bitmap.width + i] << 1) | readBit(file);
                }
            }
        }
    }

    file.close();

    // Save the decoded bitmap to a BMP file
    QString outputFileName = fileName + "_unpacked.bmp";
    saveBitmapToBMP(bitmap, outputFileName);

    delete[] bitmap.data;
}

Bitmap BitmapManager::loadBitmap(const QString &fileName)
{
    std::ifstream file(fileName.toStdString(), std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error("Cannot open file: " + fileName.toStdString());

    unsigned char header[54];
    file.read(reinterpret_cast<char*>(header), sizeof(header));

    if(file.gcount() != sizeof(header) || !(header[0] == 'B' && header[1] == 'M'))
        throw std::runtime_error("Error reading BMP header");

    int width = *reinterpret_cast<int*>(&header[18]);
    int height = *reinterpret_cast<int*>(&header[22]);

    unsigned char *data = new unsigned char[width * height];
    file.read(reinterpret_cast<char*>(data), width * height);

    if(file.gcount() != width * height)
    {
        delete[] data;
        throw std::runtime_error("Error reading BMP data");
    }

    Bitmap bitmap = {width, height, data};
    return bitmap;
}

void BitmapManager::openFile(const QString &fileName)
{
    if(m_file.is_open())
        m_file.close();

    m_file.open(fileName.toStdString(), std::ios::binary);

    if(!m_file.is_open())
        throw std::runtime_error("Cannot open file for writing: " + fileName.toStdString());
}

void BitmapManager::writeBit(bool bit)
{
    m_bitBuffer = (m_bitBuffer << 1) | bit;
    m_bitCount++;

    if(m_bitCount == CHAR_BIT)
    {
        m_file.write(reinterpret_cast<const char*>(&m_bitBuffer), 1);
        m_bitBuffer = 0;
        m_bitCount = 0;
    }
}

bool BitmapManager::readBit(std::ifstream &file)
{
    if(m_bitCount == 0)
    {
        file.read(reinterpret_cast<char*>(&m_bitBuffer), 1);
        m_bitCount = CHAR_BIT;
    }

    bool bit = (m_bitBuffer >> (m_bitCount - 1)) & 1;
    --m_bitCount;

    return bit;
}

void BitmapManager::saveBitmapToBMP(const Bitmap &bitmap, const QString &fileName)
{
    openFile(fileName);

    unsigned char header[54] = {0};
    int fileSize = 54 + bitmap.width * bitmap.height;

    // BMP Header
    header[0] = 'B';
    header[1] = 'M';

    // full size
    header[2] = static_cast<unsigned char>(fileSize);
    header[3] = static_cast<unsigned char>(fileSize >> 8);
    header[4] = static_cast<unsigned char>(fileSize >> 16);
    header[5] = static_cast<unsigned char>(fileSize >> 24);

    // bfOffBits
    header[10] = 0x8A;
    header[11] = 0x04;

    header[14] = 0x7C;

    // width & height
    header[18] = static_cast<unsigned char>(bitmap.width);
    header[19] = static_cast<unsigned char>(bitmap.width >> 8);
    header[20] = static_cast<unsigned char>(bitmap.width >> 16);
    header[21] = static_cast<unsigned char>(bitmap.width >> 24);
    header[22] = static_cast<unsigned char>(bitmap.height);
    header[23] = static_cast<unsigned char>(bitmap.height >> 8);
    header[24] = static_cast<unsigned char>(bitmap.height >> 16);
    header[25] = static_cast<unsigned char>(bitmap.height >> 24);

    header[26] = 0x01;
    header[28] = 0x08; // bit in pixel
    header[34] = 0x40;
    header[35] = 0x29;
    header[36] = 0x0F;
    header[38] = 0x13;
    header[39] = 0x0B;
    header[42] = 0x13;
    header[43] = 0x0B;
    header[47] = 0x01;
    header[51] = 0x01;

    m_file.write(reinterpret_cast<const char*>(header), sizeof(header));
    m_file.write(reinterpret_cast<const char*>(bitmap.data), bitmap.width * bitmap.height);

    m_file.close();
}
