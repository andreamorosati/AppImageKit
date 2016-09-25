/*
	cc -o digest digest.c -lssl -lcrypto
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef unsigned char byte;

int sha256_file(char *path, char outputBuffer[65], int skip_offset, int skip_length)
{
    FILE *file = fopen(path, "rb");
    if(!file) return(1);
    byte hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = 1024*1024;
    byte *buffer = malloc(bufSize);
    int bytesRead = 0;
    if(!buffer) return ENOMEM;

    int totalBytesRead = 0;
    if(skip_offset <= bufSize){
        bytesRead = fread(buffer, 1, skip_offset, file);
        totalBytesRead += bytesRead;
        // printf("totalBytesRead: %i\n", totalBytesRead);
        // printf("bytesRead: %i\n", bytesRead);
        SHA256_Update(&sha256, buffer, bytesRead);
    } else {
        int stillToRead = skip_offset-bytesRead;
        // printf("Initial stillToRead: %i\n", stillToRead);
        int readThisTime;

        if(stillToRead>bufSize){
            readThisTime = bufSize;
        } else {
            readThisTime = stillToRead;
        }
        while((bytesRead = fread(buffer, 1, readThisTime, file)))
        {
            totalBytesRead += bytesRead;
            // printf("totalBytesRead: %i\n", totalBytesRead);
            // printf("readThisTime: %i\n", readThisTime);
            // printf("bytesRead: %i\n", bytesRead);
            SHA256_Update(&sha256, buffer, bytesRead);
            stillToRead = skip_offset-totalBytesRead;
            // printf("stillToRead: %i\n", stillToRead);

            if(stillToRead>bufSize){
                readThisTime = bufSize;
            } else {
                readThisTime = stillToRead;
            }
        }
    }

    fseek(file, skip_offset+skip_length, SEEK_SET);

    while((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        totalBytesRead += bytesRead;
        // printf("totalBytesRead: %i\n", totalBytesRead);
        // printf("bytesRead: %i\n", bytesRead);
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final(hash, &sha256);

    printf("totalBytesRead: %i\n", totalBytesRead);
    
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
    
    return 0;
}

int main(int argc,char **argv)	{
        if(argc < 4){
            printf("Calculate a sha256 of a file except a skipped area from offset to offset+length bytes\n");
            printf("This is useful when a signature is placed in the skipped area\n");
            printf("Usage: %s file offset length\n", argv[0]);
            exit(1);
        }
    int skip_offset = atoi(argv[2]);
    int skip_length = atoi(argv[3]);
    char *filename = argv[1];
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    if(size < skip_offset+skip_length){
        printf("offset+length cannot be less than the file size\n");
        exit(1);
    }
	static unsigned char buffer[65];
	int res = sha256_file(filename, buffer, skip_offset, skip_length);
	printf("%s\n", buffer);
	return res;
}
