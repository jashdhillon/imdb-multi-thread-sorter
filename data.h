#ifndef DATA
#define DATA 1

	#define DEFAULT_BUFFER_SIZE 16
	#define DEFAULT_BUFFER_INCREMENT 16
	#define DEFAULT_ARRAY_SIZE 16
	#define DEFAULT_ARRAY_INCREMENT 16
	#define DEBUG 0


	//Mode type for slecting type of sorting
	typedef enum _Mode {Column=0} Mode;

	//Type of data for representing the type of the data of the movies
	typedef enum _Type {None, Integer, Double, String} Type;

	//Movie struct stores the size of the array of tokenized data, the raw data, as well as the  tokenized data
	typedef struct _Movie {
		int size;
		char* raw_data;
		char** data;
	} Movie;
	
#endif
