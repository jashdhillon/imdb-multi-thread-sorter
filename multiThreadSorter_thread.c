#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "iohelper.h"
#include "merge.h"
#include "strhelper.h"
#include "multiThreadSorter_thread.h"

pthread_mutex_t tid_index_lock;
pthread_mutex_t csv_index_lock;

pthread_mutex_t tid_lock;

int root_pid = -1;

//Gets reads and parses the execution arguments
int parse_exec_args(int argc, char **argv, Mode* mode, char** sort_cat, char** input_dir, char** output_dir) {

	int i;
	int has_c = 0, has_d = 0, has_o = 0;
	int expected_args = 1;

	for(i = 0; i < argc; i++) {
		char* arg = argv[i];

		if(is_string_equal(arg, "-c") && !has_c) {
		    has_c = i;
		    expected_args += 2;
		} else if(is_string_equal(arg, "-d") && !has_d) {
		    has_d = i;
		    expected_args += 2;
		} else if(is_string_equal(arg, "-o") && !has_o) {
		    has_o = i;
		    expected_args += 2;
		} else if(is_string_equal(arg, "-c") || is_string_equal(arg, "-d") || is_string_equal(arg, "-o")){
			print_error("Duplicate Arguments Passed");
			return 1;
		}
	}

	if(argc < expected_args) {
		print_error("Invalid Argument Count.");
		return 1;
	}

	
	if(!has_c) {
		print_error("Invalid sorting mode selected. Must specify sorting mode<c> and category<category_name>");
		return 1;
	}

	*mode = Column;
	*sort_cat = argv[has_c + 1];

	if(find(DEFAULT_CATEGORIES, DEFAULT_CATEGORIES_LEN, *sort_cat) < 0) {
		print_error("Invalid sorting category provided");
		return;
	}

	if(DEBUG) printf("Current Sort Mode is %d and sorting based on %s\n", *mode, *sort_cat);

	if(has_d) {
		*input_dir = argv[has_d + 1];

		DIR* dir = opendir(*input_dir);
		if (dir) {
    			closedir(dir);
		}
		else if (ENOENT == errno) {
			print_error("Input directory does not exist");
			return 1;
		}
		else {
			print_error("Input directory could not be opened");
			return 1;
		}
	} else {
		*input_dir = NULL;
	}

	if(has_o) {
		*output_dir = argv[has_o + 1];

		
		DIR* dir = opendir(*output_dir);
		if (dir) {
    			closedir(dir);
		}
		else if (ENOENT == errno) {
			print_error("Output directory does not exist");
			return 1;
		}
		else {
			print_error("Output directory could not be opened");
			return 1;
		}

	} else {
		*output_dir = NULL;
	}

	return 0;
}

