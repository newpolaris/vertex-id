#define _CRT_SECURE_NO_WARNINGS
#include "ELBase.h"
#include "ELFileSystem.h"
// #include "ELProperties.h"
#include "ELStream.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cstdio>

namespace el
{

/**
 * 
 * @script{ignore}
 */
class FileStream : public Stream
{
public:
    friend class FileSystem;
    
    ~FileStream();
    virtual bool canRead();
    virtual bool canWrite();
    virtual bool canSeek();
    virtual void close();
    virtual size_t read(void* ptr, size_t size, size_t count);
    virtual char* readLine(char* str, int num);
    virtual size_t write(const void* ptr, size_t size, size_t count);
    virtual bool eof();
    virtual size_t length();
    virtual long int position();
    virtual bool seek(long int offset, int origin);
    virtual bool rewind();

    static FileStream* create(const std::string& filePath, const char* mode);

private:
    FileStream(FILE* file);

private:
    FILE* _file;
    bool _canRead;
    bool _canWrite;
};

/////////////////////////////

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

Stream* FileSystem::open(const std::string& path, size_t streamMode)
{
    char modeStr[] = "rb";
    if ((streamMode & WRITE) != 0)
        modeStr[0] = 'w';
    FileStream* stream = FileStream::create(path, modeStr);
    return stream;
}

FILE* FileSystem::openFile(const std::string& filePath, const char* mode)
{
    FILE* fp = fopen(filePath.c_str(), mode);
    return fp;
}

char* FileSystem::readAll(const std::string& filePath, int* fileSize)
{
    GP_ASSERT(!filePath.empty());

    // Open file for reading.
    std::unique_ptr<Stream> stream(open(filePath));
    if (stream.get() == NULL)
    {
        GP_ERROR("Failed to load file: %s", filePath);
        return NULL;
    }
    size_t size = stream->length();

    // Read entire file contents.
    char* buffer = new char[size + 1];
    size_t read = stream->read(buffer, 1, size);
    if (read != size)
    {
        GP_ERROR("Failed to read complete contents of file '%s' (amount read vs. file size: %u < %u).", filePath, read, size);
        SAFE_DELETE_ARRAY(buffer);
        return NULL;
    }

    // Force the character buffer to be NULL-terminated.
    buffer[size] = '\0';

    if (fileSize)
    {
        *fileSize = (int)size; 
    }
    return buffer;
}

bool FileSystem::isAbsolutePath(const std::string& filePath)
{
    if (filePath.empty() || filePath[0] == '\0')
        return false;
#ifdef WIN32
    if (filePath[1] != '\0')
    {
        char first = filePath[0];
        return (filePath[1] == ':' && ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')));
    }
    return false;
#else
    return filePath[0] == '/';
#endif
}

void FileSystem::setAssetPath(const char* path)
{
}

const char* FileSystem::getAssetPath()
{
    return nullptr;
}

void FileSystem::createFileFromAsset(const char* path)
{
}

std::string FileSystem::getDirectoryName(const char* path)
{
    if (path == NULL || strlen(path) == 0)
    {
        return "";
    }
#ifdef _WIN32
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    _splitpath(path, drive, dir, NULL, NULL);
    std::string dirname;
    size_t driveLength = strlen(drive);
    if (driveLength > 0)
    {
        dirname.reserve(driveLength + strlen(dir));
        dirname.append(drive);
        dirname.append(dir);
    }
    else
    {
        dirname.assign(dir);
    }
    std::replace(dirname.begin(), dirname.end(), '\\', '/');
    return dirname;
#else
    // dirname() modifies the input string so create a temp string
    std::string dirname;
    char* tempPath = new char[strlen(path) + 1];
    strcpy(tempPath, path);
    char* dir = ::dirname(tempPath);
    if (dir && strlen(dir) > 0)
    {
        dirname.assign(dir);
        // dirname() strips off the trailing '/' so add it back to be consistent with Windows
        dirname.append("/");
    }
    SAFE_DELETE_ARRAY(tempPath);
    return dirname;
#endif
}

std::string FileSystem::getExtension(const char* path)
{
    const char* str = strrchr(path, '.');
    if (str == NULL)
        return "";

    std::string ext;
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
        ext += std::toupper(str[i]);

    return ext;
}

//////////////////

FileStream::FileStream(FILE* file)
    : _file(file), _canRead(false), _canWrite(false)
{
    
}

FileStream::~FileStream()
{
    if (_file)
    {
        close();
    }
}

FileStream* FileStream::create(const std::string& filePath, const char* mode)
{
    FILE* file = fopen(filePath.c_str(), mode);
    if (file)
    {
        FileStream* stream = new FileStream(file);
        const char* s = mode;
        while (s != NULL && *s != '\0')
        {
            if (*s == 'r')
                stream->_canRead = true;
            else if (*s == 'w')
                stream->_canWrite = true;
            ++s;
        }

        return stream;
    }
    return NULL;
}

bool FileStream::canRead()
{
    return _file && _canRead;
}

bool FileStream::canWrite()
{
    return _file && _canWrite;
}

bool FileStream::canSeek()
{
    return _file != NULL;
}

void FileStream::close()
{
    if (_file)
        fclose(_file);
    _file = NULL;
}

size_t FileStream::read(void* ptr, size_t size, size_t count)
{
    if (!_file)
        return 0;
    return fread(ptr, size, count, _file);
}

char* FileStream::readLine(char* str, int num)
{
    if (!_file)
        return 0;
    return fgets(str, num, _file);
}

size_t FileStream::write(const void* ptr, size_t size, size_t count)
{
    if (!_file)
        return 0;
    return fwrite(ptr, size, count, _file);
}

bool FileStream::eof()
{
    if (!_file || feof(_file))
        return true;
    return ((size_t)position()) >= length();
}

size_t FileStream::length()
{
    size_t len = 0;
    if (canSeek())
    {
        long int pos = position();
        if (seek(0, SEEK_END))
        {
            len = position();
        }
        seek(pos, SEEK_SET);
    }
    return len;
}

long int FileStream::position()
{
    if (!_file)
        return -1;
    return ftell(_file);
}

bool FileStream::seek(long int offset, int origin)
{
    if (!_file)
        return false;
    return fseek(_file, offset, origin) == 0;
}

bool FileStream::rewind()
{
    if (canSeek())
    {
        ::rewind(_file);
        return true;
    }
    return false;
}

} // el
