#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <float.h>

#include "merge.h"
#include "iohelper.h"
#include "strhelper.h"

//Lookup table function
Type get_data_type(int cat_index) {
	switch(cat_index)
	{
		case 0: {
			return String;	//color
		}
		case 1: {
			return String;	//director_name
		}
		case 2: {
			return Integer;	//num_critic_for_Reviews
		}
		case 3: {
			return Double;	//duration
		}
		case 4: {
			return Integer;	//director_facebook_likes
		}
		case 5: {
			return Integer;	//actor_2_facebook_likes
		}
		case 6: {
			return String;	//actor_2_name
		}
		case 7: {
			return Integer;	//actor_1_facebook_likes
		}
		case 8: {
			return Double;	//gross
		}
		case 9: {
			return String;	//genres
		}
		case 10: {
			return String;	//actor_1_name
		}
		case 11: {
			return String;	//movie_title
		}
		case 12: {
			return Integer; //num_voted_users
		}
		case 13: {
			return Integer; //cast_total_facebook_likes
		}
		case 14: {
			return String;	//actor_3_name
		}
		case 15: {
			return Integer;	//facenumber_in_poster
		}
		case 16: {
			return String;	//plot_keywords
		}
		case 17: {
			return String;	//movie_imdb_link
		}
		case 18: {
			return Integer;	//num_user_for_reviews
		}
		case 19: {
			return String;	//Language
		}
		case 20: {
			return String;	//Country
		}
		case 21: {
			return String;	//content_rating
		}
		case 22: {
			return Double; //budget
		}
		case 23: {
			return Integer; //title_year
		}
		case 24: {
			return Integer; //actor_2_facebook_likes
		}
		case 25: {
			return Double; //imdb_score
		}
		case 26: {
			return Double; //aspect_ratio
		}
		case 27: {
			return Integer; //movie_facebook_likes
		}
	}

	return None;
}

//Compares two movies and returns -1 if a < b, 0 if a == b and 1 otherwise
int cmp_movies(Movie* a, Movie* b, int cat_index) {
	char* a_data = (*a).data[cat_index];
	char* b_data = (*b).data[cat_index];
	
	Type data_type = get_data_type(cat_index);

	if(data_type == Integer || data_type == Double) {
		double l_val, r_val;

		if(*a_data == '\0') {
			if(*b_data == '\0') {
				return 0;
			} else {
				return -1;
			}
		} else {
			sscanf(a_data, "%lf", &l_val);
		}

		if(*b_data == '\0') {
			if(*a_data == '\0') {
				return 0;
			} else {
				return 1;
			}
		} else {
		    sscanf(b_data, "%lf", &r_val);
		}

		if(l_val < r_val) {
			return -1;
		} else if(l_val > r_val) {
			return 1;
		} else {
			return 0;
		}

		return l_val - r_val;
	} else {
		int left_start = 0, left_end = 0;
		int right_start = 0, right_end = 0;

		trim(a_data, &left_start, &left_end);
		trim(b_data, &right_start, &right_end);

		a_data += left_start;
		b_data += right_start;

		int left_len = left_end - left_start + 1;
		int right_len = right_end - right_start + 1;

		int left_shorter = left_len <= right_len;
		int min = left_shorter ? left_len : right_len;

		int cmp = strncmp(a_data, b_data, min);

		if(cmp == 0 && left_shorter) {
			return 0;
		} else if(cmp == 0 && !left_shorter) {
			return 1;
		} else if(cmp < 0) {
			return -1;
		} else {
			return 1;
		}
	}
}

//Merges 2 lists in sorted order
//Core of the merge sort sorting fucntion
void merge(Movie* movies, int cat_index, int l, int m, int r) {
	
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;

	Movie* L = malloc(n1 * sizeof(Movie));
	Movie* R = malloc(n2 * sizeof(Movie));

	if(!L) {
		print_error("Cannot Allocate Memory");
		return;
	}

	if(!R) {
		print_error("Cannot Allocate Memeory");
		return;
	}

	for(i = 0; i < n1; i++) {
		Movie temp;
		temp.raw_data = movies[l + i].raw_data;
		temp.data = movies[l + i].data;
		temp.size = movies[l + i].size;

		L[i] = temp;
	}

	for(j = 0; j < n2; j++) {
	    	Movie temp;
		temp.raw_data = movies[m + j + 1].raw_data;
		temp.data = movies[m + j + 1].data;
		temp.size = movies[m + j + 1].size;

		R[j] = temp;
	}

	i = 0;
	j = 0;
	k = l;

	while(i < n1 && j < n2) {		
		int cmp = cmp_movies(&L[i], &R[j], cat_index);

		if(cmp <= 0) {
			movies[k] = L[i];
			i++;
		} else {
			movies[k] = R[j];
			j++;
		}

		k++;
	}

	while(i < n1) {
		movies[k] = L[i];
		i++;
		k++;
	}

	while(j < n2) {
		movies[k] = R[j];
		j++;
		k++;
	}

	//delete temp arrays
	free(L);
	free(R);
}

