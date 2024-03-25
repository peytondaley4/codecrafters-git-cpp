#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <iterator>
#include <zlib.h>

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
    std::string objectPath = ".git/objects/" + objectHash.substr(0,2) + "/" + objectHash.substr(2);
	if (!std::filesystem::exists(objectPath)) {
		std::cout << "Object " << objectHash << " not found\n";
        std::cout << "Path: " << objectPath << std::endl;
		return EXIT_FAILURE;
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
			return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}
    } else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
