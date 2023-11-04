#include "File.h"


bool File::isCanceled = false; //initialization of static flag 


File::File(const string& filePath, Observer& observer): Observable() { //remember that the ctor throws exceptions!
    filesystem::path p(filePath); //create path object to get info from filePath
    if (!exists(p)) { //check if the path exists
        throw runtime_error("Error, the path: " + filePath + " does not exist."); //if not we throw runtime error
    }
    this->name = p.stem().string(); //set the name of file
    this->extention = p.extension().string(); //set the extention 
    this->fullName = p.stem().string() + p.extension().string(); //set the file full name
    this->fullPath = p.string(); //set full path of file
    this->length = filesystem::file_size(filePath); //set the length with file_size method
    this->addObserver(&observer); //add the observer to observer list
}


/// <summary>
/// Function to remove a file from pc 
/// </summary>
/// <param name="File file"></param>
void File::removeFile(const File& file) {
    if (remove(file.fullPath.c_str()) != 0) { //if true we failed removing file
        throw runtime_error("Error trying to delete file: " + file.fullName); //throw exception with error
    }
}


/// <summary>
/// Function that handles wiping the file contents securely using crypto methods
/// </summary>
/// <param name="File file"></param>
/// <param name="int passes"></param>
/// <param name="bool toRemove"></param>
void File::WipeFile(const File& file, int passes, bool toRemove) {
    fstream outputFile(file.fullPath, ios::in | ios::out | ios::binary); //open the file in binary mode for reading and writing

    if (!outputFile.is_open() || outputFile.fail()) { //we check if we failed opening the file
        throw runtime_error("Error opening file: " + file.fullName); //throw runtime error exception if failed to open file
        return; //stop the method
    }

    random_device rd; //for random bytes generator
    mt19937_64 generator(rd()); //secure random byte generator using Mersenne Twister algorithm 

    //we iterate in a loop each pass and wipe the file's contents
    for (int pass = 0; pass < passes; pass++) { 
        outputFile.seekp(0); //we start from the beginning of the file each pass
        size_t fileSize = file.length; //set the fileSize to be file size in bytes for wipe
        size_t currentSize = 0; //set currentSize to be zero to indicate the beginning of file
        const size_t bufferSize = fileSize / 4; //set a bufferSize to be a quarter of original file size for memory efficiency
        vector<char> buffer(bufferSize); //create a vector that uses the bufferSize

        //wiping the file with random data with Mersenne Twister algorithm
        while (currentSize != file.length) { //we write until we reach the original size of file
            const size_t chunkSize = min(fileSize, bufferSize); //set chunkSize based on the minimum between the fileSize and bufferSize
            outputFile.seekp(currentSize, ios::beg); //set the pointer for replacing bytes with the currentSize parameter to write in chunks
            //generate random data
            for (size_t i = 0; i < chunkSize; i++) {
                if (File::isCanceled) { //if true we stop the file wipe
                    outputFile.close(); //after we finish we close the file
                    file.notify(); //notify all observers that we finished the task
                    return; //finish the function if we need to cancel
                }
                buffer[i] = (unsigned char)(generator() & 0xFF); //generate a random byte and insert it into the buffer
            }
            outputFile.write(buffer.data(), chunkSize); //write buffer data to the file
            outputFile.flush(); //flush the file 
            currentSize += chunkSize; //add chunkSize to currentSize for indication and for the point where we need to write more data in next iteration
            fileSize -= chunkSize; //subtract chunkSize we wrote to file from the total fileSize
        }
    }
    outputFile.close(); //after we finish we close the file
    if (toRemove) //if true we need to remove the file
        removeFile(file); //call removeFile function to remove the file
    file.notify(); //notify all observers that we finished the task
}


/// <summary>
/// Function that handles encryption/decryption on given file using custom AES library
/// </summary>
/// <param name="File file"></param>
/// <param name="string key"></param>
/// <param name="bool decrypt"></param>
void File::CipherFile(const File& file, const string& key, bool decrypt) {
    fstream outputFile(file.fullPath, ios::in | ios::out | ios::binary); // Open the file in binary mode for reading and writing

    if (!outputFile.is_open() || outputFile.fail()) { // Check if we failed opening the file
        throw runtime_error("Error opening file: " + file.fullName); // Throw a runtime error exception if failed to open the file
        return; // Stop the method
    }

    outputFile.seekp(0); // Start from the beginning of the file each pass
    size_t fileSize = file.length; // Set the fileSize to be the file size in bytes for wiping
    size_t currentSize = 0; // Set currentSize to be zero to indicate the beginning of the file
    const size_t bufferSize = fileSize / 4; // Set a bufferSize to be a quarter of the original file size for memory efficiency
    vector<char> buffer(bufferSize); // Create a vector that uses the bufferSize
    string key1 = "PopSmokeTheWoo55";
    string iv = "PopSmokeTheWoo55";
    vector<unsigned char> keyVec(key1.begin(), key1.end());
    vector<unsigned char> ivVec(iv.begin(), iv.end());

    while (currentSize != file.length) { // Write until we reach the original size of the file
        const size_t chunkSize = min(fileSize, bufferSize);

        // Read asynchronously
        future<void> readFuture = async(launch::async, [&]() {
            outputFile.seekg(currentSize);
            vector<unsigned char> buffer(chunkSize);
            outputFile.read(reinterpret_cast<char*>(buffer.data()), chunkSize);

            // Perform encryption
            buffer = AES::Encrypt_OFB(buffer, keyVec, ivVec);

            if (File::isCanceled) { //if true we stop the file wipe
                outputFile.close(); //after we finish we close the file
                file.notify(); //notify all observers that we finished the task
                return; //finish the function if we need to cancel
            }

            // Write asynchronously
            future<void> writeFuture = async(launch::async, [&]() {
                outputFile.seekp(currentSize);
                outputFile.write(reinterpret_cast<char*>(buffer.data()), chunkSize);
                outputFile.flush();
                });

            writeFuture.get(); // Wait for the write to finish
            });

        readFuture.get(); // Wait for the read and encryption to finish

        currentSize += chunkSize;
        fileSize -= chunkSize;
    }

    outputFile.close(); //after we finish, we close the file
    file.notify(); //notify all observers that we finished the task
}