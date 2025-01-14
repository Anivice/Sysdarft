#include <SysdarftDebug.h>
#include <SysdarftDisks.h>

std::streamoff getFileSize(std::fstream& file)
{
    // Save current position (if needed)
    const std::streampos currentPos = file.tellg();

    // Seek to the end to determine file size
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();

    // Restore the pointer to the original position (optional)
    file.seekg(currentPos);

    return size;
}