//Quicksort partitioning function
int part(Movie* movies, int cat_index, int l, int r) {
    	//int pivot_index = (rand() % (r - l + 1)) + l;
	int pivot_index = r;

	/*
	{
		Movie t = movies[pivot_index];
		movies[pivot_index] = movies[r];
		movies[r] = t;
	}
	*/

	//swap(&movies[pivot_index], &movies[r]);

	int i = l;
	int j;

	for(j = l; j < r; j++) {
		int cmp = cmp_movies(&movies[j], &movies[pivot_index], cat_index);
		//int cmp = 0;

		if(cmp <= 0) {
			Movie t = movies[i];
			movies[i] = movies[j];
			movies[j] = t;

			i++;
			//swap(&movies[++i], &movies[j]);
		}
	}

	{
		Movie t = movies[i];
		movies[i] = movies[r];
		movies[r] = t;
	}

	//swap(&movies[i + 1], &movies[r]);

	return (i);
}

//Helper function for the merge sort sorting algorithm
void msort(Movie* movies, int cat_index, int l, int r) {
	if(l < r) {
		int m = (l + r) / 2;

		msort(movies, cat_index, l, m);
		msort(movies, cat_index, m+1, r);
		merge(movies, cat_index, l, m, r);
	}
}

//Helper helper function for the quick sort sorting algorithm
void qsort_h(Movie* movies, int cat_index, int l, int r) {
    	int mode = 1;

	if(mode == 0) {
	//Optimized recursive quick sort
		while(l < r) {
			int pi = part(movies, cat_index, l, r);

			if(pi - l < r - pi) {
				qsort_h(movies, cat_index, l, pi - 1);
				l = pi + 1;
			} else {
				qsort_h(movies, cat_index, pi + 1, r);
				r = pi - 1;
			}
		}
	} else if(mode == 1) {
	//Iterative quicksort
		
		int* stack = malloc(sizeof(int) * (r - l + 1));

		int top = -1;

		stack[++top] = l;
		stack[++top] = r;

		while(top >= 0) {
			r = stack[top--];
			l = stack[top--];

			int p = part(movies, cat_index, l, r);

			if(p - 1 > l) {
				stack[++top] = l;
				stack[++top] = p - 1;
			}

			if(p + 1 < r) {
				stack[++top] = p + 1;
				stack[++top] = r;
			}

			//printf("Cat:index: %d Top: %d p:%d l:%d r:%d\n", cat_index, top, p, l, r);
		}
	} else {
	//Unoptimized recursive quick sort
		if(l < r) {
			int pi = part(movies, cat_index, l, r);
			qsort_h(movies, cat_index, l, pi - 1);
			qsort_h(movies, cat_index, pi + 1, r);
		}
	}
}

//Helper function for the quick sort sorting algorithm
void qksort(Movie* movies, int cat_index, int l, int r) {
    	srand(time(0));
	qsort_h(movies, cat_index, l, r);

	//Call Insertion Sort
}


/*
//Pulls the elements with a value of null for the data at the index cat_index to the front in the original order
int bring_null_forward(Movie* movies, int cat_index, int size) {
	int i, j, count = 0;

	for(i = 0; i < size; i++) {
		char* data = movies[i].data[cat_index];
		
		if(data[0] == '\0') {
			count++;
			int null_index = i;
			
			for(j = 0; j < size; j++) {
				char* other_data = movies[j].data[cat_index];
				if(other_data[0] != '\0') {
					int replace_index = j;
					
					swap(&movies[null_index], &movies[replace_index]);
					//Movie temp = movies[null_index];
					//movies[null_index] = movies[replace_index];
					//movies[replace_index] = temp;
				}
			}
		}
	}

	int last_null_index = -1;

	for(i = 0; i < size; i++) {
		char* data = movies[i].data[cat_index];

		if(data[0] == '\0') {
			last_null_index = i;
		}
	}

	return last_null_index;
}
*/
