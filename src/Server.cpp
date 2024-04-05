#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iterator>
#include <zlib.h>
#include <iterator>
#include <vector>
#include <sstream>
#include <cstring>
#include <openssl/sha.h>

void catfile(std::string hash) {
    std::string objectPath = ".git/objects/" + hash.substr(0,2) + "/" + hash.substr(2);
	if (!std::filesystem::exists(objectPath)) {
		std::cout << "Object " << hash << " not found\n";
        std::cout << "Path: " << objectPath << std::endl;
		return;
	}
	
	try {
		//open the object file in binary mode
		std::ifstream blobObject(objectPath, std::ios::binary);
		if(blobObject.is_open()) {

		//read the entire content of object file into string
		std::string objectContent((std::istreambuf_iterator<char>(blobObject)),
			std::istreambuf_iterator<char>());

		//get the length of the object content
		unsigned long objectContentLength = objectContent.length();

		//allocate buffer to store the uncompressed object content
		unsigned long uncompressedLength = 1000;
		unsigned char *unzippedObject = new unsigned char[uncompressedLength];

		//uncomress the object content using zlib
		int result = uncompress(unzippedObject, &uncompressedLength, (unsigned char *) objectContent.c_str(), objectContentLength);
		if (result != Z_OK) {
			std::cerr << "Failed to decompress object\n";
			return;
		}

		//convert the uncompressed object to string
		objectContent = std::string((char *) unzippedObject, uncompressedLength);
		delete[] unzippedObject;

		//find the position of the first null character in file
		size_t firstNull = objectContent.find('\0');

		//extract the content data after the first null character
		std::string objectContentData = objectContent.substr(firstNull + 1);

		std::cout << objectContentData;
	}
	} catch (const std::filesystem::filesystem_error& e){
		std::cerr << e.what() << "\n";
		return;
	}
}

std::string sha_file(std::string data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)data.c_str(), data.size(), hash);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : hash) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}

void compressFile(const std::string data, uLong *bound, unsigned char *dest) {
    compress(dest, bound, (const Bytef *)data.c_str(), data.size());
}

void hashObj(std::string file) {
    std::ifstream t(file);
    std::stringstream data;
    data << t.rdbuf();

    std::string content = "blob " + std::to_string(data.str().length()) + '\0' + data.str();

    std::string buffer;
    buffer = sha_file(content);
    uLong bound = compressBound(content.size());
    unsigned char compressedData[bound];
    compressFile(content, &bound, compressedData);
    
    std::string dir = ".git/objects/" + buffer.substr(0,2);
    std::filesystem::create_directory(dir);
    std::string objectPath = dir + "/" + buffer.substr(2);
    std::ofstream objectFile(objectPath, std::ios::binary);
    objectFile.write((char *)compressedData, bound);
    objectFile.close();
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }
    
    std::string command = argv[1];
    
    if (command == "init") {
        try {
            std::filesystem::create_directory(".git");
            std::filesystem::create_directory(".git/objects");
            std::filesystem::create_directory(".git/refs");
    
            std::ofstream headFile(".git/HEAD");
            if (headFile.is_open()) {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            } else {
                std::cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }
    
            std::cout << "Initialized git directory\n";
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
    } else if (command == "cat-file") {
        if (argc < 3) {
            std::cerr << "Unknown command " << command << '\n';
            return EXIT_FAILURE;
        }

        std::string objectHash = argv[3];
        catfile(objectHash);
    
    } else if (command == "hash-object") { 
        if (argc < 3) {
            std::cerr << "Unknown command " << command << std::endl;
            return EXIT_FAILURE;
        }

        std::string dataFile = argv[3];
        hashObj(dataFile);

    } else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
