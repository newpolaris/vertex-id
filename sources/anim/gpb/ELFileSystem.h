#pragma once

#include "ELStream.h"
#include <string>

namespace el
{

class Properties;

/**
 * Defines a set of functions for interacting with the device file system.
 */
class FileSystem
{
public:

    /**
     * Mode flags for opening a stream.
     *
     * @script{ignore}
     */
    enum StreamMode
    {
        READ = 1,
        WRITE = 2
    };

    /**
     * Mode flags for displaying a dialog.
     *
     * @script{ignore}
     */
    enum DialogMode
    {
        OPEN,
        SAVE 
    };

    /**
     * Destructor.
     */
    ~FileSystem();

    static Stream* open(const std::string& path, size_t streamMode = READ);

    /**
     * Opens the specified file.
     *
     * The file at the specified location is opened, relative to the currently set
     * resource path.
     *
     * @param filePath The path to the file to be opened, relative to the currently set resource path.
     * @param mode The mode used to open the file, passed directly to fopen.
     * 
     * @return A pointer to a FILE object that can be used to identify the stream or NULL on error.
     * 
     * @see setResourcePath(const char*)
     * @script{ignore}
     */
    static FILE* openFile(const std::string& filePath, const char* mode);

    /**
     * Reads the entire contents of the specified file and returns its contents.
     *
     * The returned character array is allocated with new[] and must therefore
     * deleted by the caller using delete[].
     *
     * @param filePath The path to the file to be read.
     * @param fileSize The size of the file in bytes (optional).
     * 
     * @return A newly allocated (NULL-terminated) character array containing the
     *      contents of the file, or NULL if the file could not be read.
     */
    static char* readAll(const std::string& filePath, int* fileSize = NULL);

    /**
     * Determines if the file path is an absolute path for the current platform.
     * 
     * @param filePath The file path to test.
     * 
     * @return True if the path is an absolute path or false otherwise.
     */
    static bool isAbsolutePath(const std::string& filePath);

    /**
    * Sets the asset root path for the game on platforms that have separate assets (currently just Android).
    *
    * Once set, all asset paths will be loaded relative to the given path.
    * The default asset path is an empty string ("").
    *
    * @param path The asset root path.
    */
    static void setAssetPath(const char* path);

    /**
    * Returns the currently set asset root path.
    *
    * @return The currently set asset root path.
    */
    static const char* getAssetPath();

    /**
     * Creates a file on the file system from the specified asset (Android-specific).
     * 
     * @param path The path to the file.
     */
    static void createFileFromAsset(const char* path);

    /**
     * Returns the directory name up to and including the trailing '/'.
     * 
     * This is a lexical method so it does not verify that the directory exists.
     * Back slashes will be converted to forward slashes.
     * 
     * - "res/image.png" will return "res/"
     * - "image.png" will return ""
     * - "c:\foo\bar\image.png" will return "c:/foo/bar/"
     * 
     * @param path The file path. May be relative or absolute, forward or back slashes. May be NULL.
     * 
     * @return The directory name with the trailing '/'. Returns "" if path is NULL or the path does not contain a directory.
     */
    static std::string getDirectoryName(const char* path);

    /**
     * Returns the extension of the given file path.
     *
     * The extension returned includes all character after and including the last '.'
     * in the file path. The extension is returned as all uppercase.
     *
     * If the path does not contain an extension, an empty string is returned.
     * 
     * @param path File path.
     *
     * @return The file extension, all uppercase, including the '.'.
     */
    static std::string getExtension(const char* path);

private:

    /**
     * Constructor.
     */
    FileSystem();
};

}