//Handles the reading, parsing, sorting and outputting of the sorted data to the file
int handle_sort(Mode* mode, Movie** movies, int* movie_index, int* movie_len, char* sort_cat, char* input_dir) {
	
	FILE* input_file = fopen(input_dir, "r");	
		if(!input_file) {
		print_error("Could not read input file");
		return 1;
	}

	//get first line of data
	char* data = get_line(input_file);
	
	int cat_count = 0;
	char** categories = tokenize(data, ',', &cat_count);

	free(data);

	int i;

	for(i = 0; i < cat_count; i++) {
		char* cat = categories[i];

		if(find(DEFAULT_CATEGORIES, DEFAULT_CATEGORIES_LEN, cat) < 0) {
			print_error("Invalid Categories Detected");
			fprintf(stderr, "Category Found: %s\n", cat);
			return 1;
		}
	}
	
	if(cat_count > DEFAULT_CATEGORIES_LEN) {
		print_error("Category count scanned is greater than the total number valid");
		fprintf(stderr, "Category count: %d\n", cat_count);
		return 1;
	}
	
	int changed = 0;
	//int cat_exists = 1;
	
	int index_map[DEFAULT_CATEGORIES_LEN];

	//default out index map
	for(i = 0; i < DEFAULT_CATEGORIES_LEN; i++) {
		index_map[i] = -1;
	}

	for(i = 0; i < DEFAULT_CATEGORIES_LEN; i++) {
		int index = find(categories, cat_count, DEFAULT_CATEGORIES[i]);

		if(index >= 0) {
			index_map[index] = i;
		} else {
			//if(is_string_equal(DEFAULT_CATEGORIES[i], sort_cat)) {
				//sort_cat does not exist, set optimization flag
				//cat_exists = 0;
			//}

			changed = 1;
		}
	}

	int cat_index = find(DEFAULT_CATEGORIES, DEFAULT_CATEGORIES_LEN, sort_cat);

	if(cat_index < 0) {
		print_error("Invalid category selected");
		fclose(input_file);
		return;
	}

	data = get_line(input_file);

	while(*data != '\0') {
		Movie* temp = malloc(sizeof(Movie));
		
		int data_len = DEFAULT_CATEGORIES_LEN;
		
		//data is tokenized and ordered properly
		char** temp_data = tokenize_mapped(data, ',', data_len, index_map);

		//TODO optimize so that read_line returns this val
		int raw_len = strlen(data);
		int raw_index = 0;

		if(changed) {
			//reform string "raw" string to "reformatted" string
			//TODO rename raw_data to something more appropriate
			for(i = 0; i < DEFAULT_CATEGORIES_LEN; i++) {
		    		//All non assigned data values should be nulled out by this point
				if(temp_data[i]) {
					data = append(data, temp_data[i], &raw_index, &raw_len);
				}

				if(i != DEFAULT_CATEGORIES_LEN - 1) {
					data = append(data, ",", &raw_index, &raw_len);
				}
			}
		}

		temp -> data = temp_data;
		temp -> raw_data = data;
		temp -> size = data_len;
	
		pthread_mutex_lock(&csv_index_lock);
		
		(*movie_index)++;

		(*movies)[(*movie_index)] = *temp;	

		if((*movie_index) == ((*movie_len) - 1)) {
			(*movie_len) += DEFAULT_ARRAY_INCREMENT;
			*movies = realloc((*movies), (*movie_len) * sizeof(Movie));
			
			if((*movies) == NULL) {
				print_error("Cannot Allocate Memory");
				fclose(input_file);
				return 1;
			}
		}

		pthread_mutex_unlock(&csv_index_lock);
	
		data = get_line(input_file);
	}

	fclose(input_file);
	
	//Optimization: If the cat did not exist in the header then there is no need to sort the file
	//if(cat_exists && 0) {
		//qksort(movies, cat_index, 0, count - 1);
	//}
}

//The threaded file handler
void* file_handler(void* argv) {
	file_handler_args* args = (file_handler_args*) argv;

	Mode* mode = args -> mode;
	
	char* sort_cat = args -> sort_cat;
	char* name = args -> name;
	char* path = args -> path;
	Movie** movies = args -> movies;
	int* movie_index = args -> movie_index;
	int* movie_len = args -> movie_len;
	
	//Ingnore file and do not sort
	if(!ends_with(name, ".csv") || contains_string(name, "-sorted-")) {
		return;
	}
	
	//If result of the sort is not 0 then an error is thrown
	if(handle_sort(mode, movies, movie_index, movie_len, sort_cat, path)) {
		print_error("The sorting process returned with an error");
		return;
	}

	free(name);
	free(path);
	free(argv);
}

int val = 0;

//Utility function for generating tids
int get_next() {
	pthread_mutex_lock(&tid_lock);
	int next = val++;
	pthread_mutex_unlock(&tid_lock);

	return next;
}

