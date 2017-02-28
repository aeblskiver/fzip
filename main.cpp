#include <string>
#include <stdlib.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ftw.h>

#define BUFFER_SIZE 4096
char * output_file;
/*
 * TODO: Find a way to put treewalk into single fzip file
 * TODO: Directory Extraction
 */

//You must fill out your name and id below
char * studentName = (char *) "Justin Shelley";
char * studentCWID = (char *) "890443492";

//Do not change this section in your submission
char * usageString =
		(char *) "To archive a file: 		fzip -a FILE_PATH FILE_OUTPUT_PATH\n"
            "To archive a directory: 	fzip -a DIR_PATH FILE_OUTPUT_PATH\n"
            "To extract a file: 		fzip -x FILE_PATH FILE_OUTPUT_PATH\n"
            "To extract a directory: 	fzip -x DIR_PATH FILE_OUTPUT_PATH\n";

bool isExtract = false;
char * parseArg(int argc, char *argv[]) {
    if (strncmp("-n", argv[1], 2) == 0) {
        printf("Student Name: %s\n", studentName);
        printf("Student CWID: %s\n", studentCWID);
        exit(EXIT_SUCCESS);
    }

    if (argc != 4) {
		fprintf(stderr, "Incorrect arguements\n%s", usageString);
		exit(EXIT_FAILURE);
	}
	if (strncmp("-a", argv[1], 2) == 0) {
		isExtract = false;		
	} else if (strncmp("-x", argv[1], 2) == 0) {
		isExtract = true;
	} else {
		fprintf(stderr, "Incorrect arguements\n%s", usageString);
		exit(EXIT_FAILURE);
	}
    output_file = argv[3];
	return argv[2];
}
//END OF: Do not change this section



int archive(const char *path, const struct stat *sb,
             int typeflag, struct FTW *ftwbuf) {
    char file_flag = '0';
    char dir_flag = '1';
    char buffer[BUFFER_SIZE];
    ssize_t rd_in;

    int input_fd = open(path,O_RDONLY);

    //Create a single fzip file
    //No longer necessary because of extra argument
    /*char * path_fzip = ".fzip";
    char * output_file;
    output_file = (char *) malloc(strlen(path) + 1 + 4);
    strcpy(output_file,path);
    strcat(output_file,path_fzip);*/

    //Get file descriptor for output file
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (output_fd < 0)
        return -1;

    int title_size = strlen(path);
    int file_size = sb->st_size;
    ssize_t bits_written;

    switch (typeflag){
        case FTW_F:
            bits_written = write(output_fd,&file_flag,sizeof(char));                    //Write file flag
            bits_written = write(output_fd, &title_size, sizeof(int));         //Write size of title
            bits_written = write(output_fd,path,title_size);              //Write title
            bits_written = write(output_fd, &file_size, sizeof(size_t));          //Write file size

           while ((rd_in = read(input_fd,&buffer,BUFFER_SIZE)) > 0)
            {
                //printf((const char *) ("%s",buffer));
                write(output_fd,&buffer,(ssize_t) rd_in);
            };
            close(input_fd);
            close(output_fd);

            break;
        case FTW_D:
            write(output_fd,&dir_flag,sizeof(char));
            write(output_fd,&title_size,sizeof(int));
            write(output_fd,path,title_size);
            close(input_fd);
            close(output_fd);
            break;
        default:
            printf("I did nothing");
            break;
    }

    return 0;
}
/**
 *
 * Your program should archive or extract based on the flag passed in.
 * Both when extracting and archiving, it should print the output file/dir path as the last line.
 *
 * @param argc the number of args
 * @param argv the arg string table
 * @return
 */

int extract(const char * path) {
    int arch_fd = open(path, O_RDONLY);
    char buffer[BUFFER_SIZE];
    char file_flag;
    int titlesize;
    ssize_t rd_in;

    //Read the file flag first, then proceed based on flag
    while (read(arch_fd,&file_flag,1)) {
        //If it's a file then make the file and write contents
        switch (file_flag) {
            case '0': {
                //Read title size, allocate memory and read/set title
                //TODO Append something to end of title since it already exists
                read(arch_fd, &titlesize, sizeof(int));
                char *file_title = (char *) malloc(titlesize);
                read(arch_fd, file_title, titlesize);

                //Create a file with the title
                int fd = open(file_title, O_WRONLY | O_CREAT | O_EXCL, 0666);
                if (fd < 0)
                    return -1;

                free(file_title);

                //Set size of the contents
                int content_size;
                read(arch_fd, &content_size, sizeof(size_t));

                //Write the contents
                char *contents = (char *) malloc(content_size);
                read(arch_fd, contents, content_size);
                write(fd, contents, content_size);
                free(contents);
                close(fd);
                break;
            }
            case '1': {
                //Read title size, allocate memory and read/set title
                //TODO Append something to end of folder since it already exists (_"extracted")
                read(arch_fd, &titlesize, sizeof(int));
                char *folder_title = (char *) malloc(titlesize);
                read(arch_fd, folder_title, titlesize);

                //Create a folder with the title
                if (mkdir(folder_title, 0666) < 0) {
                    perror("Error making directory");
                    return (EXIT_FAILURE);
                }
                free(folder_title);
                break;
            }
            default:
                break;

        }
        close(arch_fd);
        return 1;
    }
}

int main(int argc, char** argv) {
	char * path = parseArg(argc, argv);

	int input_fd = open(path, O_RDONLY);
	if (input_fd == -1) {
		perror(("open"));
		return EXIT_FAILURE;
	}

	if (isExtract) {
		printf("Extracting %s\n", path);
		//TODO: your code to start extracting.
		char *outputPath = (char *) ""; 	//the path to the file or directory extracted
								//	this should be the same as the `path` var above but
								//	without the .fzip

    if (extract(path) != 1) {
        printf("Error extracting");
        exit(EXIT_FAILURE);
    }


		printf("%s\n", outputPath);//relative or absolute path
	} else {
		printf("Archiving %s\n", path);
		//TODO: your code to start archiving.
        //Get file stat from file descriptor
        struct stat file_stat;
        int status = fstat(input_fd, &file_stat);
        if (status < 0) {
            printf("Error reading file stat");
        }
        int flags = 0;

        /*
         * Check if it's a file or directory.
         * If it's a file, easy.
         * If it's a directory, do a tree walk.
         */
        if (nftw(path,archive,20,flags) ==-1) {
            perror("nftw");
            exit(EXIT_FAILURE);
        }

		printf("%s.fzip\n", path);
	}
	return EXIT_SUCCESS;
}

