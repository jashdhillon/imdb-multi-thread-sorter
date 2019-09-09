#include "data.h"

#ifndef PROTOTYPES
#define PROTOTYPES	
	//Type def for struct dirent renaming
	typedef struct dirent dirent;

	#define DEFAULT_CATEGORIES_LEN 28

	char *DEFAULT_CATEGORIES[DEFAULT_CATEGORIES_LEN] = {
    		"color",
    		"director_name",
    		"num_critic_for_reviews",
    		"duration",
    		"director_facebook_likes",
    		"actor_3_facebook_likes",
    		"actor_2_name",
    		"actor_1_facebook_likes",
    		"gross",
    		"genres",
    		"actor_1_name",
    		"movie_title",
    		"num_voted_users",
    		"cast_total_facebook_likes",
    		"actor_3_name",
    		"facenumber_in_poster",
    		"plot_keywords",
    		"movie_imdb_link",
    		"num_user_for_reviews",
    		"language",
    		"country",
    		"content_rating",
    		"budget",
    		"title_year",
    		"actor_2_facebook_likes",
    		"imdb_score",
    		"aspect_ratio",
    		"movie_facebook_likes"
	};


	//Gets reads and parses the execution arguments
	int parse_exec_args(int argc, char **argv, Mode* mode, char** sort_cat, char** input_dir, char** output_dir);

	//Handles the reading, parsing, sorting and outputting of the sorted data to the file
	int handle_sort(Mode* mode, Movie** csvs, int* movie_index, int* movie_len, char* sort_cat, char* input_dir);

	//The threaded file handler
	void* file_handler(void* argv);

	//The threaded directory handler
	void* dir_handler(void* argv);

	typedef struct file_handler_args {
   		Mode* mode;
		char* sort_cat;
		char* name;
		char* path;
		Movie** movies;
		int* movie_index;
		int* movie_len;
	} file_handler_args;

	void* file_handler(void* argv);

	//TODO: Add to header file along with other new functions and structs
	void* dir_handler(void* argv);

	typedef struct dir_handler_args_t {
       		Mode* mode;
		char* sort_cat;
		char* path;
		int* path_index;
		int* path_len;
		int** tids;
		int* tid_index;
		int* tid_len;
		Movie** movies;
		int* movie_index;
		int* movie_len;
	} dir_handler_args;
#endif