//The threaded directory handler
void* dir_handler(void* argv) {	
	dir_handler_args* args = (dir_handler_args*) argv;

	Mode* mode = args -> mode;
	char* sort_cat = args -> sort_cat;
	char* path = args -> path;
	int* path_index = args -> path_index;
	int* path_len = args -> path_len;
	int** tids = args -> tids;
	int* tid_index = args -> tid_index;
	int* tid_len = args -> tid_len;
	Movie** movies = args -> movies;
	int* movie_index = args -> movie_index;
	int* movie_len = args -> movie_len;

	int* dh_tid_index = malloc(sizeof(int));
	int* dh_tid_len = malloc(sizeof(int));

	int* dhc_tid_index = malloc(sizeof(int));
	int* dhc_tid_len = malloc(sizeof(int));

	if(!(dh_tid_index && dh_tid_len && dhc_tid_index && dhc_tid_len)) {
		print_error("Cannot Allocate Memory");
		return;
	}

	*dh_tid_index = -1;
	*dhc_tid_index = -1;

	//TODO: implement tid_len adjusting
	*dh_tid_len = DEFAULT_ARRAY_SIZE * DEFAULT_ARRAY_SIZE * DEFAULT_ARRAY_SIZE;
	*dhc_tid_len = DEFAULT_ARRAY_SIZE * DEFAULT_ARRAY_SIZE * DEFAULT_ARRAY_SIZE;

	pthread_t* dh_tids = malloc(sizeof(pthread_t) * (*dh_tid_len));
	int* dhi_tids = malloc(sizeof(int) * (*dh_tid_len));
	int* dhc_tids = malloc(sizeof(int) * (*dhc_tid_len));

	if(!(dh_tids && dhi_tids && dhc_tids)) {
		print_error("Cannot Allocate Memory");
		return;
	}
    	
	FILE* buffer_file = stdout;
	DIR* dir = opendir(path);
	dirent* dir_ent;

	if(!dir) {
	    	print_error("Could not open the current directory as input directory");
		return;
	}

	dir_ent = readdir(dir);

	while(dir_ent) {
		char* name = dir_ent->d_name;
		unsigned char type = dir_ent->d_type;

		if(type == DT_DIR && !is_string_equal(name, ".") && !is_string_equal(name, "..")) {
			(*dh_tid_index)++;
			pthread_t* tid = &(dh_tids[*dh_tid_index]);
			
			//Copy and modify path
			//probably don't need to lock this as it will not be modified once it is created
			
			int* new_path_index = malloc(sizeof(int));
			int* new_path_len = malloc(sizeof(int));

			if(!(new_path_index && new_path_len)) {
				print_error("Cannot Allocate Memory");
				return;
			}	

			*new_path_index = *path_index;
			*new_path_len = *path_len;
			char* new_path = malloc(sizeof(char) * (*new_path_len));

			if(!new_path) {
				print_error("Cannot Allocate Memory");
				return;
			}

			int i;

			for(i = 0; i < *new_path_index; i++) {
				new_path[i] = path[i];
			}	
			
			new_path[i] = '\0';

			if(new_path_index > 0 && new_path[(*new_path_index) - 1] != '/' && new_path[(*new_path_index) - 1] != '\\') {
				new_path = append(new_path, "/", new_path_index, new_path_len);
			}

			new_path = append(new_path, name, new_path_index, new_path_len);
			
			dir_handler_args* dhargs = malloc(sizeof(dir_handler_args));

			if(!dhargs) {
				print_error("Cannot Allocate Memory");
				return;
			}

			dhargs -> mode = mode;
			dhargs -> sort_cat = sort_cat;
			dhargs -> path = new_path;
			dhargs -> path_index = new_path_index;
			dhargs -> path_len = new_path_len;
			dhargs -> tids = &dhc_tids;
			dhargs -> tid_index = dhc_tid_index;
			dhargs -> tid_len = dhc_tid_len;
			dhargs -> movies = movies;
			dhargs -> movie_index = movie_index;
			dhargs -> movie_len = movie_len;

			if(pthread_create(tid, NULL, dir_handler, (void*) dhargs)) {
				print_error("Error Spawning Thread");
				return;
			}

		} else if(type == DT_REG) {
			(*dh_tid_index)++;
			pthread_t* tid = &(dh_tids[*dh_tid_index]);
			
			//Optimization
			//send name, and copy new path, new path index, and new path len
			
			int new_path_index = *path_index;
			int new_path_len = *path_len;
			int new_name_len = strlen(name) + 1;

			char* new_path = malloc(sizeof(char) * new_path_len);
			char* new_name = malloc(sizeof(char) * new_name_len);

			if(!(new_path && new_name)) {
				print_error("Cannot Allocate Memory");
				return;
			} 

			strncpy(new_path, path, new_path_len);
			strncpy(new_name, name, new_name_len);

			if(new_path_index > 0 && new_path[new_path_index - 1] != '/' && new_path[new_path_index - 1] != '\\') {
				new_path = append(new_path, "/", &new_path_index, &new_path_len);
			}

			new_path = append(new_path, name, &new_path_index, &new_path_len);


			file_handler_args* fhargs = malloc(sizeof(file_handler_args));

			if(!fhargs) {
				print_error("Cannot Allocate Memory");
				return;
			}

			fhargs -> mode = mode;
			fhargs -> sort_cat = sort_cat;
			fhargs -> name = new_name;
			fhargs -> path = new_path;
			fhargs -> movies = movies;
			fhargs -> movie_index = movie_index;
			fhargs -> movie_len = movie_len;
		
			if(pthread_create(tid, NULL, file_handler, fhargs)) {
				print_error("Error Spawning Thread");
				return;
			}
			
			fprintf(buffer_file, "\nInitial PID: %d\n", root_pid);
			fprintf(buffer_file, "TIDS of all spawned threads: ");
			fprintf(buffer_file, "\nTotal number of threads: \n");
		}

		if((*dh_tid_index) == (*dh_tid_len) - 1) {
			(*dh_tid_len) += DEFAULT_ARRAY_INCREMENT;
			dh_tids = realloc(dh_tids, sizeof(pthread_t) * (*dh_tid_len));
			dhi_tids = realloc(dhi_tids, sizeof(int) * (*dh_tid_len));
		}

		dir_ent = readdir(dir);
	}

	closedir(dir);

	int i;

	//Waiting for all child threads to finish
	for(i = 0; i < (*dh_tid_index) + 1; i++) {
		pthread_join(dh_tids[i], NULL);
		dhi_tids[i] = get_next();
	}

	//Copying child spawned tids to current thread list
	for(i = 0; i < (*dhc_tid_index) + 1; i++) {
		(*dh_tid_index)++;
		dhi_tids[(*dh_tid_index)] = dhc_tids[i];
	}
	
	pthread_mutex_lock(&tid_index_lock);
	fprintf(buffer_file, "\nInitial PID: %d\n", root_pid);
	fprintf(buffer_file, "TIDS of all spawned threads: ");
	
	//TODO: Revise for string naming and optimizations
	for(i = 0; i < (*dh_tid_index) + 1; i++) {
		(*tid_index)++;
		
		int val = dhi_tids[i];

		(*tids)[(*tid_index)] = val;
		fprintf(buffer_file, "%d", val);

		if(i != (*dh_tid_index)) {
			fprintf(buffer_file, ", ");
		}

		if((*tid_index) == (*tid_len) -1) {
			(*tid_len) += DEFAULT_ARRAY_INCREMENT;
			(*tids) = realloc((*tids), sizeof(int) * (*tid_len));
		}
	}

	fprintf(buffer_file, "\nTotal number of threads: %d\n", (*dh_tid_index) + 1);
	pthread_mutex_unlock(&tid_index_lock);

	free(argv);
	free(dh_tid_index);
	free(dh_tid_len);
	free(dh_tids);
	free(dhc_tid_index);
	free(dhc_tid_len);
	free(dhc_tids);
}

//Main function of execution
int main(int argc, char **argv) {	
	pthread_mutex_init(&tid_index_lock, NULL);
	pthread_mutex_init(&csv_index_lock, NULL);

	pthread_mutex_init(&tid_lock, NULL);

	root_pid = getpid();
	
	Mode mode;
	char* sort_cat = NULL;
	char* input_dir = NULL;
	char* output_dir = NULL;

	//parsing the input args
	int res = parse_exec_args(argc, argv, &mode, &sort_cat, &input_dir, &output_dir);

	if(res > 0) {
		return res;
	}

	//Path setup
	int* path_index = malloc(sizeof(int));
	int* path_len = malloc(sizeof(int));
    	char* path;

	if(!(path_index && path_len)) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	*path_index = 0;
	*path_len = DEFAULT_BUFFER_SIZE;

	if(input_dir) {  
		int len = strlen(input_dir), i;

		path = malloc(sizeof(char) * (len + 1));

		if(!path) {
			print_error("Cannot Allocate Memory");
			return 1;
		}

		//Copy provided path
		strcpy(path, input_dir);

		*path_index = len;
		*path_len = len + 1;
	} else {
		path = malloc(sizeof(char) * (*path_len + 1));
		
		if(!path) {
			print_error("Cannot Allocate Memory");
			return 1;
		}

		path[0] = '.';
		path[1] = '/';
		path[2] = '\0';

		*path_index = 2;
	}

	//CSV Data Structure Setup
	
	int* movie_index = malloc(sizeof(int));
	int* movie_len = malloc(sizeof(int));

	if(!(movie_index && movie_len)) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	*movie_index = -1;

	//TODO: implement csv_index adjusting
	*movie_len = DEFAULT_ARRAY_SIZE;

	Movie* movies = malloc(sizeof(Movie) * (*movie_len));

	if(!movies) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	movies = malloc(sizeof(Movie) * DEFAULT_ARRAY_SIZE);

	//File handling
	FILE* buffer_file = stdout;

	int* tid_index = malloc(sizeof(int));
	int* tid_len = malloc(sizeof(int));

	if(!(tid_index && tid_len)) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	*tid_index = -1;

	//TODO: implement tid_len adjusting
	*tid_len = DEFAULT_ARRAY_SIZE * DEFAULT_ARRAY_SIZE * DEFAULT_ARRAY_SIZE;

	int* tids = malloc(sizeof(int) * (*tid_len));

	if(!tids) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	//call dir handler
	dir_handler_args* dhargs = malloc(sizeof(dir_handler_args));

	
	if(!dhargs) {
		print_error("Cannot Allocate Memory");
		return 1;
	}

	dhargs -> mode = &mode;
	dhargs -> sort_cat = sort_cat;
	dhargs -> path = path;
	dhargs -> path_index = path_index;
	dhargs -> path_len = path_len;
	dhargs -> tids = &tids;
	dhargs -> tid_index = tid_index;
	dhargs -> tid_len = tid_len;
	dhargs -> movies = &movies;
	dhargs -> movie_index = movie_index;
	dhargs -> movie_len = movie_len;

	dir_handler((void*) dhargs);

	free(path_index);
	free(path_len);
	free(path);

	int i;

	int j;

	int master_list_index = 0;
	int master_list_len = DEFAULT_ARRAY_SIZE;

	/*
	Movie* master_list = malloc(sizeof(Movie) * master_list_len);

	for(i = 0; i < (*csv_index) + 1; i++) {
		CSV csv = csvs[i];
		int movie_count = csv.size;
		Movie* movies = csv.movies;

		if(movie_count > (master_list_len - master_list_index)) {
			master_list_len += movie_count;
			master_list = realloc(master_list, sizeof(Movie) * master_list_len);
		}

		for(j = 0; j < movie_count; j++) {
			master_list[master_list_index++] = movies[j];
		}
	}*/

	Movie* master_list = movies;
	master_list_index = *movie_index;
	master_list_len = *movie_len;

	int cat_index = find(DEFAULT_CATEGORIES, DEFAULT_CATEGORIES_LEN, sort_cat);

	msort(master_list, cat_index, 0, master_list_index - 1);
	
	path_index = malloc(sizeof(int));
	path_len = malloc(sizeof(int));

	*path_index = 0;
	*path_len = DEFAULT_BUFFER_SIZE;

	if(output_dir) {
		int len = strlen(output_dir), i;

		path = malloc(sizeof(char) * (len + 1));

		if(!path) {
			print_error("Cannot Allocate Memory");
			return 1;
		}

		//Copy provided path
		strcpy(path, output_dir);

		*path_index = len;
		*path_len = len + 1;
	} else {
		path = malloc(sizeof(char) * (*path_len + 1));
		
		if(!path) {
			print_error("Cannot Allocate Memory");
			return 1;
		}

		path[0] = '.';
		path[1] = '/';
		path[2] = '\0';

		*path_index = 2;
	}

	if(*path_index > 0 && path[(*path_index) - 1] != '/' && path[(*path_index) - 1] != '\\') {
		path = append(path, "/", path_index, path_len);
	}
	
	path = append(path, "AllFiles-sorted-", path_index, path_len);
	path = append(path, sort_cat, path_index, path_len);
	path = append(path, ".csv", path_index, path_len);

	FILE* output_file = fopen(path, "w");

	if(!output_file) {
		print_error("Cannot open output file");
		return 0;
	}

	for(i = 0; i < DEFAULT_CATEGORIES_LEN; i++) {
		fprintf(output_file, "%s", DEFAULT_CATEGORIES[i]);
		
		if(i != DEFAULT_CATEGORIES_LEN - 1) {
			fprintf(output_file, ",");
		}
	}

	fprintf(output_file, "\n");

	for(i = 0; i < master_list_index; i++) {
		fprintf(output_file, "%s\n", master_list[i].raw_data);
	}

	fclose(output_file);

	return 0;
}
